#pragma once

#include <iostream>

#include <QObject>

struct VideoMeta
{
	Q_GADGET

	Q_PROPERTY(uint width MEMBER WIDTH)
	Q_PROPERTY(uint height MEMBER HEIGHT)
	Q_PROPERTY(float fps MEMBER FPS)
	Q_PROPERTY(size_t length MEMBER LENGTH)
	Q_PROPERTY(uint length_min MEMBER LENGTH_MIN)
	Q_PROPERTY(uint length_sec MEMBER LENGTH_SEC)
	Q_PROPERTY(size_t framecount MEMBER FRAMECOUNT)

public:

	uint WIDTH;
	uint HEIGHT;
	float FPS;
	size_t LENGTH;
	uint LENGTH_MIN;
	uint LENGTH_SEC;
	size_t FRAMECOUNT;

	VideoMeta();
	VideoMeta(const VideoMeta & other);

	VideoMeta & operator=(const VideoMeta & other);
	VideoMeta & operator=(VideoMeta && other);

	void clear();
};

Q_DECLARE_METATYPE(VideoMeta)

enum class VehicleType
{
	undefined = -1,
	CAR = 0,
	BUS = 1,
	TRUCK = 2,
	MOTORCYCLE = 3,
	BICYCLE = 4
};

std::istream & operator>>(std::istream & iStream, VehicleType & vType);
std::ostream & operator<<(std::ostream & oStream, const VehicleType & vType);
