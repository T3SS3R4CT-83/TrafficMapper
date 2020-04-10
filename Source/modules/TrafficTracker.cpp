#include "TrafficTracker.hpp"

//#include <experimental/filesystem>
#include <fstream>
#include <unordered_set>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include <QtConcurrent/QtConcurrent>
#include <QMutexLocker>

#include <TrafficMapper/Globals>
#include <TrafficMapper/Asserts/HungarianAlgorithm>
#include <TrafficMapper/Modules/FrameProvider>
#include <TrafficMapper/Classes/Vehicle>
#include <TrafficMapper/Classes/Gate>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>



// ========================================
//       CONSTRUCTORS & DESTRUCTORS
// ========================================

TrafficTracker::TrafficTracker()
{
	m_gateModel_ptr = nullptr;
	m_isRunning = false;
}

TrafficTracker::~TrafficTracker()
{
	for (auto vehicle_ptr : m_vehicles)
		delete vehicle_ptr;
}



void TrafficTracker::setGateModel(GateModel *_gateModel)
{
    m_gateModel_ptr = _gateModel;
}

void TrafficTracker::extractDetectionData(QUrl _cacheFileUrl)
{
	m_isRunning = true;

    QtConcurrent::run([this, _cacheFileUrl]()
    {
		GlobalMeta* globals = GlobalMeta::getInstance();
		const int allFrameNr = globals->VIDEO_FRAMECOUNT();
		const int videoWidth = globals->VIDEO_WIDTH();
		const int videoHeight = globals->VIDEO_HEIGHT();

		openCacheFile(_cacheFileUrl);

		std::fstream cacheFile(_cacheFileUrl.toLocalFile().toStdString(), std::fstream::out | std::fstream::app);
		FrameProvider video(m_detections.size());
        cv::Mat frame;
		cv::dnn::Net net = initNeuralNet();

		for (int frameIdx(m_detections.size()); frameIdx < allFrameNr; ++frameIdx)
		{
			if (!m_isRunning) {
				emit processTerminated();
				break;
			}

			emit progressUpdated(frameIdx, allFrameNr);

			video.getNextFrame(frame);
			if (frame.empty()) break;

			std::vector<Detection> frameDetections = getRawFrameDetections(frame, net);
			cacheFile << frameIdx << " " << frameDetections.size() << "\n";
			for (auto detection : frameDetections) {
				cacheFile << detection;
			}
		}

		cacheFile.close();

		emit processTerminated();
    });
}

void TrafficTracker::analizeVideo()
{
	m_isRunning = true;

	m_vehicles.clear();
	m_trajectories.clear();

	QtConcurrent::run([this]()
	{
		//emit analysisStarted();

		FrameProvider video;
		cv::Mat frame, prevFrame;

		std::vector<Vehicle*> activeTracks;

		//cv::dnn::Net net = initNeuralNet();

		const int allFrameNr = GlobalMeta::getInstance()->VIDEO_FRAMECOUNT();

		for (size_t frameIdx(0); frameIdx < allFrameNr; ++frameIdx)
		{
			// If the user is interrupted the process then exit the cycle.
			if (!m_isRunning) break;

			// Update the "Progress Window".
			emit progressUpdated(frameIdx, allFrameNr);

			video.getNextFrame(frame);

			// Trying to read the cached detection data of the frame.
			// If cannot, use DNN to get vehicle detections.
			std::vector<Detection> frameDetections;
			try {
				frameDetections = m_detections.at(frameIdx);
			}
			catch (std::out_of_range& ex) {
				//frameDetections = getRawFrameDetections(frame, net);
				//filterFrameDetections(frameDetections);
			}

			const int trackingNumber = activeTracks.size();
			const int detectionNumber = frameDetections.size();

			std::vector<int> assignment;
			std::vector<std::vector<double>> iouMatrix;

			if (trackingNumber && detectionNumber)
			{
				std::vector<Detection> prevDetections;

				for (auto vehicle : activeTracks)
					prevDetections.push_back(vehicle->detection(frameIdx - 1));

				// Preparing IOU matrix
				prepIOUmatrix(iouMatrix, trackingNumber, detectionNumber, prevDetections, frameDetections);

				// Running "Hungarian algorithm" on IOU matrix
				HungarianAlgorithm::Solve(iouMatrix, assignment);
			}

			for (auto [i, vehicle_ptr] : enumerate(activeTracks))
			{
				if (assignment[i] != -1 && iouMatrix[i][assignment[i]] <= Settings::TRACKER_IOU_TRESHOLD) {
					vehicle_ptr->updatePosition(frameIdx, frameDetections[assignment[i]]);
					m_trajectories[frameIdx].push_back(vehicle_ptr);
				}
				else {
					vehicle_ptr->m_isTracked = false;
					assignment[i] = -1;
					if (vehicle_ptr->trackPosition(frame, prevFrame, frameIdx)) {
						m_trajectories[frameIdx].push_back(vehicle_ptr);
					}
					else {

					}
				}
			}

			for (auto [i, detection] : enumerate(frameDetections))
			{
				//if (std::none_of(std::begin(assignment), std::end(assignment), i))
				if (std::find(std::begin(assignment), std::end(assignment), i) == std::end(assignment))
				{
					Vehicle* newVehicle = new Vehicle(frameIdx, detection);
					m_vehicles.push_back(newVehicle);
					m_trajectories[frameIdx].push_back(newVehicle);
					activeTracks.push_back(newVehicle);
				}
			}

			activeTracks.erase(
				std::remove_if(
					activeTracks.begin(),
					activeTracks.end(),
					[](Vehicle *vehicle_ptr) {
						return !vehicle_ptr->isTracked();
					}
				),
				activeTracks.end()
			);

			prevFrame = frame;
		}

		for (auto vehicle : m_vehicles)
		{
			vehicle->calcVehicleType();
			m_gateModel_ptr->checkVehicle(vehicle);
		}

		m_gateModel_ptr->buildGateStats();

		emit processTerminated();

		m_isRunning = false;
	});
}

inline void TrafficTracker::prepIOUmatrix(
	std::vector<std::vector<double>>& _iouMatrix,
	const int& _tNum, const int& _dNum,
	std::vector<Detection>& _prevDetections,
	std::vector<Detection>& _frameDetections)
{
	_iouMatrix.resize(_tNum);
	for (int vehicleIdx(0); vehicleIdx < _tNum; ++vehicleIdx)
	{
		_iouMatrix[vehicleIdx].resize(_dNum);
		for (int detectionIdx(0); detectionIdx < _dNum; ++detectionIdx)
		{
			float iou = -1 * Detection::iou(_prevDetections[vehicleIdx], _frameDetections[detectionIdx]);
			_iouMatrix[vehicleIdx][detectionIdx] = iou;
			//_iouMatrix[vehicleIdx][detectionIdx] = (iou <= Settings::TRACKER_IOU_TRESHOLD) ? iou : 0.f;
		}
	}
}

void TrafficTracker::terminate()
{
//	QMutexLocker locker(&m_runningMutex);
	m_isRunning = false;
}

std::vector<Vehicle *> TrafficTracker::getVehiclesOnFrame(const int frameIdx)
{
	try {
		return m_trajectories.at(frameIdx);
	}
	catch (const std::out_of_range & ex) {
		return std::vector<Vehicle*>();
	}
}

//bool TrafficTracker::isRunning()
//{
//	QMutexLocker locker(&m_runningMutex);
//	return m_isRunning;
//}

inline cv::dnn::Net TrafficTracker::initNeuralNet()
{
	cv::dnn::Net net = cv::dnn::readNet(Settings::DETECTOR_WEIGHTS_PATH, Settings::DETECTOR_CONFIG_PATH);
	net.setPreferableBackend(Settings::DETECTOR_BACKEND);
	net.setPreferableTarget(Settings::DETECTOR_TARGET);

	return net;
}

inline std::vector<Detection> TrafficTracker::getRawFrameDetections(const cv::Mat& _frame, cv::dnn::Net &_net)
{
	GlobalMeta* globals = GlobalMeta::getInstance();

	const int videoWidth = globals->VIDEO_WIDTH();
	const int videoHeight = globals->VIDEO_HEIGHT();

	cv::Mat blob;
	std::vector<cv::Mat> outs;
	std::vector<Detection> frameDetections;

	cv::dnn::blobFromImage(_frame, blob, 1 / 255.0, Settings::DETECTOR_DNN_BLOB_SIZE, cv::Scalar(), false, false, CV_32F);

	_net.setInput(blob);
	_net.forward(outs, _net.getUnconnectedOutLayersNames());

	for (auto layer : outs)
	{
		float* data = (float*)layer.data;
		for (int detIdx = 0; detIdx < layer.rows; ++detIdx, data += layer.cols)
		{
			cv::Mat scores = layer.row(detIdx).colRange(5, layer.cols);
			cv::Point classIdPoint;
			double confidence;
			cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
			const VehicleType vType = VehicleType(classIdPoint.x);
			if (Settings::DETECTOR_CLASSES.count(vType) > 0)
			{
				const int centerX	= static_cast<int>(data[0] * videoWidth);
				const int centerY	= static_cast<int>(data[1] * videoHeight);
				const int width		= static_cast<int>(data[2] * videoWidth);
				const int height	= static_cast<int>(data[3] * videoHeight);
				const int x			= static_cast<int>(centerX - width * 0.5f);
				const int y			= static_cast<int>(centerY - height * 0.5f);

				frameDetections.push_back(Detection(x, y, width, height, vType, confidence));
			}
		}
	}

	return frameDetections;
}

//inline void TrafficTracker::filterFrameDetections(std::vector<Detection>& _frameDetections)
//{
//	GlobalMeta* globals = GlobalMeta::getInstance();
//
//	// Removing detections which's confidence score is too low or they're too close to the frame border.
//	_frameDetections.erase(
//		std::remove_if(
//			_frameDetections.begin(),
//			_frameDetections.end(),
//			[globals](const Detection detection) {
//				return detection.confidence() < Settings::DETECTOR_CONF_THRESHOLD
//					|| detection.x < Settings::DETECTOR_CLIP_TRESHOLD
//					|| detection.y < Settings::DETECTOR_CLIP_TRESHOLD
//					|| detection.x + detection.width > globals->VIDEO_WIDTH() - Settings::DETECTOR_CLIP_TRESHOLD
//					|| detection.y + detection.height > globals->VIDEO_HEIGHT() - Settings::DETECTOR_CLIP_TRESHOLD;
//			}
//		),
//		_frameDetections.end()
//	);
//
//	// Sort by confidence scores in descending order.
//	std::stable_sort(_frameDetections.begin(), _frameDetections.end(),
//		[](const Detection& det_1, const Detection& det_2) {
//			return det_1.confidence() > det_2.confidence();
//		});
//
//	// Run NMS (Non-Maximum Supression) on the stack and mark the removable items.
//	for (auto it_higher = std::begin(_frameDetections); it_higher < std::end(_frameDetections) - 1; ++it_higher)
//	{
//		if (it_higher->deletable()) continue;
//		for (auto it_lower = it_higher + 1; it_lower != std::end(_frameDetections); ++it_lower)
//		{
//			const float iou = Detection::iou(*it_higher, *it_lower);
//			if (iou > Settings::DETECTOR_NMS_THRESHOLD)
//				it_lower->markToDelete();
//		}
//	}
//
//	// Removing the marked detections.
//	_frameDetections.erase(
//		std::remove_if(
//			_frameDetections.begin(),
//			_frameDetections.end(),
//			[](const Detection detection) {
//				return detection.deletable();
//			}
//		),
//		_frameDetections.end()
//	);
//}

void TrafficTracker::openCacheFile(QUrl _fileUrl)
{
	m_detections.clear();

	GlobalMeta* globals = GlobalMeta::getInstance();

	int frameIdx, detNum;

	std::ifstream ifs(_fileUrl.toLocalFile().toStdString());

	while (ifs >> frameIdx)
	{
		ifs >> detNum;

		std::vector<Detection> frameDetections;

		for (int i(0); i < detNum; ++i)
		{
			Detection detection;
			ifs >> detection;
			frameDetections.push_back(detection);
		}

//		filterFrameDetections(frameDetections);

		m_detections[frameIdx] = frameDetections;
	}

	ifs.close();
}

void TrafficTracker::exportFrames()
{
	m_isRunning = true;

	QtConcurrent::run([this]()
	{
		cv::Mat frame;
		FrameProvider video;

		for (int i(0); ; ++i)
		{
			if (!m_isRunning) {
				emit processTerminated();
				break;
			}

			emit progressUpdated(i, GlobalMeta::getInstance()->VIDEO_FRAMECOUNT());

			video.getNextFrame(frame);

			if (frame.empty()) break;

			for (auto det : m_detections[i])
			{
				cv::rectangle(frame, det, cv::Scalar(255, 0, 0), 2);
			}

			// TODO: Remove later
			cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE); // Create a window for display.
			cv::imshow("Display window", frame);
			cv::waitKey(0);

			std::stringstream fileName("");
			fileName << "D:\\Videos\\Export\\" << std::setfill('0') << std::setw(5) << i << ".png";
			cv::imwrite(fileName.str(), frame);
		}

		emit processTerminated();
	});
}

std::vector<Detection> TrafficTracker::getDetections(const int frameIdx) const
{
	try {
		return m_detections.at(frameIdx);
	}
	catch (const out_of_range & ex) {
		return std::vector<Detection>();
	}
}

void TrafficTracker::generateStatistics(Gate* _gate_ptr, const int _interval)
{
	const int intNr = std::ceil(GlobalMeta::getInstance()->VIDEO_LENGTH() / _interval * 0.001f);
	const int intSize = GlobalMeta::getInstance()->VIDEO_FPS() * _interval;

	_gate_ptr = m_gateModel_ptr->getGates()[0];
	auto stat = _gate_ptr->getStatistics();
	m_stat_axisY_maxval = 0;

	m_stat_axisX_values.clear();
	m_stat_vtype_values[VehicleType::BICYCLE].clear();
	m_stat_vtype_values[VehicleType::BICYCLE].resize(intNr, 0);
	m_stat_vtype_values[VehicleType::BUS].clear();
	m_stat_vtype_values[VehicleType::BUS].resize(intNr, 0);
	m_stat_vtype_values[VehicleType::CAR].clear();
	m_stat_vtype_values[VehicleType::CAR].resize(intNr, 0);
	m_stat_vtype_values[VehicleType::MOTORCYCLE].clear();
	m_stat_vtype_values[VehicleType::MOTORCYCLE].resize(intNr, 0);
	m_stat_vtype_values[VehicleType::TRUCK].clear();
	m_stat_vtype_values[VehicleType::TRUCK].resize(intNr, 0);

	for (int i(0); i < GlobalMeta::getInstance()->VIDEO_FRAMECOUNT(); ++i)
	{
		m_stat_vtype_values[VehicleType::BICYCLE][i / intSize] += stat[VehicleType::BICYCLE][i];
		m_stat_vtype_values[VehicleType::BUS][i / intSize] += stat[VehicleType::BUS][i];
		m_stat_vtype_values[VehicleType::CAR][i / intSize] += stat[VehicleType::CAR][i];
		m_stat_vtype_values[VehicleType::MOTORCYCLE][i / intSize] += stat[VehicleType::MOTORCYCLE][i];
		m_stat_vtype_values[VehicleType::TRUCK][i / intSize] += stat[VehicleType::TRUCK][i];
	}

	for (int i(0); i < intNr; ++i)
	{
		int val = 0;

		val += m_stat_vtype_values[VehicleType::BICYCLE][i];
		val += m_stat_vtype_values[VehicleType::BUS][i];
		val += m_stat_vtype_values[VehicleType::CAR][i];
		val += m_stat_vtype_values[VehicleType::MOTORCYCLE][i];
		val += m_stat_vtype_values[VehicleType::TRUCK][i];

		if (val > m_stat_axisY_maxval) m_stat_axisY_maxval = val;
	}

	for (int i(1); i <= intNr; ++i)
	{
		const int minutes = i * _interval / 60;
		const int seconds = i * _interval % 60;

		m_stat_axisX_values << QString("%1:%2").arg(minutes).arg(seconds);
	}
}

QStringList TrafficTracker::getAxisX()
{
	return m_stat_axisX_values;
}

int TrafficTracker::getAxisY()
{
	return m_stat_axisY_maxval;
}

QList<QVariant> TrafficTracker::getCarValues()
{
	QList<QVariant> values;

	for (int i(0); i < m_stat_vtype_values[VehicleType::CAR].size(); ++i)
	{
		values << m_stat_vtype_values[VehicleType::CAR][i];
	}

	return values;
}

QList<QVariant> TrafficTracker::getTruckValues()
{
	QList<QVariant> values;

	for (int i(0); i < m_stat_vtype_values[VehicleType::TRUCK].size(); ++i)
	{
		values << m_stat_vtype_values[VehicleType::TRUCK][i];
	}

	return values;
}

QList<QVariant> TrafficTracker::getBusValues()
{
	QList<QVariant> values;

	for (int i(0); i < m_stat_vtype_values[VehicleType::BUS].size(); ++i)
	{
		values << m_stat_vtype_values[VehicleType::BUS][i];
	}

	return values;
}

void TrafficTracker::onFrameDisplayed(int _frameIdx)
{
	m_gateModel_ptr->onFrameDisplayed(_frameIdx);
}
