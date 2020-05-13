#include "TrafficTracker.hpp"


#include <fstream>
#include <iostream>

#include <QtConcurrent/QtConcurrent>

#include <TrafficMapper/Globals>
#include <TrafficMapper/Asserts/HungarianAlgorithm>
#include <TrafficMapper/Modules/FrameProvider>
#include <TrafficMapper/Modules/GateModel>
#include <TrafficMapper/Classes/Vehicle>
#include <TrafficMapper/Classes/Gate>
#include <TrafficMapper/Classes/Detection>

#include <cppitertools/enumerate.hpp>


using namespace iter;


// ========================================
//       CONSTRUCTORS & DESTRUCTORS
// ========================================

TrafficTracker::TrafficTracker()
{
	m_gateModel_ptr = nullptr;
	m_statModel_ptr = nullptr;
	m_isRunning = false;
	m_stat_axisY_maxval = 0;
}

TrafficTracker::~TrafficTracker()
{
	for (auto vehicle_ptr : m_vehicles)
		delete vehicle_ptr;
}



void TrafficTracker::setGateModel(GateModel * gateModel_ptr)
{
	m_gateModel_ptr = gateModel_ptr;
}

void TrafficTracker::setStatModel(StatModel * statModel_ptr)
{
	m_statModel_ptr = statModel_ptr;
}

void TrafficTracker::extractDetectionData(const QUrl & cacheFileUrl)
{
	m_isRunning = true;

	QtConcurrent::run([this, cacheFileUrl]() {
		GlobalMeta * globals = GlobalMeta::getInstance();
		const int allFrameNr = globals->VIDEO_FRAMECOUNT();
		const int videoWidth = globals->VIDEO_WIDTH();
		const int videoHeight = globals->VIDEO_HEIGHT();

		openCacheFile(cacheFileUrl);

		std::fstream cacheFile(cacheFileUrl.toLocalFile().toStdString(), std::fstream::out | std::fstream::app);
		FrameProvider video(m_detections.size());
		cv::Mat frame;
		cv::dnn::Net net = initNeuralNet();

		for (int frameIdx(m_detections.size()); frameIdx < allFrameNr; ++frameIdx)
		{
			if (!m_isRunning)
			{
				emit processTerminated();
				break;
			}

			emit progressUpdated(frameIdx, allFrameNr);

			video.getNextFrame(frame);
			if (frame.empty()) break;

			std::vector<Detection> frameDetections = getRawFrameDetections(frame, net);
			cacheFile << frameIdx << " " << frameDetections.size() << "\n";
			for (auto detection : frameDetections)
			{
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

	QtConcurrent::run([this]() {
		emit analysisStarted();

		FrameProvider video;
		cv::Mat frame, prevFrame;

		std::vector<Vehicle *> activeTracks;

		cv::dnn::Net net = initNeuralNet();

		const int allFrameNr = GlobalMeta::getInstance()->VIDEO_FRAMECOUNT();

		for (size_t frameIdx(0); frameIdx < allFrameNr; ++frameIdx)
		{
	//		if (frameIdx < 24) continue;

			// If the user interrupted the process then exit the cycle.
			if (!m_isRunning) break;

			// Update the "Progress Window".
			emit progressUpdated(frameIdx, allFrameNr);

			video.getNextFrame(frame);

			// Trying to read the cached detection data of the frame.
			// If cannot, use DNN to get vehicle detections.
			std::vector<Detection> frameDetections;
			try
			{
				frameDetections = m_detections.at(frameIdx);
			}
			catch (std::out_of_range & ex)
			{
				frameDetections = getRawFrameDetections(frame, net);
				filterFrameDetections(frameDetections);
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

			for (auto && [i, vehicle_ptr] : enumerate(activeTracks))
			{
				if (assignment[i] != -1 && iouMatrix[i][assignment[i]] <= Settings::TRACKER_IOU_TRESHOLD)
				{
					vehicle_ptr->updatePosition(frameIdx, frameDetections[assignment[i]]);
					m_trajectories[frameIdx].push_back(vehicle_ptr);
				}
				else
				{
					vehicle_ptr->stopTracking();
					assignment[i] = -1;
					if (vehicle_ptr->trackPosition(frame, prevFrame, frameIdx))
					{
						m_trajectories[frameIdx].push_back(vehicle_ptr);
					}
				}
			}

			for (auto&& [i, detection] : enumerate(frameDetections))
			{
				//if (std::none_of(std::begin(assignment), std::end(assignment), i))
				if (std::find(std::begin(assignment), std::end(assignment), i) == std::end(assignment))
				{
					Vehicle * newVehicle = new Vehicle(frameIdx, detection);
					m_vehicles.push_back(newVehicle);
					m_trajectories[frameIdx].push_back(newVehicle);
					activeTracks.push_back(newVehicle);
				}
			}

			activeTracks.erase(
				std::remove_if(
					activeTracks.begin(),
					activeTracks.end(),
					[](Vehicle * vehicle_ptr) { return !vehicle_ptr->isTracked(); }
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
	std::vector<std::vector<double>> & iouMatrix,
	const int & tNum, const int & dNum,
	std::vector<Detection> & prevDetections,
	std::vector<Detection> & frameDetections)
{
	iouMatrix.resize(tNum);
	for (int vehicleIdx(0); vehicleIdx < tNum; ++vehicleIdx)
	{
		iouMatrix[vehicleIdx].resize(dNum);
		for (int detectionIdx(0); detectionIdx < dNum; ++detectionIdx)
		{
			float iou = -1 * Detection::iou(prevDetections[vehicleIdx], frameDetections[detectionIdx]);
			iouMatrix[vehicleIdx][detectionIdx] = iou;
		}
	}
}

inline void TrafficTracker::filterFrameDetections(std::vector<Detection> & frameDetections)
{
	const float confTreshold = Settings::DETECTOR_CONF_THRESHOLD;
	const float clipTreshold = Settings::DETECTOR_CLIP_TRESHOLD;
	const float nmsTreshold = Settings::DETECTOR_NMS_THRESHOLD;
	const int videoWidth = GlobalMeta::getInstance()->VIDEO_WIDTH();
	const int videoHeight = GlobalMeta::getInstance()->VIDEO_HEIGHT();

	// Filtering detections by clipping treshold.
	if (clipTreshold > 0)
	{
		frameDetections.erase(
			std::remove_if(std::begin(frameDetections), std::end(frameDetections),
				[confTreshold, clipTreshold, videoWidth, videoHeight](const Detection & det) {
					return det.x < clipTreshold
						|| det.y < clipTreshold
						|| det.x + det.width > videoWidth - clipTreshold
						|| det.y + det.height > videoHeight - clipTreshold;
				}), std::end(frameDetections));
	}

	if (frameDetections.size() > 1)
	{
		// Prepairing detections (sorting by confidence score) for NMS.
		std::sort(std::begin(frameDetections), std::end(frameDetections),
			[](const Detection & lhs, const Detection & rhs) { return lhs.confidence() > rhs.confidence(); });

		// Applying Non-Maximum Supresson.
		for (auto it_1 = std::begin(frameDetections); it_1 != std::end(frameDetections) - 1; ++it_1)
		{
			for (auto it_2 = it_1 + 1; it_2 != std::end(frameDetections); ++it_2)
			{
				if (Detection::iou(*it_1, *it_2) > nmsTreshold)
				{
					it_2->markToDelete();
				}
			}
		}
		frameDetections.erase(
			std::remove_if(std::begin(frameDetections), std::end(frameDetections),
				[](const Detection & det) {
					return det.isDeletable();
				}),
			std::end(frameDetections));
	}
}

void TrafficTracker::terminate()
{
	QMutexLocker locker(&m_runningMutex);

	m_isRunning = false;
}

std::vector<Vehicle *> TrafficTracker::getVehiclesOnFrame(const int & frameIdx)
{
	try
	{
		return m_trajectories.at(frameIdx);
	}
	catch (const std::out_of_range & ex)
	{
		return std::vector<Vehicle *>();
	}
}

inline cv::dnn::Net TrafficTracker::initNeuralNet()
{
	cv::dnn::Net net = cv::dnn::readNet(Settings::DETECTOR_WEIGHTS_PATH, Settings::DETECTOR_CONFIG_PATH);
	net.setPreferableBackend(Settings::DETECTOR_BACKEND);
	net.setPreferableTarget(Settings::DETECTOR_TARGET);

	return net;
}

inline std::vector<Detection> TrafficTracker::getRawFrameDetections(const cv::Mat & frame, cv::dnn::Net & net)
{
	const int videoWidth = frame.cols;
	const int videoHeight = frame.rows;

	cv::Mat blob;
	std::vector<cv::Mat> outs;
	std::vector<Detection> frameDetections;

	cv::dnn::blobFromImage(frame, blob, 1 / 255.0, Settings::DETECTOR_DNN_BLOB_SIZE, cv::Scalar(), false, false, CV_32F);

	net.setInput(blob);
	net.forward(outs, net.getUnconnectedOutLayersNames());

	for (auto layer : outs)
	{
		float * data = (float *)layer.data;
		for (int detIdx = 0; detIdx < layer.rows; ++detIdx, data += layer.cols)
		{
			cv::Mat scores = layer.row(detIdx).colRange(5, layer.cols);
			cv::Point classIdPoint;
			double confidence;
			cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
			if (confidence > Settings::DETECTOR_CONF_THRESHOLD)
			{
				const int centerX	= static_cast<int>(data[0] * videoWidth);
				const int centerY	= static_cast<int>(data[1] * videoHeight);
				const int width		= static_cast<int>(data[2] * videoWidth);
				const int height	= static_cast<int>(data[3] * videoHeight);
				const int x			= static_cast<int>(centerX - width * 0.5f);
				const int y			= static_cast<int>(centerY - height * 0.5f);

				frameDetections.push_back(Detection(x, y, width, height, VehicleType(classIdPoint.x), confidence));
			}
		}
	}

	return frameDetections;
}

void TrafficTracker::openCacheFile(const QUrl & fileUrl)
{
	m_detections.clear();

	GlobalMeta * globals = GlobalMeta::getInstance();

	int frameIdx, detNum;

	std::ifstream ifs(fileUrl.toLocalFile().toStdString());

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

std::vector<Detection> TrafficTracker::getDetections(const int & frameIdx) const
{
	try
	{
		return m_detections.at(frameIdx);
	}
	catch (const out_of_range & ex)
	{
		return std::vector<Detection>();
	}
}

void TrafficTracker::generateStatistics(Gate * gate_ptr, const int & interval)
{
	const int intNr = std::ceil(GlobalMeta::getInstance()->VIDEO_LENGTH() / interval * 0.001f);
	const int intSize = GlobalMeta::getInstance()->VIDEO_FPS() * interval;

	// TODO: Remove later
	gate_ptr = m_gateModel_ptr->getGates()[0];
	auto stat = gate_ptr->getStatistics();
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
		const int minutes = i * interval / 60;
		const int seconds = i * interval % 60;

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

void TrafficTracker::onFrameDisplayed(const int & frameIdx)
{
	m_gateModel_ptr->onFrameDisplayed(frameIdx);
}
