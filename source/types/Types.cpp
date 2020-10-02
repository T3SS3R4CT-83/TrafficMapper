#include "Types.hpp"

#include <string>

VideoMeta::VideoMeta() :
	WIDTH(0),
	HEIGHT(0),
	FPS(0.f),
	LENGTH(0),
	LENGTH_MIN(0),
	LENGTH_SEC(0),
	FRAMECOUNT(0)
{
}

VideoMeta::VideoMeta(const VideoMeta & other) :
	WIDTH(other.WIDTH),
	HEIGHT(other.HEIGHT),
	FPS(other.FPS),
	LENGTH(other.LENGTH),
	LENGTH_MIN(other.LENGTH_MIN),
	LENGTH_SEC(other.LENGTH_SEC),
	FRAMECOUNT(other.FRAMECOUNT)
{
}

VideoMeta & VideoMeta::operator=(const VideoMeta & other)
{
	WIDTH		= other.WIDTH;
	HEIGHT		= other.HEIGHT;
	FPS			= other.FPS;
	LENGTH		= other.LENGTH;
	LENGTH_MIN	= other.LENGTH_MIN;
	LENGTH_SEC	= other.LENGTH_SEC;
	FRAMECOUNT	= other.FRAMECOUNT;

	return *this;
}

VideoMeta & VideoMeta::operator=(VideoMeta && other)
{
	WIDTH		= std::move(other.WIDTH);
	HEIGHT		= std::move(other.HEIGHT);
	FPS			= std::move(other.FPS);
	LENGTH		= std::move(other.LENGTH);
	LENGTH_MIN	= std::move(other.LENGTH_MIN);
	LENGTH_SEC	= std::move(other.LENGTH_SEC);
	FRAMECOUNT	= std::move(other.FRAMECOUNT);

	return *this;
}

void VideoMeta::clear()
{
	WIDTH = 0;
	HEIGHT = 0;
	FPS = 0.f;
	LENGTH = 0;
	LENGTH_MIN = 0;
	LENGTH_SEC = 0;
	FRAMECOUNT = 0;
}

std::istream & operator>>(std::istream & iStream, VehicleType & vType)
{
	std::string value;
	iStream >> value;
	vType = VehicleType(std::stoi(value));

	return iStream;
}

std::ostream & operator<<(std::ostream & oStream, const VehicleType & vType)
{
	oStream << static_cast<int>(vType);

	return oStream;
}

VehicleType TrafficMapper::getVTypeByID(const int & i)
{
	switch (i)
	{
	case 0:
		return VehicleType::CAR;
	case 1:
		return VehicleType::BUS;
	case 2:
		return VehicleType::TRUCK;
	case 3:
		return VehicleType::MOTORCYCLE;
	case 4:
		return VehicleType::BICYCLE;
	default:
		return VehicleType::undefined;
	}
}

QString TrafficMapper::getVNameByType(const VehicleType & type)
{
	switch (type)
	{
	case VehicleType::BICYCLE:
		return QStringLiteral("Bicycle");
	case VehicleType::BUS:
		return QStringLiteral("Bus");
	case VehicleType::CAR:
		return QStringLiteral("Car");
	case VehicleType::MOTORCYCLE:
		return QStringLiteral("Motorcycle");
	case VehicleType::TRUCK:
		return QStringLiteral("Truck");
	default:
		return QStringLiteral("unidentified");
	}
}
