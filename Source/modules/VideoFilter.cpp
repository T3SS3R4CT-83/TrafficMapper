#include "VideoFilter.hpp"

#include <QDebug>
#include <QMap>
#include <QQmlProperty>

#include <TrafficMapper/Globals>
#include <TrafficMapper/Classes/Detection>
#include <TrafficMapper/Classes/Vehicle>

VideoFilterRunnable::VideoFilterRunnable(TrafficTracker *tracker)
{
	m_tracker_ptr = tracker;
	m_globals_ptr = GlobalMeta::getInstance();

	m_pen_detection.setColor(QColor("blue"));
	m_pen_detection.setWidth(3);
	m_pen_tracking.setColor(QColor("green"));
	m_pen_tracking.setWidth(3);
	m_pen_position.setColor(QColor("#CD2222"));
	m_pen_position.setWidth(m_globals_ptr->VIDEO_HEIGHT() * 0.01f);
	m_pen_label.setColor("white");
	m_pen_trajectory.setColor(QColor(255,255,255,150));
	m_pen_trajectory.setWidth(3);
	m_painterFont = QFont("Arial", m_globals_ptr->VIDEO_HEIGHT() / 46, 700);
}

QVideoFrame VideoFilterRunnable::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags)
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

		const int frameIdx			= std::round(input->startTime() * 0.000001 * m_globals_ptr->VIDEO_FPS());
		const bool showDetections	= m_globals_ptr->PLAYER_SHOW_DETECTIONS();
		const bool showPaths		= m_globals_ptr->PLAYER_SHOW_PATHS();
		const bool showLabels		= m_globals_ptr->PLAYER_SHOW_LABELS();
		const bool showPositions	= m_globals_ptr->PLAYER_SHOW_POSITIONS();

		std::vector<Vehicle *> vehicles = m_tracker_ptr->getVehiclesOnFrame(frameIdx);

		m_painter.begin(&frame);
		{
			for (auto vehicle : vehicles)
			{
				if (showPaths) {
					m_painter.setPen(m_pen_trajectory);
					std::vector<QPoint> vehiclePositions = vehicle->getAllPositions();
					for (int i(0); i < vehiclePositions.size() - 1; ++i)
						m_painter.drawLine(vehiclePositions[i], vehiclePositions[i + 1]);
				}

				if (showPositions) {
					m_painter.setPen(m_pen_position);
					m_painter.drawPoint(vehicle->position(frameIdx));
				}

				if (showLabels) {
					static QFontMetrics fm(m_painterFont);
					QRect labelBackground = fm.tightBoundingRect(vehicle->className());
					
					static int marginSize = m_globals_ptr->VIDEO_HEIGHT() * 0.005f;
					static QMargins margin(marginSize, marginSize, marginSize, marginSize);
					labelBackground += margin;

					QPoint labelTranslate = vehicle->position(frameIdx);
					labelBackground.translate(labelTranslate);

					m_painter.fillRect(labelBackground, QColor("#CD2222"));

					m_painter.setPen(m_pen_label);
					m_painter.setFont(m_painterFont);
					m_painter.drawText(
						labelTranslate.x(),
						labelTranslate.y(),
						vehicle->className());
				}
			}

			if (showDetections) {
				m_painter.setPen(m_pen_detection);
				for (auto detection : m_tracker_ptr->getDetections(frameIdx)) {
					m_painter.drawRect(detection.x, detection.y, detection.width, detection.height);
				}
			}
		}
		m_painter.end();

		emit frameDisplayed(frameIdx);
		m_tracker_ptr->onFrameDisplayed(frameIdx);
	}

	input->unmap();

	return *input;
}



void VideoFilter::setTracker(TrafficTracker *tracker)
{
	m_tracker_ptr = tracker;
}

QVideoFilterRunnable *VideoFilter::createFilterRunnable()
{
	VideoFilterRunnable *filter = new VideoFilterRunnable(m_tracker_ptr);

	//connect(filter, &VideoFilterRunnable::frameDisplayed,
	//	this, &VideoFilter::frameDisplayed);

	return filter;
}
