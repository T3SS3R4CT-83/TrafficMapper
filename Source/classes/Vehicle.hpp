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

class Vehicle : public QObject
{
	Q_OBJECT

	std::map<int, Detection> m_detections;
	std::map<int, QPoint> m_positions;

	cv::Ptr<cv::Tracker> m_tracker;
	cv::Ptr<cv::tracking::UnscentedKalmanFilter> m_kalmanFilter;

	bool m_isActive;
	int m_timeSinceLastHit;
	int m_lastTrackedFrameIdx;
	int m_vehicleClass;

public:
	Vehicle(const cv::Mat &frame, const int frameIdx, Detection &detection);

	Detection detection(const int frameIdx) const;
//	void setDetection(const int frameIdx, const Detection &detection);
	QPoint position(const int frameIdx) const;
	QString className() const;
//	int classID() const;
	bool isActive() const;
//	bool isValid() const;
	
	//const bool trackPosition(const cv::Mat &frame, const cv::Mat &prevFrame, const int frameIdx);
	const bool trackPosition(FrameProvider& video, const int frameIdx);
	const void updatePosition(const cv::Mat &frame, const int frameIdx, const Detection &detection);

	void calcVehicleType();
	std::vector<QPoint> getAllPositions() const;
//	std::map<int, QPoint> getVehiclePath();

private:
	inline void kalmanUpdate(const int frameIdx);
	inline void initTracker(const cv::Mat &frame, const cv::Rect2d &detection);
	inline void deactivate();
};
