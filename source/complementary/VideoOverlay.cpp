#include "VideoOverlay.hpp"

#include <TrafficMapper/Complementary/VideoFilter>
#include <TrafficMapper/Modules/GateModel>

VideoOverlay::VideoOverlay(QObject * parent)
	: QAbstractVideoFilter(parent), m_vehicleModel_ptr(nullptr)
{
}

QVideoFilterRunnable * VideoOverlay::createFilterRunnable()
{
	VideoFilter * filter_ptr = new VideoFilter(m_vehicleModel_ptr);

	QObject::connect(filter_ptr, &VideoFilter::frameDisplayed,
		m_gateModel_ptr, &GateModel::onFrameDisplayed);

	return filter_ptr;
}

void VideoOverlay::setVehicleModel(VehicleModel * vehicleModel_ptr)
{
	m_vehicleModel_ptr = vehicleModel_ptr;
}

void VideoOverlay::setGateModel(GateModel * gateModel_ptr)
{
	m_gateModel_ptr = gateModel_ptr;
}
