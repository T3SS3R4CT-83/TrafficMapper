#pragma once

//#include <opencv2/core/types.hpp>

#include <QRectF>

#include <TrafficMapper/Types/Vehicle>
#include <TrafficMapper/Types/Types>


//class QPoint;


class Detection : public QRectF
{
	VehicleType m_vehicleType;
    float m_confidence;
	bool m_deletable;

public:

	Detection();
	Detection(const float & x, const float & y, const float & width, const float & height,
		const VehicleType & vehicleType = VehicleType::undefined, const float & confidence = 0.f);
	//Detection(const cv::Rect2d & old, const VehicleType & vehicleClass = VehicleType::undefined, const float & confidence = 0.f);

	//Detection operator=(const cv::Rect2d & old);
	//friend std::ostream & operator<<(std::ostream & oStream, const Detection & det);
	friend std::istream & operator>>(std::istream & iStream, Detection & det);

	VehicleType vehicleType() const;
	float confidence() const;
	
	void markToDelete();
	bool isDeletable() const;
	bool isValid() const;
	
	static float iou(const QRectF & rect_1, const QRectF & rect_2);
};
