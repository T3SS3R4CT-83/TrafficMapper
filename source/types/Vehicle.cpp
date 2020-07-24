#include "Vehicle.hpp"

//#include <QLineF>

#include <TrafficMapper/Complementary/FrameProvider>
#include <TrafficMapper/Complementary/AcceleratedModel>
#include <TrafficMapper/Types/Types>
#include <TrafficMapper/Types/Detection>

//#include <cppitertools/enumerate.hpp>
#include <cppitertools/sliding_window.hpp>

using namespace iter;



Vehicle::Vehicle(const int & frameIdx, const Detection & detection)	:
	m_isTracked(true),
	m_timeSinceLastHit(0),
	m_vehicleClass(VehicleType::undefined),
	m_firstFrame(frameIdx),
	m_lastFrame(frameIdx)
{
	m_detections[frameIdx] = detection;
}



Detection Vehicle::getDetection(const int & frameIdx) const
{
	try
	{
		return m_detections.at(frameIdx);
	}
	catch (const std::out_of_range &)
	{
		return Detection();
	}
}

std::vector<std::pair<uint, QLineF>> Vehicle::getTrajectory() const
{
	return m_trajectory;
}

bool Vehicle::isTracked() const
{
	return m_isTracked;
}



void Vehicle::updatePosition(const int & frameIdx, const Detection & detection)
{
	// If optical tracking is enabled then disable it.
	if (!m_tracker.empty())
	{
		m_tracker.release();
		m_timeSinceLastHit = 0;
	}

	addDetection(frameIdx, detection);
}

void Vehicle::trackPosition(const cv::Mat & frame, const cv::Mat & prevFrame, const int & frameIdx)
{
	cv::Rect2d newTrack;

	if (m_tracker.empty())
		initTracker(prevFrame, m_detections[frameIdx - 1]);

	if (m_tracker->update(frame, newTrack))
	{
		const float x = newTrack.x / frame.cols;
		const float y = newTrack.y / frame.rows;
		const float width = newTrack.width / frame.cols;
		const float height = newTrack.height / frame.rows;

		addDetection(frameIdx, Detection(x, y, width, height));

		if (++m_timeSinceLastHit == 10)
		{
			stopTracking();
		}
	}
	else
	{
		stopTracking();
	}
}



void Vehicle::calcVehicleType()
{
	std::unordered_map<VehicleType, float> sumConf {
		{VehicleType::CAR, 0},
		{VehicleType::BUS, 0},
		{VehicleType::TRUCK, 0},
		{VehicleType::MOTORCYCLE, 0},
		{VehicleType::BICYCLE, 0}
	};

	for (auto & detection : m_detections)
	{
		const VehicleType vehicleType = detection.second.vehicleType();
		const float confidence = detection.second.confidence();

		if (vehicleType != VehicleType::undefined)
			sumConf[vehicleType] += confidence;
	}

	auto max_element = std::max_element(std::begin(sumConf), std::end(sumConf),
		[](const decltype(sumConf)::value_type & p1, const decltype(sumConf)::value_type & p2) {
			return p1.second < p2.second;
		});

	m_vehicleClass = max_element->first;
}

void Vehicle::calcPositions()
{
	m_positions[m_firstFrame] = QPoint(
		std::round(m_detections[m_firstFrame].center().x() * FrameProvider::m_videoMeta.WIDTH),
		std::round(m_detections[m_firstFrame].center().y() * FrameProvider::m_videoMeta.HEIGHT)
	);

	cv::Ptr<cv::tracking::UnscentedKalmanFilter> kalmanFilter;
	initKalmanFilter(kalmanFilter);

	for (int frameIdx(m_firstFrame + 1); frameIdx <= m_lastFrame; ++frameIdx)
	{
		const uint vWidth = FrameProvider::m_videoMeta.WIDTH;
		const uint vHeight = FrameProvider::m_videoMeta.HEIGHT;

		kalmanFilter->predict();

		cv::Mat state(6, 1, CV_32F);
		cv::Mat measurement(2, 1, CV_32F);

		measurement.at<float>(0) = std::round(m_detections[frameIdx].center().x() * FrameProvider::m_videoMeta.WIDTH);
		measurement.at<float>(1) = std::round(m_detections[frameIdx].center().y() * FrameProvider::m_videoMeta.HEIGHT);

		state = kalmanFilter->correct(measurement);

		m_positions[frameIdx] = QPointF(state.at<float>(0), state.at<float>(1));

		m_trajectory.push_back(std::make_pair(frameIdx, QLineF(m_positions[frameIdx - 1], m_positions[frameIdx])));
	}



	kalmanFilter.release();
}

void Vehicle::calcVehicleSpeed(const cv::Mat & homographyMatrix)
{
	std::vector<cv::Point2f> imgPoints, planePoints;

	imgPoints.reserve(m_positions.size());
	planePoints.reserve(m_positions.size());

	for (auto position : m_positions)
	{
		imgPoints.push_back(cv::Point2f(
			(float)position.second.x(),
			(float)position.second.y()
		));
	}

	cv::perspectiveTransform(imgPoints, planePoints, homographyMatrix);

	const float multiplier = FrameProvider::m_videoMeta.FPS * 3.6f;

	std::vector<float> distances;
	distances.reserve(planePoints.size() - 1);

	for (auto && window : sliding_window(planePoints, 2))
	{
		auto distance = cv::norm(window[0] - window[1]);
		distances.push_back(distance * multiplier);
	}

	int idx = m_firstFrame + 9;
	for (auto && window : sliding_window(distances, 10))
	{
		float val = 0;

		for (auto && i : window)
			val += i;
		val /= 10;

		m_speed[idx++] = val;
	}
}

bool Vehicle::weaksFallFirst()
{
	return m_lastFrame - m_firstFrame < 20;
}

//QPoint Vehicle::getPositionOnFrame() const
//{
//	return QPoint();
//}



//std::vector<QPoint> Vehicle::getAllPositions() const
//{
//	std::vector<QPoint> positions;
//	std::transform(
//		m_positions.begin(),
//		m_positions.end(),
//		std::back_inserter(positions),
//		[](const std::map<int, QPoint>::value_type & pair) { return pair.second; });
//
//	return positions;
//}

//QLineF Vehicle::getPathSegment(const int & frameIdx)
//{
//	try
//	{
//		return QLineF(m_positions[frameIdx - 1], m_positions[frameIdx]);
//	}
//	catch (const std::out_of_range & ex)
//	{
//		return QLineF();
//	}
//}

//std::vector<std::pair<int, QLineF>> Vehicle::getVehiclePath()
//{
//	return m_trajectory;
//}

inline void Vehicle::initTracker(const cv::Mat & frame, const QRectF & detection)
{
	const int x = detection.x() * frame.cols;
	const int y = detection.y() * frame.rows;
	const int width = detection.width() * frame.cols;
	const int height = detection.height() * frame.rows;

	const cv::Rect2i det(x, y, width, height);
	
	m_tracker.release();
	//m_tracker = cv::TrackerKCF::create();
	//m_tracker = cv::TrackerCSRT::create();
	m_tracker = cv::TrackerMOSSE::create();
	m_tracker->init(frame, det);
}

inline void Vehicle::initKalmanFilter(cv::Ptr<cv::tracking::UnscentedKalmanFilter> & kalmanFilter)
{
	const int MP = 2;
	const int DP = 6;
	const int CP = 0;

	cv::Mat processNoiseCov = cv::Mat::zeros(DP, DP, CV_32FC1);  // Q
	processNoiseCov.at<float>(0, 0) = 0.0001f;
	processNoiseCov.at<float>(1, 1) = 0.0001f;
	processNoiseCov.at<float>(2, 2) = 0.0001f;
	processNoiseCov.at<float>(3, 3) = 0.0001f;
	processNoiseCov.at<float>(4, 4) = 0.0001f;
	processNoiseCov.at<float>(5, 5) = 0.0001f;

	cv::Mat measurementNoiseCov = cv::Mat::zeros(MP, MP, CV_32FC1);  // R
	measurementNoiseCov.at<float>(0, 0) = 0.1;
	measurementNoiseCov.at<float>(1, 1) = 0.1;

	cv::Mat errorCov = cv::Mat::zeros(DP, DP, CV_32FC1);  // P
	errorCov.at<float>(0, 0) = 0.1f;
	errorCov.at<float>(1, 1) = 0.1f;
	errorCov.at<float>(2, 2) = 0.1f;
	errorCov.at<float>(3, 3) = 0.1f;
	errorCov.at<float>(4, 4) = 0.1f;
	errorCov.at<float>(5, 5) = 0.1f;

	const QPointF velocity = (m_detections[m_firstFrame + 19].center() - m_detections[m_firstFrame].center()) / 20;
	const float pos_X = m_detections[m_firstFrame].center().x() * FrameProvider::m_videoMeta.WIDTH;
	const float pos_Y = m_detections[m_firstFrame].center().y() * FrameProvider::m_videoMeta.HEIGHT;

	cv::Mat initState(DP, 1, CV_32FC1);
	initState.at<float>(0, 0) = pos_X;										// center X
	initState.at<float>(1, 0) = pos_Y;										// center Y
	initState.at<float>(2, 0) = velocity.x();								// velocicy X
	initState.at<float>(3, 0) = velocity.y();								// velocity Y
	initState.at<float>(4, 0) = 0.f;										// acceleration X
	initState.at<float>(5, 0) = 0.f;										// acceleration Y

	cv::Ptr<AcceleratedModel> model(new AcceleratedModel());
	cv::tracking::UnscentedKalmanFilterParams params(DP, MP, CP, 0, 0, model);
	params.dataType = CV_32FC1;
	params.stateInit = initState.clone();
	params.errorCovInit = errorCov.clone();
	params.measurementNoiseCov = measurementNoiseCov.clone();
	params.processNoiseCov = processNoiseCov.clone();

	params.alpha = 1;
	params.beta = 2.0;
	params.k = -2.0;

	kalmanFilter = cv::tracking::createUnscentedKalmanFilter(params);
}

inline void Vehicle::addDetection(const uint & frameIdx, const Detection & detection)
{
	m_detections[frameIdx] = detection;
	m_lastFrame = frameIdx;
}

inline void Vehicle::stopTracking()
{
	m_isTracked = false;
	m_tracker.release();
}
