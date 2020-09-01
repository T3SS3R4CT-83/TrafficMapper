#include "Vehicle.hpp"

#include <unordered_map>

#include <opencv2/video/tracking.hpp>

#include <TrafficMapper/Media/MediaPlayer>
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

		if (++m_timeSinceLastHit == 30)
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
	const uint vWidth = MediaPlayer::m_videoMeta.WIDTH;
	const uint vHeight = MediaPlayer::m_videoMeta.HEIGHT;

	m_positions[m_firstFrame] = QPointF(
		std::round(m_detections[m_firstFrame].center().x() * vWidth),
		std::round(m_detections[m_firstFrame].center().y() * vHeight)
	);

	//cv::Ptr<cv::tracking::UnscentedKalmanFilter> kalmanFilter;
	//initKalmanFilter_unscented(kalmanFilter);
	cv::KalmanFilter kalmanFilter(6, 2, 0);
	initKalmanFilter(kalmanFilter);

	for (int frameIdx(m_firstFrame + 1); frameIdx <= m_lastFrame; ++frameIdx)
	{
		cv::Mat measurement(2, 1, CV_32F);
		measurement.at<float>(0) = std::round(m_detections[frameIdx].center().x() * vWidth);
		measurement.at<float>(1) = std::round(m_detections[frameIdx].center().y() * vHeight);

		auto state_pre = kalmanFilter.predict();
		auto state_post = kalmanFilter.correct(measurement);

		const float ax = state_post.at<float>(4) - state_pre.at<float>(4);
		const float ay = state_post.at<float>(5) - state_pre.at<float>(5);

		kalmanFilter.statePost.at<float>(4) = ax;
		kalmanFilter.statePost.at<float>(5) = ay;

		m_positions[frameIdx] = QPointF(state_post.at<float>(0), state_post.at<float>(1));

		m_trajectory.push_back(std::make_pair(frameIdx, QLineF(m_positions[frameIdx - 1], m_positions[frameIdx])));
	}

	//kalmanFilter.release();

	m_firstFrame += 0;

	std::erase_if(m_positions, [this](const auto & item) {
		auto const & [key, value] = item;
		return key < this->m_firstFrame;
	});
	std::erase_if(m_detections, [this](const auto & item) {
		auto const & [key, value] = item;
		return key < this->m_firstFrame;
	});

	m_trajectory.reserve(m_positions.size() - 1);
	for (int frameIdx(m_firstFrame + 1); frameIdx <= m_lastFrame; ++frameIdx)
	{
		m_trajectory.push_back(std::make_pair(frameIdx, QLineF(m_positions[frameIdx - 1], m_positions[frameIdx])));
	}
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

	const float multiplier = MediaPlayer::m_videoMeta.FPS * 3.6f;

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
	return m_lastFrame - m_firstFrame < 40;
}



#include <QPen>

void Vehicle::drawOnFrame(QPainter & painter, const size_t & frameIdx, const std::bitset<4> & options)
{


	static QPen pen;
	static QFont painterFont("Arial", MediaPlayer::m_videoMeta.HEIGHT / 46, 700);
	static QFontMetrics fm(painterFont);
	static int marginSize = MediaPlayer::m_videoMeta.HEIGHT * 0.005f;
	static QMargins margin(marginSize, marginSize, marginSize, marginSize);

	if (options[1])  // TRAJECTORY
	{
		pen.setColor(QColor("#EEEEEE"));
		pen.setWidth(2);
		painter.setPen(pen);

		for (const auto & line : m_trajectory)
			painter.drawLine(line.second);
	}

	if (options[3])  // POSITION
	{
		pen.setColor(QColor("#CD5555"));
		pen.setWidth(5);
		painter.setPen(pen);

		painter.drawPoint(m_positions[frameIdx]);
	}

	if (options[2])  // LABELS
	{
		pen.setColor("#FFFFFF");
		painter.setPen(pen);
		painter.setFont(painterFont);

		//QRect labelBackground = fm.tightBoundingRect(vehicle_ptr->className());
		QRectF labelBackground(0, 0, 100, 20);
		labelBackground += margin;
		labelBackground.translate(m_positions[frameIdx]);

		painter.fillRect(labelBackground, QColor("#CD2222"));
		painter.drawText(
			m_positions[frameIdx].x() + 5,
			m_positions[frameIdx].y() + 20,
			QString::number(m_speed[frameIdx]));
		//m_painter.drawText(
		//	labelTranslate.x(),
		//	labelTranslate.y(),
		//	vehicle_ptr->className());
	}

	if (options[0])  // DETECTIONS
	{
		switch (m_detections[frameIdx].vehicleType())
		{
		case VehicleType::CAR:
			pen.setColor(QColor("blue"));
			break;
		case VehicleType::BUS:
			pen.setColor(QColor("yellow"));
			break;
		case VehicleType::TRUCK:
			pen.setColor(QColor("red"));
			break;
		case VehicleType::MOTORCYCLE:
			pen.setColor(QColor("purple"));
			break;
		case VehicleType::BICYCLE:
			pen.setColor(QColor("green"));
			break;
		default:
			pen.setColor(QColor("white"));
			break;
		}
		pen.setWidth(3);
		painter.setPen(pen);

		painter.drawRect(
			m_detections[frameIdx].x() * MediaPlayer::m_videoMeta.WIDTH,
			m_detections[frameIdx].y() * MediaPlayer::m_videoMeta.HEIGHT,
			m_detections[frameIdx].width() * MediaPlayer::m_videoMeta.WIDTH,
			m_detections[frameIdx].height() * MediaPlayer::m_videoMeta.HEIGHT
		);
	}
}



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

inline void Vehicle::initKalmanFilter(cv::KalmanFilter & kalmanFilter)
{
	const int vMaxIndex = std::min(50, static_cast<int>(m_lastFrame - m_firstFrame));
	const QPointF velocity = (m_detections[m_firstFrame + vMaxIndex].center() - m_detections[m_firstFrame].center()) / 50;
	const float pos_X = m_detections[m_firstFrame].center().x() * MediaPlayer::m_videoMeta.WIDTH;
	const float pos_Y = m_detections[m_firstFrame].center().y() * MediaPlayer::m_videoMeta.HEIGHT;

	kalmanFilter.statePost.at<float>(0, 0) = pos_X;				// center X
	kalmanFilter.statePost.at<float>(1, 0) = pos_Y;				// center Y
	kalmanFilter.statePost.at<float>(2, 0) = velocity.x();		// velocicy X
	kalmanFilter.statePost.at<float>(3, 0) = velocity.y();		// velocity Y
	kalmanFilter.statePost.at<float>(4, 0) = velocity.x() * 2;	// acceleration X
	kalmanFilter.statePost.at<float>(5, 0) = velocity.y() * 2;	// acceleration Y


	// A (State Transition Matrix)
	//kalmanFilter.transitionMatrix = (cv::Mat_<float>(4, 4) << 1, 0, 1, 0,  0, 1, 0, 1,  0, 0, 1, 0,  0, 0, 0, 1);
	cv::setIdentity(kalmanFilter.transitionMatrix);
	kalmanFilter.transitionMatrix.at<float>(0, 2) = 1.f;
	kalmanFilter.transitionMatrix.at<float>(1, 3) = 1.f;
	kalmanFilter.transitionMatrix.at<float>(2, 4) = 0.5f;
	kalmanFilter.transitionMatrix.at<float>(3, 5) = 0.5f;

	// Q (Process Covariance Matrix)
	cv::setIdentity(kalmanFilter.processNoiseCov, cv::Scalar::all(1e-5));

	// H (Measurement Matrix)
	cv::setIdentity(kalmanFilter.measurementMatrix);

	// R (Measurement Noise)
	cv::setIdentity(kalmanFilter.measurementNoiseCov, cv::Scalar::all(5));

	// P (State Covariance Matrix)
	cv::setIdentity(kalmanFilter.errorCovPost, cv::Scalar::all(1e-1));
	//kalmanFilter.errorCovPost.at<float>(0, 0) = std::pow(pos_X, 2);
	//kalmanFilter.errorCovPost.at<float>(1, 1) = std::pow(pos_Y, 2);
	kalmanFilter.errorCovPost.at<float>(2, 2) = 1;
	kalmanFilter.errorCovPost.at<float>(3, 3) = 1;
	kalmanFilter.errorCovPost.at<float>(4, 4) = 1;
	kalmanFilter.errorCovPost.at<float>(5, 5) = 1;




	//cv::Ptr<AcceleratedModel> model(new AcceleratedModel());
	//cv::tracking::UnscentedKalmanFilterParams params(DP, MP, CP, 0, 0, model);
	//params.dataType = CV_32FC1;
	//params.stateInit = initState.clone();
	//params.errorCovInit = errorCov.clone();
	//params.measurementNoiseCov = measurementNoiseCov.clone();
	//params.processNoiseCov = processNoiseCov.clone();

	//params.alpha = 1;
	//params.beta = 2.0;
	//params.k = -2.0;

	//kalmanFilter = cv::tracking::createUnscentedKalmanFilter(params);
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
