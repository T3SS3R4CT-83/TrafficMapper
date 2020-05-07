#pragma once

#include <QAbstractVideoFilter>
#include <QVideoFilterRunnable>
#include <QQuickPaintedItem>
#include <QMediaPlayer>
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QBrush>

//#include "TrafficTracker.hpp"

class TrafficTracker;
class VideoFilter;
class GlobalMeta;

class VideoFilterRunnable : public QObject, public QVideoFilterRunnable
{
	Q_OBJECT 

	QPen m_pen_detection;
	QPen m_pen_tracking;
	QPen m_pen_position;
	QPen m_pen_trajectory;
	QPen m_pen_label;
	QPainter m_painter;
	QFont m_painterFont;

	TrafficTracker* m_tracker_ptr;
	GlobalMeta* m_globals_ptr;

public:
	VideoFilterRunnable(TrafficTracker *tracker);

	QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags);

signals:
	void frameDisplayed(int frameIdx);
};



class VideoFilter :	public QAbstractVideoFilter
{
	Q_OBJECT

	TrafficTracker *m_tracker_ptr;

public:
	void setTracker(TrafficTracker *tracker);

private:
	// Inherited via QAbstractVideoFilter
	virtual QVideoFilterRunnable *createFilterRunnable() override;

signals:
	void frameDisplayed(int frameIdx);
};
