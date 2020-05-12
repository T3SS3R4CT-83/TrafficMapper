#include "Vehicle.hpp"

#include <QLineF>

#include <TrafficMapper/Globals>
#include <TrafficMapper/Types>
#include <TrafficMapper/Classes/Detection>


class AcceleratedModel : public cv::tracking::UkfSystemModel
{
	float m_deltaTime;

public:
	AcceleratedModel(float deltaTime)
		: cv::tracking::UkfSystemModel(), m_deltaTime(deltaTime) { }

	void stateConversionFunction(const cv::Mat &x_k, const cv::Mat &u_k, const cv::Mat &v_k, cv::Mat &x_kplus1)
	{
		float x0 = x_k.at<float>(0, 0);
		float y0 = x_k.at<float>(1, 0);
		float vx0 = x_k.at<float>(2, 0);
		float vy0 = x_k.at<float>(3, 0);
		float ax0 = x_k.at<float>(4, 0);
		float ay0 = x_k.at<float>(5, 0);

		x_kplus1.at<float>(0, 0) = x0 + vx0 * m_deltaTime + ax0 * pow(m_deltaTime, 2) / 2;
		x_kplus1.at<float>(1, 0) = y0 + vy0 * m_deltaTime + ay0 * pow(m_deltaTime, 2) / 2;
		x_kplus1.at<float>(2, 0) = vx0 + ax0 * m_deltaTime;
		x_kplus1.at<float>(3, 0) = vy0 + ay0 * m_deltaTime;
		x_kplus1.at<float>(4, 0) = ax0;
		x_kplus1.at<float>(5, 0) = ay0;

		if (v_k.size() == u_k.size()) {
			x_kplus1 += v_k + u_k;
		} else {
			x_kplus1 += v_k;
		}
	}

	void measurementFunction(const cv::Mat &x_k, const cv::Mat &n_k, cv::Mat &z_k)
	{
		float x0 = x_k.at<float>(0, 0);
		float y0 = x_k.at<float>(1, 0);
		float vx0 = x_k.at<float>(2, 0);
		float vy0 = x_k.at<float>(3, 0);
		float ax0 = x_k.at<float>(4, 0);
		float ay0 = x_k.at<float>(5, 0);

		float new_X = x0 + vx0 * m_deltaTime + ax0 * pow(m_deltaTime, 2) / 2 + n_k.at<float>(0, 0);
		float new_Y = y0 + vy0 * m_deltaTime + ay0 * pow(m_deltaTime, 2) / 2 + n_k.at<float>(1, 0);

		z_k.at<float>(0, 0) = new_X;
		z_k.at<float>(1, 0) = new_Y;
	}
};


Vehicle::Vehicle(const int & frameIdx, const Detection & detection)
{
	m_detections[frameIdx] = detection;
	m_positions[frameIdx] = detection.getCenter();
	m_vehicleClass = VehicleType::undefined;
	m_isTracked = true;
	m_timeSinceLastHit = 0;

	//const float deltaTime = 1 / 30.f;
	const float deltaTime = 0.2f;

	int MP = 2;
	int DP = 6;
	int CP = 0;

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
	errorCov.at<float>(0, 0) = 0.1;
	errorCov.at<float>(1, 1) = 0.1;
	errorCov.at<float>(2, 2) = 0.1;
	errorCov.at<float>(3, 3) = 0.1;
	errorCov.at<float>(4, 4) = 0.1;
	errorCov.at<float>(5, 5) = 0.1;

	cv::Mat initState(DP, 1, CV_32FC1);
	initState.at<float>(0, 0) = detection.getCenter().x();	// center X
	initState.at<float>(1, 0) = detection.getCenter().y();	// center Y
	initState.at<float>(2, 0) = 0.f;						// velocicy X
	initState.at<float>(3, 0) = 0.f;						// velocity Y
	initState.at<float>(4, 0) = 0.f;						// acceleration X
	initState.at<float>(5, 0) = 0.f;						// acceleration Y

	cv::Ptr<AcceleratedModel> model(new AcceleratedModel(deltaTime));
	cv::tracking::UnscentedKalmanFilterParams params(DP, MP, CP, 0, 0, model);
	params.dataType = CV_32FC1;
	params.stateInit = initState.clone();
	params.errorCovInit = errorCov.clone();
	params.measurementNoiseCov = measurementNoiseCov.clone();
	params.processNoiseCov = processNoiseCov.clone();

	params.alpha = 1;
	params.beta = 2.0;
	params.k = -2.0;

	m_kalmanFilter = cv::tracking::createUnscentedKalmanFilter(params);
}



Detection Vehicle::detection(const int & frameIdx) const
{
	try
	{
		return m_detections.at(frameIdx);
	}
	catch (const std::out_of_range & ex)
	{
		return Detection();
	}
}

Detection Vehicle::getLastDetection() const
{
	return m_detections.rbegin()->second;
}

QPoint Vehicle::position(const int & frameIdx) const
{
	try
	{
		return m_positions.at(frameIdx);
	}
	catch (const std::out_of_range & ex)
	{
		return QPoint();
	}
}

VehicleType Vehicle::vehicleClass()
{
	return m_vehicleClass;
}

QString Vehicle::className() const
{
	try
	{
		return Settings::DETECTOR_CLASSES.at(m_vehicleClass);
	}
	catch (const std::out_of_range & ex)
	{
		return QString("undefined");
	}
}

bool Vehicle::isTracked() const
{
	return m_isTracked;
}

void Vehicle::stopTracking()
{
	m_isTracked = false;
}

inline void Vehicle::deactivate()
{
	m_tracker.release();
	m_isTracked = false;
}

inline void Vehicle::initTracker(const cv::Mat & frame, const cv::Rect2d & detection)
{
	m_tracker.release();
	m_tracker = (*Settings::TRACKER_ALGORITHM)();
	m_tracker->init(frame, detection);
}

const bool Vehicle::trackPosition(const cv::Mat & frame, const cv::Mat & prevFrame, const int & frameIdx)
{
	cv::Rect2d newTrack;
	//cv::Mat frame;

	if (m_tracker.empty())
	{
		initTracker(prevFrame, m_detections[frameIdx - 1]);
	}

	if (m_tracker->update(frame, newTrack))
	{
		//if (newTrack.x < 10
		//	|| newTrack.y < 10
		//	|| newTrack.x + newTrack.width > GlobalMeta::getInstance()->VIDEO_WIDTH() - 10
		//	|| newTrack.y + newTrack.height > GlobalMeta::getInstance()->VIDEO_HEIGHT() - 10)
		//{
		//	deactivate();
		//	return false;
		//}

		m_detections[frameIdx] = Detection(newTrack);
		kalmanUpdate(frameIdx);
		if (++m_timeSinceLastHit == Settings::TRACKER_VISUAL_TRACKING_LENGTH)
			deactivate();
		return true;
	}
	else
	{
		deactivate();
		return false;
	}
}

const void Vehicle::updatePosition(const int & frameIdx, const Detection & detection)
{
	if (!m_tracker.empty())
	{
		m_tracker.release();
		m_timeSinceLastHit = 0;
	}

	m_detections[frameIdx] = detection;

	kalmanUpdate(frameIdx);
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

	for (auto detection : m_detections)
	{
		VehicleType vehicleType = detection.second.vehicleType();
		float confidence = detection.second.confidence();

		if (vehicleType != VehicleType::undefined)
		{
			sumConf[vehicleType] += confidence;
		}
	}

	auto max_element = std::max_element(std::begin(sumConf), std::end(sumConf),
		[](const decltype(sumConf)::value_type & p1, const decltype(sumConf)::value_type & p2) {
			return p1.second < p2.second;
		});

	m_vehicleClass = max_element->first;
}

std::vector<QPoint> Vehicle::getAllPositions() const
{
	std::vector<QPoint> positions;
	std::transform(
		m_positions.begin(),
		m_positions.end(),
		std::back_inserter(positions),
		[](const std::map<int, QPoint>::value_type & pair) { return pair.second; });

	return positions;
}

QLineF Vehicle::getPathSegment(const int & frameIdx)
{
	try
	{
		return QLineF(m_positions[frameIdx - 1], m_positions[frameIdx]);
	}
	catch (const std::out_of_range & ex)
	{
		return QLineF();
	}
}

std::vector<std::pair<int, QLineF>> Vehicle::getVehiclePath()
{
	return m_path;
}

inline void Vehicle::kalmanUpdate(const int & frameIdx)
{
	m_kalmanFilter->predict();

	cv::Mat state(6, 1, CV_32F);
	cv::Mat measurement(2, 1, CV_32F);

	measurement.at<float>(0) = m_detections[frameIdx].getCenter().x();
	measurement.at<float>(1) = m_detections[frameIdx].getCenter().y();

	state = m_kalmanFilter->correct(measurement);

	float x = state.at<float>(0);

	m_positions[frameIdx] = QPoint(state.at<float>(0), state.at<float>(1));
	if (m_positions.size() > 1)
		m_path.push_back(std::make_pair(frameIdx, QLineF(m_positions[frameIdx - 1], m_positions[frameIdx])));
}
