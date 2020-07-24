#pragma once

#include <iostream>

#include <QObject>

struct VideoMeta
{
	Q_GADGET

	Q_PROPERTY(int width MEMBER WIDTH)
	Q_PROPERTY(int height MEMBER HEIGHT)
	Q_PROPERTY(float fps MEMBER FPS)
	Q_PROPERTY(int length MEMBER LENGTH)
	Q_PROPERTY(int length_min MEMBER LENGTH_MIN)
	Q_PROPERTY(int length_sec MEMBER LENGTH_SEC)
	Q_PROPERTY(int framecount MEMBER FRAMECOUNT)

public:

	int WIDTH;
	int HEIGHT;
	float FPS;
	int LENGTH;
	int LENGTH_MIN;
	int LENGTH_SEC;
	int FRAMECOUNT;

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
