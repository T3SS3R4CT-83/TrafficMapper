#include "Detection.hpp"

//#include <QPoint>
//#include <TrafficMapper/Classes/Vehicle>


// ========================================
//             CONSTRUCTORS
// ========================================

Detection::Detection()
	: QRectF(), m_vehicleType(VehicleType::undefined), m_confidence(0), m_deletable(false) { }

Detection::Detection(const float & x, const float & y, const float & width, const float & height, const VehicleType & vehicleType, const float & confidence)
	: QRectF(x, y, width, height), m_vehicleType(vehicleType), m_confidence(confidence), m_deletable(false) { }

//Detection::Detection(const cv::Rect2d & old, const VehicleType & vehicleClass, const float & confidence)
//	: cv::Rect2d(old), m_vehicleType(vehicleClass), m_confidence(confidence), m_deletable(false) { }



// ========================================
//               OPERATORS
// ========================================

//Detection Detection::operator=(const cv::Rect2d & old)
//{
//	Detection detection(old);
//
//	return detection;
//}
//
//std::ostream & operator<<(std::ostream & oStream, const Detection & det)
//{
//	oStream << det.m_vehicleType << ' ' << det.m_confidence << ' ' << det.x << ' ' << det.y << ' ' << det.width << ' ' << det.height << '\n';
//
//	return oStream;
//}

std::istream & operator>>(std::istream & iStream, Detection & det)
{
	float x, y, width, height;

	iStream >> det.m_vehicleType >> det.m_confidence >> x >> y >> width >> height;

	det.setX(x);
	det.setY(y);
	det.setWidth(width);
	det.setHeight(height);

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

void Detection::markToDelete()
{
	m_deletable = true;
}

bool Detection::isDeletable() const
{
	return m_deletable;
}



// ========================================
//            STATIC FUNCTIONS
// ========================================

float Detection::iou(const QRectF & rect_1, const QRectF & rect_2)
{
	const QRectF intersection = rect_1 & rect_2;
	const float area_int = intersection.width() * intersection.height();

	if (area_int == 0) return 0.f;

	const float area_rect_1 = rect_1.width() * rect_1.height();
	const float area_rect_2 = rect_2.width() * rect_2.height();

	if (area_int == area_rect_1 || area_int == area_rect_2)
		return 1.f;

	return area_int / (area_rect_1 + area_rect_2 - area_int);
}
