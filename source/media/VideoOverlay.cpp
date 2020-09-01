#include "VideoOverlay.hpp"

#include <TrafficMapper/Media/VideoFilter>
#include <TrafficMapper/Modules/GateModel>



VideoOverlay::VideoOverlay(QObject * parent)
	: QAbstractVideoFilter(parent), m_filter_ptr(nullptr), m_vehicleModel_ptr(nullptr), m_gateModel_ptr(nullptr)
{
}

VideoOverlay::VideoOverlay(const VideoOverlay & other)
{
	m_filter_ptr = other.m_filter_ptr;
	m_vehicleModel_ptr = other.m_vehicleModel_ptr;
	m_gateModel_ptr = other.m_gateModel_ptr;
}



QVideoFilterRunnable * VideoOverlay::createFilterRunnable()
{
	m_filter_ptr = new VideoFilter(m_vehicleModel_ptr);

	QObject::connect(m_filter_ptr, &VideoFilter::frameDisplayed,
		m_gateModel_ptr, &GateModel::onFrameDisplayed);
	QObject::connect(this, &VideoOverlay::setPlaybackOptions,
		m_filter_ptr, &VideoFilter::onPlaybackOptionsSet);

	return m_filter_ptr;
}



void VideoOverlay::setVehicleModel(VehicleModel * vehicleModel_ptr)
{
	m_vehicleModel_ptr = vehicleModel_ptr;
}

void VideoOverlay::setGateModel(GateModel * gateModel_ptr)
{
	m_gateModel_ptr = gateModel_ptr;
}
