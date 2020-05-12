#pragma once

#include <opencv2/tracking/tracker.hpp>
#include <opencv2/tracking/kalman_filters.hpp>

#include <QObject>


class Detection;
enum class VehicleType : int;


class Vehicle : public QObject
{
	Q_OBJECT

	std::map<int, Detection> m_detections;
	std::map<int, QPoint> m_positions;
	std::vector<std::pair<int, QLineF>> m_path;
	VehicleType m_vehicleClass;

	cv::Ptr<cv::Tracker> m_tracker;
	cv::Ptr<cv::tracking::UnscentedKalmanFilter> m_kalmanFilter;

	int m_timeSinceLastHit;
	int m_lastTrackedFrameIdx;
	bool m_isTracked;

public:
	Vehicle(const int & frameIdx, const Detection & detection);

	Detection detection(const int & frameIdx) const;
	Detection getLastDetection() const;
	QPoint position(const int & frameIdx) const;
	VehicleType vehicleClass();
	QString className() const;
	bool isTracked() const;
	void stopTracking();
	
	const bool trackPosition(const cv::Mat & frame, const cv::Mat & prevFrame, const int & frameIdx);
	const void updatePosition(const int & frameIdx, const Detection & detection);

	void calcVehicleType();
	std::vector<QPoint> getAllPositions() const;
	QLineF getPathSegment(const int & frameIdx);
	std::vector<std::pair<int, QLineF>> getVehiclePath();

private:
	inline void deactivate();
	inline void initTracker(const cv::Mat & frame, const cv::Rect2d & detection);
	inline void kalmanUpdate(const int & frameIdx);
};
