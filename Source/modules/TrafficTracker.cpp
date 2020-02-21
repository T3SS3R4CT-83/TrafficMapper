#include "TrafficTracker.hpp"

//#include <experimental/filesystem>
#include <fstream>
#include <unordered_set>
#include <iomanip>
#include <iostream>

//#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QMutexLocker>
//#include <QUrl>

#include <TrafficMapper/Globals>
#include <TrafficMapper/Asserts/HungarianAlgorithm>
#include <TrafficMapper/Modules/FrameProvider>
#include <TrafficMapper/Classes/Vehicle>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>  // TODO: Remove later

//#include <opencv2/core/mat.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/videoio.hpp>
//#include <opencv2/dnn/dnn.hpp>


TrafficTracker::TrafficTracker()
{
	m_isRunning = false;
	//m_detections = std::map<int, std::vector<Detection>>();
	//m_trajectories = std::map<int, std::vector<Vehicle*>>();
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

		std::fstream file(_cacheFileUrl.toLocalFile().toStdString(), std::fstream::out | std::fstream::app);
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

//			//if (frameIdx <= 19645) continue;

			std::vector<Detection> frameDetections = getRawFrameDetections(frame, net);
			file << frameIdx << " " << frameDetections.size() << "\n";
			for (auto detection : frameDetections) {
				file << detection;
			}
		}

		file.close();

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
//		emit analysisStarted();

		cv::Mat frame;
		std::vector<Vehicle *> m_activeTrackings;

		const int allFrameNr = GlobalMeta::getInstance()->VIDEO_FRAMECOUNT();

		FrameProvider video;
		cv::dnn::Net net = initNeuralNet();
		
		for (int frameIdx(0); frameIdx < allFrameNr; ++frameIdx)
		{
			// If the user is interrupted the process then exit the cycle.
			if (!m_isRunning) break;

			// Update the "Progress Window".
			emit progressUpdated(frameIdx, allFrameNr);
			
			std::vector<Detection> frameDetections;
			try {
				frameDetections = m_detections.at(frameIdx);
			}
			catch (std::out_of_range & ex) {
				video.getNextFrame(frame, frameIdx);
				frameDetections = getRawFrameDetections(frame, net);
				filterFrameDetections(frameDetections);
			}

			if (frameDetections.size())
			{
				std::vector<int> unmatchedTrackers, assignment;
				std::vector<std::vector<double>> iouMatrix;
				std::vector<Detection> prevDetections;
					
				for (auto vehicle : m_activeTrackings)
					prevDetections.push_back(vehicle->detection(frameIdx - 1));

				const int trackingNumber = prevDetections.size();
				const int detectionNumber = frameDetections.size();
				
//				// Preparing IOU matrix
				iouMatrix.resize(trackingNumber);
				for (int i(0); i < iouMatrix.size(); ++i)
					iouMatrix[i].resize(detectionNumber);
				for (int vehicleIdx(0); vehicleIdx < trackingNumber; ++vehicleIdx)
					for (int detectionIdx(0); detectionIdx < detectionNumber; ++detectionIdx)
						iouMatrix[vehicleIdx][detectionIdx] = -1 * Detection::iou(prevDetections[vehicleIdx], frameDetections[detectionIdx]);

				// Running "Hungarian algorithm" on IOU matrix
				if (trackingNumber && detectionNumber)
					HungarianAlgorithm::Solve(iouMatrix, assignment);

				for (int i(0); i < trackingNumber; ++i) {
					// Assign matches to trackers
					if (assignment[i] != -1 && iouMatrix[i][assignment[i]] <= Settings::TRACKER_IOU_TRESHOLD) {
						m_activeTrackings[i]->updatePosition(frame, frameIdx, frameDetections[assignment[i]]);
						m_trajectories[frameIdx].push_back(m_activeTrackings[i]);
						//m_gateList->onVehiclePositionUpdated(m_activeTrackings[i], frameIdx);
					}
					// Deal with "unmatched trackers"
					else if (m_activeTrackings[i]->trackPosition(video, frameIdx)) {
						m_trajectories[frameIdx].push_back(m_activeTrackings[i]);
						//m_gateList->onVehiclePositionUpdated(m_activeTrackings[i], frameIdx);
					}
				}

				// Removing finished trackings from activeTrackings
				m_activeTrackings.erase(
					std::remove_if(
						m_activeTrackings.begin(),
						m_activeTrackings.end(),
						[](const Vehicle *vehicle) {
							return !vehicle->isActive();
						}
					),
					m_activeTrackings.end()
				);

				// Populate "unmatched detections"
				std::unordered_set<int> unmatchedDets;
				for (int i(0); i < frameDetections.size(); ++i)
					unmatchedDets.insert(i);
				for (int i(0); i < assignment.size(); ++i)
					unmatchedDets.erase(assignment[i]);
				for (int detectionIdx : unmatchedDets) {
					Vehicle *newVehicle = new Vehicle(frame, frameIdx, frameDetections[detectionIdx]);
					m_vehicles.push_back(newVehicle);
					m_activeTrackings.push_back(newVehicle);
					m_trajectories[frameIdx].push_back(newVehicle);
				}
			}
		}

		//m_vehicles.erase(
		//	std::remove_if(
		//		m_vehicles.begin(),
		//		m_vehicles.end(),
		//		[](const Vehicle *vehicle) {
		//			return !vehicle->isValid();
		//		}
		//	),
		//	m_vehicles.end()
		//);

		for (auto vehicle : m_vehicles) {
			//m_gateList->checkVehicle(vehicle);
			vehicle->calcVehicleType();
		}
		//m_gateList->buildGateHistory();

		emit processTerminated();
	});
}

void TrafficTracker::terminate()
{
	QMutexLocker locker(&m_runningMutex);
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

inline void TrafficTracker::filterFrameDetections(std::vector<Detection>& _frameDetections)
{
	GlobalMeta* globals = GlobalMeta::getInstance();

	// Removing detections which's confidence score is too low or they're too close to the frame border.
	_frameDetections.erase(
		std::remove_if(
			_frameDetections.begin(),
			_frameDetections.end(),
			[globals](const Detection detection) {
				return detection.confidence() < Settings::DETECTOR_CONF_THRESHOLD
					|| detection.x < Settings::DETECTOR_CLIP_TRESHOLD
					|| detection.y < Settings::DETECTOR_CLIP_TRESHOLD
					|| detection.x + detection.width > globals->VIDEO_WIDTH() - Settings::DETECTOR_CLIP_TRESHOLD
					|| detection.y + detection.height > globals->VIDEO_HEIGHT() - Settings::DETECTOR_CLIP_TRESHOLD;
			}
		),
		_frameDetections.end()
	);

	// Sort by confidence scores in descending order.
	std::stable_sort(_frameDetections.begin(), _frameDetections.end(),
		[](const Detection& det_1, const Detection& det_2) {
			return det_1.confidence() > det_2.confidence();
		});

	// Run NMS (Non-Maximum Supression) on the stack and mark the removable items.
	for (auto it_higher = std::begin(_frameDetections); it_higher < std::end(_frameDetections) - 1; ++it_higher)
	{
		if (it_higher->deletable()) continue;
		for (auto it_lower = it_higher + 1; it_lower != std::end(_frameDetections); ++it_lower)
		{
			const float iou = Detection::iou(*it_higher, *it_lower);
			if (iou > Settings::DETECTOR_NMS_THRESHOLD)
				it_lower->markToDelete();
		}
	}

	// Removing the marked detections.
	_frameDetections.erase(
		std::remove_if(
			_frameDetections.begin(),
			_frameDetections.end(),
			[](const Detection detection) {
				return detection.deletable();
			}
		),
		_frameDetections.end()
	);
}

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

		filterFrameDetections(frameDetections);

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
