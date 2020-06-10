#pragma once

#include <opencv2/core/mat.hpp>

#include <QQuickPaintedItem>
#include <QPoint>
#include <QImage>

class QPainter;
//class QImage;

class CameraCalibration : public QQuickPaintedItem
{
	Q_OBJECT

	Q_PROPERTY(QPoint point_0 MEMBER m_point_0)
	Q_PROPERTY(QPoint point_1 MEMBER m_point_1)
	Q_PROPERTY(QPoint point_2 MEMBER m_point_2)
	Q_PROPERTY(QPoint point_3 MEMBER m_point_3)
	Q_PROPERTY(QPoint point_hover MEMBER m_point_hover)
	Q_PROPERTY(int pointSet MEMBER m_pointSet NOTIFY pointSetChanged)
	Q_PROPERTY(float distance_0 MEMBER m_distance_0)
	Q_PROPERTY(float distance_1 MEMBER m_distance_1)

	QPoint m_point_0, m_point_1, m_point_2, m_point_3, m_point_hover;
	int m_pointSet;
	float m_distance_0, m_distance_1;
	cv::Mat m_homography_i2p, m_homography_p2i;
	QImage m_image;

	friend class TrafficTracker;

public:
	CameraCalibration(QQuickItem * parent = nullptr);

	Q_INVOKABLE void calculateHomography();
	Q_INVOKABLE void loadRandomFrame();

	//cv::Mat getHomographyMatrix() const;

	// Inherited via QQuickPaintedItem
	virtual void paint(QPainter * painter) override;

signals:
	void pointSetChanged();
};
