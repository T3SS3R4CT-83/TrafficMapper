#pragma once

#include <QVideoFilterRunnable>
#include <QPainter>

class VehicleModel;

class VideoFilter : public QObject, public QVideoFilterRunnable
{
	Q_OBJECT

	VehicleModel * m_vehicleModel_ptr;

	QPainter m_painter;
	QPen m_pen_trajectory, m_pen_position, m_pen_detection, m_pen_label;
	QFont m_painterFont;

public:

	VideoFilter(VehicleModel * vehicleModel_ptr);

	// Inherited via QVideoFilterRunnable
	virtual QVideoFrame run(QVideoFrame * input, const QVideoSurfaceFormat & surfaceFormat, RunFlags flags) override;

signals:

	void frameDisplayed(int frameIdx);
};

