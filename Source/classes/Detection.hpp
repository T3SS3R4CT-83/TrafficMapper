#pragma once

#include <opencv2/core/types.hpp>
//#include <QPainter>
//#include <QPen>
#include <QPoint>

#include <TrafficMapper/Globals>

class Detection : public cv::Rect2d
{
public:
	Detection();
	Detection(int _x, int _y, int _width, int _height, VehicleType _vehicleClass, float _confidence);
	//Detection(const Detection &old);
	Detection(const cv::Rect2d &old, VehicleType vehicleClass = VehicleType::undefined, float confidence = 0.f);

	VehicleType classID() const;
	float confidence() const;

	Detection &operator=(const cv::Rect2d &old);
//	bool operator==(const Detection &rhs) const;
	friend std::ostream &operator<<(std::ostream &os, const Detection &det);
	friend std::istream &operator>>(std::istream &is, Detection &det);
	

	VehicleType vehicleClass() const;
//	float confidence() const;
//	void setConfidence(float confidence);
	QPoint getCenter() const;

	bool deletable() const;
	void markToDelete();

	static float iou(const cv::Rect2d &lhs, const cv::Rect2d &rhs);

protected:
    VehicleType m_vehicleClass;
    float m_confidence;

private:
	bool m_deletable;
};
