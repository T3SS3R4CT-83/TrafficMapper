#pragma once

#include <opencv2/core/types.hpp>
//#include <QPainter>
//#include <QPen>
#include <QPoint>

#include <TrafficMapper/Globals>

class Detection : public cv::Rect2d
{
protected:
    VehicleType m_vehicleType;
    float m_confidence;

public:
	Detection();
	Detection(const int _x, const int _y, const int _width, const int _height, const VehicleType _vehicleType = VehicleType::undefined, const float _confidence = 0.f);
	Detection(const cv::Rect2d & _old, const VehicleType _vehicleClass = VehicleType::undefined, const float _confidence = 0.f);

	Detection &operator=(const cv::Rect2d &_old);
	friend std::ostream &operator<<(std::ostream &_os, const Detection &_det);
	friend std::istream &operator>>(std::istream &_is, Detection &_det);

	VehicleType vehicleType() const;
	float confidence() const;
	
	QPoint getCenter() const;

	static float iou(const cv::Rect2d &lhs, const cv::Rect2d &rhs);
};
