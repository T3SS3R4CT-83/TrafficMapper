#include "VideoFilter.hpp"

#include <TrafficMapper/Complementary/FrameProvider>
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

		const int frameIdx = std::round(input->startTime() * 0.000001 * FrameProvider::m_videoMeta.FPS);

		//const bool showDetections = m_globals_ptr->PLAYER_SHOW_DETECTIONS();
		//const bool showPaths = m_globals_ptr->PLAYER_SHOW_PATHS();
		//const bool showLabels = m_globals_ptr->PLAYER_SHOW_LABELS();
		//const bool showPositions = m_globals_ptr->PLAYER_SHOW_POSITIONS();

		std::vector<Vehicle *> * vehicles_ptr = &(m_vehicleModel_ptr->m_vehicleMap[frameIdx]);

		m_painter.begin(&frame);
		{
			for (auto vehicle_ptr : *vehicles_ptr)
			{
				if (true)  // TRAJECTORY
				{
					m_painter.setPen(m_pen_trajectory);
					for (const auto & line : vehicle_ptr->m_trajectory)
						m_painter.drawLine(line.second);
				}

				if (true)  // POSITION
				{
					m_painter.setPen(m_pen_position);
					m_painter.drawPoint(vehicle_ptr->m_positions[frameIdx]);
				}

				if (true)  // LABELS
				{
					static QFontMetrics fm(m_painterFont);
					//QRect labelBackground = fm.tightBoundingRect(vehicle_ptr->className());
					QRectF labelBackground(0, 0, 100, 20);

					static int marginSize = FrameProvider::m_videoMeta.HEIGHT * 0.005f;
					static QMargins margin(marginSize, marginSize, marginSize, marginSize);
					labelBackground += margin;

					QPointF labelTranslate = vehicle_ptr->m_positions[frameIdx];
					labelBackground.translate(labelTranslate);

					m_painter.fillRect(labelBackground, QColor("#CD2222"));

					m_painterFont = QFont("Arial", FrameProvider::m_videoMeta.HEIGHT / 46, 700);

					m_painter.setPen(m_pen_label);
					m_painter.setFont(m_painterFont);
					m_painter.drawText(
						labelTranslate.x(),
						labelTranslate.y() + 20,
						QString::number(vehicle_ptr->m_speed[frameIdx]));
					//m_painter.drawText(
					//	labelTranslate.x(),
					//	labelTranslate.y(),
					//	vehicle_ptr->className());
				}

				if (true)  // DETECTIONS
				{
					switch (vehicle_ptr->m_detections[frameIdx].vehicleType())
					{
					case VehicleType::CAR:
						m_pen_detection.setColor(QColor("blue"));
						break;
					case VehicleType::BUS:
						m_pen_detection.setColor(QColor("yellow"));
						break;
					case VehicleType::TRUCK:
						m_pen_detection.setColor(QColor("red"));
						break;
					case VehicleType::MOTORCYCLE:
						m_pen_detection.setColor(QColor("purple"));
						break;
					case VehicleType::BICYCLE:
						m_pen_detection.setColor(QColor("green"));
						break;
					default:
						m_pen_detection.setColor(QColor("white"));
						break;
					}
					m_painter.setPen(m_pen_detection);
					m_painter.drawRect(
						vehicle_ptr->m_detections[frameIdx].x() * FrameProvider::m_videoMeta.WIDTH,
						vehicle_ptr->m_detections[frameIdx].y() * FrameProvider::m_videoMeta.HEIGHT,
						vehicle_ptr->m_detections[frameIdx].width() * FrameProvider::m_videoMeta.WIDTH,
						vehicle_ptr->m_detections[frameIdx].height() * FrameProvider::m_videoMeta.HEIGHT
					);
				}
			}
		}
		m_painter.end();

		emit frameDisplayed(frameIdx);
	}

	input->unmap();

	return *input;
}
