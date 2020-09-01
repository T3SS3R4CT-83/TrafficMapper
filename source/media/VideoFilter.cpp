#include "VideoFilter.hpp"

#include <TrafficMapper/Media/MediaPlayer>
#include <TrafficMapper/Modules/VehicleModel>
#include <TrafficMapper/Types/Vehicle>
#include <TrafficMapper/Types/Detection>

VideoFilter::VideoFilter(VehicleModel * vehicleModel_ptr)
	: m_vehicleModel_ptr(vehicleModel_ptr)
{
	m_pen_trajectory.setColor(QColor("#EEEEEE"));
	m_pen_trajectory.setWidth(3);

	m_pen_position.setColor(QColor("#CD5555"));
	m_pen_position.setWidth(5);

	m_pen_detection.setWidth(3);

	m_pen_label.setColor("#FFFFFF");

	m_playbackSettings[0] = 1;
	m_playbackSettings[1] = 1;
	m_playbackSettings[2] = 1;
	m_playbackSettings[3] = 1;
}

QVideoFrame VideoFilter::run(QVideoFrame * input, const QVideoSurfaceFormat & surfaceFormat, RunFlags flags)
{
	if (input->map(QAbstractVideoBuffer::ReadOnly))
	{
		QImage frame(
			input->bits(),
			input->width(),
			input->height(),
			input->bytesPerLine(),
			QVideoFrame::imageFormatFromPixelFormat(input->pixelFormat())
		);

		const int frameIdx = std::round(input->startTime() * 0.000001 * MediaPlayer::m_videoMeta.FPS);

		auto vehicles_ptr = &(m_vehicleModel_ptr->m_vehicleMap[frameIdx]);

		m_painter.begin(&frame);
		for (auto vehicle_ptr : *vehicles_ptr)
			vehicle_ptr->drawOnFrame(m_painter, frameIdx, m_playbackSettings);
		m_painter.end();

		emit frameDisplayed(frameIdx);
	}

	input->unmap();

	return *input;
}

void VideoFilter::onPlaybackOptionsSet(bool det, bool path, bool label, bool pos)
{
	m_playbackSettings[0] = det;
	m_playbackSettings[1] = path;
	m_playbackSettings[2] = label;
	m_playbackSettings[3] = pos;
}
