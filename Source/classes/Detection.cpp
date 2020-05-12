#include "Detection.hpp"

#include <QPoint>


// ========================================
//             CONSTRUCTORS
// ========================================

Detection::Detection() : cv::Rect2d()
{
	m_vehicleType = VehicleType::undefined;
	m_confidence = 0;
}

Detection::Detection(const size_t &x, const size_t &y, const size_t &width, const size_t &height, const VehicleType &vehicleType, const float &confidence)
	: cv::Rect2d(x, y, width, height), m_vehicleType(vehicleType), m_confidence(confidence) { }

Detection::Detection(const cv::Rect2d & old, const VehicleType & vehicleClass, const float & confidence)
	: cv::Rect2d(old), m_vehicleType(vehicleClass), m_confidence(confidence) { }



// ========================================
//               OPERATORS
// ========================================

Detection Detection::operator=(const cv::Rect2d & old)
{
	Detection detection(old);

	return detection;
}

std::ostream & operator<<(std::ostream & oStream, const Detection & det)
{
	oStream << det.m_vehicleType << ' ' << det.m_confidence << ' ' << det.x << ' ' << det.y << ' ' << det.width << ' ' << det.height << '\n';

	return oStream;
}

std::istream & operator>>(std::istream & iStream, Detection & det)
{
	iStream >> det.m_vehicleType >> det.m_confidence >> det.x >> det.y >> det.width >> det.height;

	return iStream;
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

float Detection::iou(const cv::Rect2d & rect_1, const cv::Rect2d & rect_2)
{
	const cv::Rect2d intersection = rect_1 & rect_2;
	const float area_int = intersection.area();

	if (area_int == 0) return 0.f;

	if (intersection.area() == rect_1.area() || intersection.area() == rect_2.area())
		return 1.f;

	const float area_rect_1 = rect_1.area();
	const float area_rect_2 = rect_2.area();

	return area_int / (area_rect_1 + area_rect_2 - area_int);
}
