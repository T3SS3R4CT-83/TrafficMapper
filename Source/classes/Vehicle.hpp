#pragma once

//#include <map>

#include <opencv2/core/mat.hpp>
//#include <opencv2/core/cvstd.hpp>
#include <opencv2/tracking/tracker.hpp>
//#include <opencv2/video/tracking.hpp>
#include <opencv2/tracking/kalman_filters.hpp>
#include <QObject>
//#include <QString>

#include <TrafficMapper/Classes/Detection>
#include <TrafficMapper/Modules/FrameProvider>

class QPoint;
class QLineF;

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

public:
	bool m_isTracked;
	Vehicle(const int frameIdx, const Detection &detection);

	Detection detection(const int frameIdx) const;
	Detection getLastDetection() const;
//	void setDetection(const int frameIdx, const Detection &detection);
	QPoint position(const int frameIdx) const;
	VehicleType vehicleClass();
	QString className() const;
//	int vehicleType() const;
	bool isTracked() const;
//	bool isValid() const;
	
	const bool trackPosition(const cv::Mat &_frame, const cv::Mat &_prevFrame, const int _frameIdx);
	//const bool trackPosition(FrameProvider& video, const int frameIdx);
	const void updatePosition(const int frameIdx, const Detection &detection);

	void calcVehicleType();
	std::vector<QPoint> getAllPositions() const;
	QLineF getPathSegment(const int _frameIdx);
	std::vector<std::pair<int, QLineF>> getVehiclePath();


private:
	inline void deactivate();
	inline void initTracker(const cv::Mat &frame, const cv::Rect2d &detection);
	inline void kalmanUpdate(int frameIdx);
};
