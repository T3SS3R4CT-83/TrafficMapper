#include "Detection.hpp"

#include <algorithm>



Detection::Detection()
{
	m_deletable = false;
}

Detection::Detection(int _x, int _y, int _width, int _height, VehicleType _vehicleClass, float _confidence)
	: cv::Rect2d(_x, _y, _width, _height), m_vehicleClass(_vehicleClass), m_confidence(_confidence) { Detection(); }

//Detection::Detection(int x, int y, int width, int height, int vehicleClass, float confidence)
//	: cv::Rect2d(x, y, width, height), m_vehicleClass(vehicleClass), m_confidence(confidence) { }

//Detection::Detection(const Detection &old)
//	: cv::Rect2d(old), m_vehicleClass(old.m_vehicleClass), m_confidence(old.m_confidence) { }

Detection::Detection(const cv::Rect2d &old, VehicleType vehicleClass, float confidence)
	: cv::Rect2d(old), m_vehicleClass(vehicleClass), m_confidence(confidence) { Detection(); }



VehicleType Detection::classID() const
{
	return m_vehicleClass;
}

float Detection::confidence() const
{
	return m_confidence;
}

Detection &Detection::operator=(const cv::Rect2d &old)
{
	Detection detection(old);

	return detection;
}

//bool Detection::operator==(const Detection &rhs) const
//{
//	if (this->x == rhs.x && this->y == rhs.y && this->width == rhs.width && this->height == rhs.height)
//		return true;
//	else
//		return false;
//}

float Detection::iou(const cv::Rect2d &lhs, const cv::Rect2d &rhs)
{
	const cv::Rect2d intersection = lhs & rhs;
	const float area_int = intersection.area();

	if (area_int == 0) return 0.f;

	if (intersection.area() == lhs.area() || intersection.area() == rhs.area())
		return 1.f;

	const float area_lhs = lhs.area();
	const float area_rhs = rhs.area();

	return area_int / (area_rhs + area_lhs - area_int);
}

VehicleType Detection::vehicleClass() const
{
	return m_vehicleClass;
}

//float Detection::confidence() const
//{
//	return m_confidence;
//}

//void Detection::setConfidence(float confidence)
//{
//	m_confidence = confidence;
//}

QPoint Detection::getCenter() const
{
	return QPoint(x + width * 0.5f, y + height * 0.5f);
}

bool Detection::deletable() const
{
	return m_deletable;
}

void Detection::markToDelete()
{
	m_deletable = true;
}

std::ostream &operator<<(std::ostream &os, const Detection &det)
{
	os << det.x << " " << det.y << " " << det.width << " " << det.height << " " << det.m_vehicleClass << " " << det.m_confidence << '\n';
	
	return os;
}

std::istream &operator>>(std::istream &is, Detection &det)
{
	is >> det.x >> det.y >> det.width >> det.height >> det.m_vehicleClass >> det.m_confidence;

	return is;
}
