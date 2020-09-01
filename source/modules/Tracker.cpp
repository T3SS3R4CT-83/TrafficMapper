#include "Tracker.hpp"



#include <fstream>

#include <QtConcurrent/QtConcurrent>

#include <TrafficMapper/Modules/VehicleModel>
#include <TrafficMapper/Complementary/CameraCalibration>
#include <TrafficMapper/Media/MediaPlayer>
#include <TrafficMapper/Types/Detection>

#include <HungarianAlgorithm.hpp>
#include <cppitertools/enumerate.hpp>


#define IOU_TRESHOLD -0.3f


using namespace iter;


Tracker::Tracker(QObject * parent)
	: QObject(parent), m_isRunning(false)
{
}



void Tracker::setFrameProvider(MediaPlayer * frameProvider_ptr)
{
	m_media_ptr = frameProvider_ptr;

	connect(this, &Tracker::analysisStarted,
		m_media_ptr, &MediaPlayer::onAnalysisStarted);
	connect(this, &Tracker::analysisEnded,
		m_media_ptr, &MediaPlayer::onAnalysisEnded);

	connect(frameProvider_ptr, &QMediaPlayer::mediaStatusChanged,
		this, &Tracker::onVideoLoading);
}



void Tracker::openCacheFile(QUrl fileUrl)
{
	m_detections.clear();

	uint frameIdx;
	uint detNum;

	std::ifstream ifs(fileUrl.toLocalFile().toStdString());

	while (ifs >> frameIdx)
	{
		ifs >> detNum;

		std::vector<Detection> frameDetections;

		for (int i(0); i < detNum; ++i)
		{
			Detection detection;
			ifs >> detection;

			if (detection.isValid())
				frameDetections.push_back(detection);
		}

	//	filterFrameDetections(frameDetections);

		m_detections[frameIdx] = frameDetections;
	}

	emit cacheSizeChanged();

	ifs.close();
}

void Tracker::analizeVideo(bool useGPU)
{
	m_isRunning = true;

	QtConcurrent::run([this, useGPU]()
	{
		qDebug() << "Tracker worker started!";

		emit analysisStarted();

		cv::Mat frame, prevFrame;

		std::vector<Vehicle *> activeTrackings;

		cv::dnn::Net net = initYOLO(useGPU);

		for (size_t frameIdx(0); frameIdx < m_media_ptr->m_videoMeta.FRAMECOUNT && m_isRunning; ++frameIdx)
		{
			// Update the "Progress Window".
			emit progressUpdated(frameIdx, m_media_ptr->m_videoMeta.FRAMECOUNT);

			m_media_ptr->getNextFrame(frame);

			std::vector<Detection> frameDetections;
			// Trying to read the cached detection data of the frame.
			// If cannot, use DNN to get vehicle detections.
			try
			{
				frameDetections = m_detections.at(frameIdx);
			}
			catch (std::out_of_range & ex)
			{
				frameDetections = getRawFrameDetections(frame, net);
				filterFrameDetections(frameDetections);
				m_detections[frameIdx] = frameDetections;
				emit cacheSizeChanged();
			}

			const int trackingNumber = activeTrackings.size();
			const int detectionNumber = frameDetections.size();

			std::vector<int> assignment;
			std::vector<std::vector<double>> iouMatrix;

			// If we have vehicles that are tracked (trackingNumber > 0)
			// and there are detections on the current frame (detectionNumber > 0)
			// then match them.
			if (trackingNumber && detectionNumber)
			{
				std::vector<Detection> prevDetections;

				for (auto vehicle : activeTrackings)
					prevDetections.push_back(vehicle->getDetection(frameIdx - 1));

				// Preparing IOU matrix
				prepIOUmatrix(iouMatrix, prevDetections, frameDetections);

				// Running "Hungarian algorithm" on IOU matrix
				HungarianAlgorithm::Solve(iouMatrix, assignment);
			}

			// Handling the currently tracked vehicles.
			for (auto && [i, vehicle_ptr] : enumerate(activeTrackings))
			{
				if (detectionNumber && assignment[i] != -1 && iouMatrix[i][assignment[i]] <= IOU_TRESHOLD)
				//if (detectionNumber && assignment[i] != -1)
				{
					// Add matched detections to vehicle.
					vehicle_ptr->updatePosition(frameIdx, frameDetections[assignment[i]]);
				}
				else
				{
					// If there is no matched detection for a vehicle, then
					// try to track its position using an optical tracker.
					vehicle_ptr->trackPosition(frame, prevFrame, frameIdx);
				}
			}

			// Creating new vehicles from the unmatched detections.
			for (auto && [i, detection] : enumerate(frameDetections))
			{
				if (std::find(std::begin(assignment), std::end(assignment), i) == std::end(assignment))
				{
					Vehicle * newVehicle = new Vehicle(frameIdx, detection);
					activeTrackings.push_back(newVehicle);
				}
			}

			// Remove finished trackings from the 'activeTrackings' list,
			// and forward them in the pipeline.
			activeTrackings.erase(
				std::remove_if(
					std::begin(activeTrackings),
					std::end(activeTrackings),
					[this](Vehicle * vehicle_ptr) {
						if (!vehicle_ptr->isTracked())
						{
							emit pipelineOutput(vehicle_ptr);
							return true;
						}
						return false;
					}
				),
				std::end(activeTrackings)
			);

			prevFrame = frame.clone();
		}

		for (auto vehicle_ptr : activeTrackings)
			emit pipelineOutput(vehicle_ptr);

		emit analysisEnded();

		qDebug() << "Tracker worker finished!";
	});
}

void Tracker::analizeVideo_v2(bool useGPU)
{
	m_isRunning = true;

	QtConcurrent::run([this, useGPU]()
	{
		qDebug() << "Tracker worker started!";

		emit analysisStarted();

		cv::Mat frame, prevFrame, frameGray, prevFrameGray;
		std::vector<cv::Point2f> optFlowPoints, prevOptFlowPoints;

		std::vector<Vehicle *> activeTrackings;

		cv::dnn::Net net = initYOLO(useGPU);

		m_media_ptr->getNextFrame(prevFrame);
		cv::cvtColor(prevFrame, prevFrameGray, cv::COLOR_BGR2GRAY);

		for (size_t frameIdx(1); frameIdx < m_media_ptr->m_videoMeta.FRAMECOUNT && m_isRunning; ++frameIdx)
		{
			// Update the "Progress Window".
			emit progressUpdated(frameIdx, m_media_ptr->m_videoMeta.FRAMECOUNT);

			m_media_ptr->getNextFrame(frame);
			cv::cvtColor(frame, frameGray, cv::COLOR_BGR2GRAY);

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
				m_detections[frameIdx] = frameDetections;
				emit cacheSizeChanged();
			}

			const int trackingNumber = activeTrackings.size();
			const int detectionNumber = frameDetections.size();

			std::vector<int> assignment;
			std::vector<std::vector<double>> iouMatrix;

			// If we have vehicles that are tracked (trackingNumber > 0)
			// and there are detections on the current frame (detectionNumber > 0)
			// then match them.
			if (trackingNumber && detectionNumber)
			{
				std::vector<Detection> prevDetections;

				for (auto vehicle : activeTrackings)
					prevDetections.push_back(vehicle->getDetection(frameIdx - 1));

				// Preparing IOU matrix
				prepIOUmatrix(iouMatrix, prevDetections, frameDetections);

				// Running "Hungarian algorithm" on IOU matrix
				HungarianAlgorithm::Solve(iouMatrix, assignment);
			}

			// Handling the currently tracked vehicles.
			for (auto && [i, vehicle_ptr] : enumerate(activeTrackings))
			{
				if (assignment[i] != -1 && iouMatrix[i][assignment[i]] <= -0.5f)
				{
					// Add matched detections to vehicle.
					vehicle_ptr->updatePosition(frameIdx, frameDetections[assignment[i]]);
				}
				else
				{
					// If there is no matched detection for a vehicle, then
					// try to track its position using an optical tracker.
					vehicle_ptr->trackPosition(frame, prevFrame, frameIdx);
				}
			}

			// Creating new vehicles from the unmatched detections.
			for (auto && [i, detection] : enumerate(frameDetections))
			{
				if (std::find(std::begin(assignment), std::end(assignment), i) == std::end(assignment))
				{
					Vehicle * newVehicle = new Vehicle(frameIdx, detection);
					activeTrackings.push_back(newVehicle);
				}
			}

			// Remove finished trackings from the 'activeTrackings' list,
			// and forward them in the pipeline.
			activeTrackings.erase(
				std::remove_if(
					std::begin(activeTrackings),
					std::end(activeTrackings),
					[this](Vehicle * vehicle_ptr) {
						if (!vehicle_ptr->isTracked())
						{
							emit pipelineOutput(vehicle_ptr);
							return true;
						}
						return false;
					}
				),
				std::end(activeTrackings)
			);

			prevFrame = frame;
		}

		for (auto vehicle_ptr : activeTrackings)
			emit pipelineOutput(vehicle_ptr);

		emit analysisEnded();

		qDebug() << "Tracker worker finished!";
	});
}

void Tracker::stop()
{
	m_isRunning = false;
}

void Tracker::onVideoLoading(QMediaPlayer::MediaStatus status)
{
	if (status == QMediaPlayer::LoadingMedia)
		m_detections.clear();
}



size_t Tracker::getCacheSize() const
{
	return m_detections.size();
}



inline cv::dnn::Net Tracker::initYOLO(bool useGPU)
{
	cv::dnn::Net net = cv::dnn::readNet("models/yolov4.weights", "models/yolov4.cfg");

	net.setPreferableBackend(cv::dnn::Backend::DNN_BACKEND_DEFAULT);
	if (useGPU)
		net.setPreferableTarget(cv::dnn::Target::DNN_TARGET_OPENCL);
	else
		net.setPreferableTarget(cv::dnn::Target::DNN_TARGET_CPU);

	return net;
}

inline std::vector<Detection> Tracker::getRawFrameDetections(const cv::Mat & frame, cv::dnn::Net & net)
{
	const int videoWidth = frame.cols;
	const int videoHeight = frame.rows;

	cv::Mat blob;
	std::vector<cv::Mat> outs;
	std::vector<Detection> frameDetections;

	cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(416, 416), cv::Scalar(), false, false, CV_32F);
	//cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(608, 608), cv::Scalar(), false, false, CV_32F);

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
			if (confidence > 0.5f)
			{
				const float centerX = static_cast<int>(data[0] * videoWidth);
				const float centerY = static_cast<int>(data[1] * videoHeight);
				const float width = static_cast<int>(data[2] * videoWidth);
				const float height = static_cast<int>(data[3] * videoHeight);
				const float x = static_cast<int>(centerX - width * 0.5f);
				const float y = static_cast<int>(centerY - height * 0.5f);

				frameDetections.push_back(Detection(x, y, width, height, VehicleType(classIdPoint.x), confidence));
			}
		}
	}

	return frameDetections;
}

inline void Tracker::filterFrameDetections(std::vector<Detection> & frameDetections)
{
	//const float clipTreshold = Settings::DETECTOR_CLIP_TRESHOLD;
	const float nmsTreshold = 0.5f;
	//const int videoWidth = GlobalMeta::getInstance()->VIDEO_WIDTH();
	//const int videoHeight = GlobalMeta::getInstance()->VIDEO_HEIGHT();

	// Filtering detections by clipping treshold.
	//if (clipTreshold > 0)
	//{
	//	frameDetections.erase(
	//		std::remove_if(std::begin(frameDetections), std::end(frameDetections),
	//			[clipTreshold, videoWidth, videoHeight](const Detection & det) {
	//				return det.x < clipTreshold
	//					|| det.y < clipTreshold
	//					|| det.x + det.width > videoWidth - clipTreshold
	//					|| det.y + det.height > videoHeight - clipTreshold;
	//			}), std::end(frameDetections));
	//}

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
				[](const Detection & det) { return det.isDeletable(); }
			),
			std::end(frameDetections));
	}
}

inline void Tracker::prepIOUmatrix(
	std::vector<std::vector<double>> & iouMatrix,
	std::vector<Detection> & prevDetections,
	std::vector<Detection> & frameDetections)
{
	const uint tNum = prevDetections.size();
	const uint dNum = frameDetections.size();

	iouMatrix.resize(tNum);
	for (int vehicleIdx(0); vehicleIdx < tNum; ++vehicleIdx)
	{
		iouMatrix[vehicleIdx].resize(dNum);
		for (int detectionIdx(0); detectionIdx < dNum; ++detectionIdx)
		{
			float iou = -1 * Detection::iou(prevDetections[vehicleIdx], frameDetections[detectionIdx]);
			//iouMatrix[vehicleIdx][detectionIdx] = iou;
			iouMatrix[vehicleIdx][detectionIdx] = iou < IOU_TRESHOLD ? iou : 0;
		}
	}
}
