#pragma once

#include <opencv2/core/types.hpp>

#include <TrafficMapper/Types>


enum class VehicleType : int;
class QPoint;


class Detection : public cv::Rect2d
{
protected:
    VehicleType m_vehicleType;
    float m_confidence;

public:
	Detection();
	Detection(const size_t & x, const size_t & y, const size_t & width, const size_t & height, const VehicleType & vehicleType = VehicleType::undefined, const float & confidence = 0.f);
	Detection(const cv::Rect2d & old, const VehicleType & vehicleClass = VehicleType::undefined, const float & confidence = 0.f);

	Detection operator=(const cv::Rect2d & old);
	friend std::ostream & operator<<(std::ostream & oStream, const Detection & det);
	friend std::istream & operator>>(std::istream & iStream, Detection & det);

	VehicleType vehicleType() const;
	float confidence() const;
	
	QPoint getCenter() const;

	static float iou(const cv::Rect2d & rect_1, const cv::Rect2d & rect_2);
};
