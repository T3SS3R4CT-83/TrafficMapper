#pragma once

#include <bitset>

#include <opencv2/tracking/tracker.hpp>
//#include <opencv2/tracking/kalman_filters.hpp>
#include <opencv2/video/tracking.hpp>

#include <QObject>
#include <QLineF>
#include <QPainter>


class Detection;
struct VideoMeta;
enum class VehicleType : int;


class Vehicle : public QObject
{
	Q_OBJECT

	std::map<int, Detection> m_detections;
	std::map<int, QPointF> m_positions;
	std::vector<std::pair<uint, QLineF>> m_trajectory;
	std::map<int, int> m_speed;
	VehicleType m_vehicleClass;

	cv::Ptr<cv::Tracker> m_tracker;

	bool m_isTracked;
	uint m_timeSinceLastHit;
	uint m_firstFrame;
	uint m_lastFrame;

	friend class VehicleModel;
	friend class StatModel;
	friend class VideoFilter;
	friend class MediaPlayer; // TODO: Remove later

public:

	Vehicle(const int & frameIdx, const Detection & detection);

	Detection getDetection(const int & frameIdx) const;
	std::vector<std::pair<uint, QLineF>> getTrajectory() const;

	bool isTracked() const;
	
	void updatePosition(const int & frameIdx, const Detection & detection);
	void trackPosition(const cv::Mat & frame, const cv::Mat & prevFrame, const int & frameIdx);

	//void track(const size_t & frameIdx, const cv::Mat & frame, const cv::Mat & prevFrame, const Detection & detection);

	void calcVehicleType();
	void calcPositions();
	void calcVehicleSpeed(const cv::Mat & homographyMatrix);

	bool weaksFallFirst();

	void drawOnFrame(QPainter & painter, const size_t & frameIdx, const std::bitset<4> & options);

private:

	inline void initTracker(const cv::Mat & frame, const QRectF & detection);
	inline void initKalmanFilter(cv::KalmanFilter & kalmanFilter);
	//inline void initKalmanFilter_unscented(cv::Ptr<cv::tracking::UkfSystemModel> kalmanFilter);

	inline void addDetection(const uint & frameIdx, const Detection & detection);
	inline void stopTracking();
};
