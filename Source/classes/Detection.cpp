#include "Detection.hpp"

#include <algorithm>


// ========================================
//             CONSTRUCTORS
// ========================================

Detection::Detection()
	: cv::Rect2d()
{
	m_vehicleType = VehicleType::undefined;
	m_confidence = 0;
}

Detection::Detection(const int _x, const int _y, const int _width, const int _height, const VehicleType _vehicleType, const float _confidence)
	: cv::Rect2d(_x, _y, _width, _height), m_vehicleType(_vehicleType), m_confidence(_confidence) { }

Detection::Detection(const cv::Rect2d & _old, const VehicleType _vehicleClass, const float _confidence)
	: cv::Rect2d(_old), m_vehicleType(_vehicleClass), m_confidence(_confidence) { }



// ========================================
//               OPERATORS
// ========================================

Detection &Detection::operator=(const cv::Rect2d &_old)
{
	Detection detection(_old);

	return detection;
}

std::ostream &operator<<(std::ostream &_os, const Detection &_det)
{
	_os << _det.m_vehicleType << " " << _det.m_confidence << " " << _det.x << " " << _det.y << " " << _det.width << " " << _det.height << '\n';
	
	return _os;
}

std::istream &operator>>(std::istream &_is, Detection &_det)
{
	_is >> _det.m_vehicleType >> _det.m_confidence >> _det.x >> _det.y >> _det.width >> _det.height;

	return _is;
}



// ========================================
//           GETTERS & SETTERS
// ========================================

VehicleType Detection::vehicleType() const
{
	return m_vehicleType;
}

float Detection::confidence() const
{
	return m_confidence;
}



// ========================================
//            PUBLIC FUNCTIONS
// ========================================

QPoint Detection::getCenter() const
{
	return QPoint(x + width * 0.5f, y + height * 0.5f);
}



// ========================================
//            STATIC FUNCTIONS
// ========================================

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
