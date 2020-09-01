#include "CameraCalibration.hpp"

#include <opencv2/core/types.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QPainter>

#include <TrafficMapper/Media/MediaPlayer>

CameraCalibration::CameraCalibration(QQuickItem * parent)
	: QQuickPaintedItem(parent), m_pointSet(0)
{
}

void CameraCalibration::calculateHomography()
{
	//const float ratio_X = 1.f;
	//const float ratio_Y = 1.f;
	const float ratio_X = MediaPlayer::m_videoMeta.WIDTH / (float)property("width").toFloat();
	const float ratio_Y = MediaPlayer::m_videoMeta.HEIGHT / (float)property("height").toFloat();

	std::vector<cv::Point2f> planePoints = {
		cv::Point2f(0, 0),
		cv::Point2f(m_distance_0, 0),
		cv::Point2f(m_distance_0, m_distance_1),
		cv::Point2f(0, m_distance_1)
	};
	std::vector<cv::Point2f> imgPoints = {
		cv::Point2f(m_point_0.x() * ratio_X, m_point_0.y() * ratio_Y),
		cv::Point2f(m_point_1.x() * ratio_X, m_point_1.y() * ratio_Y),
		cv::Point2f(m_point_2.x() * ratio_X, m_point_2.y() * ratio_Y),
		cv::Point2f(m_point_3.x() * ratio_X, m_point_3.y() * ratio_Y)
	};

	m_homography_p2i = cv::findHomography(planePoints, imgPoints);
	m_homography_i2p = cv::findHomography(imgPoints, planePoints);
}

void CameraCalibration::loadRandomFrame()
{
	cv::Mat frame;

	MediaPlayer::getRandomFrame(frame);
	cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

	m_image = QImage((uchar *)frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888).scaled(QSize(960, 540));
}

//cv::Mat CameraCalibration::getHomographyMatrix() const
//{
//	return m_homography_i2p;
//}

void CameraCalibration::paint(QPainter * painter)
{
	painter->setRenderHint(QPainter::Antialiasing);

	QPen pen;
	pen.setWidth(3);
	pen.setColor(QColor("#CD5555"));
	pen.setCapStyle(Qt::RoundCap);

	painter->setPen(pen);

	painter->drawImage(QPoint(0, 0), m_image);

	switch (m_pointSet)
	{
	case 1:
		painter->drawLine(m_point_0, m_point_hover);
		break;

	case 2:
		painter->drawLine(m_point_0, m_point_1);
		pen.setColor(QColor("#336699"));
		painter->setPen(pen);
		painter->drawLine(m_point_0, m_point_hover);
		painter->drawLine(m_point_1, m_point_hover);
		break;

	case 3:
		painter->drawLine(m_point_0, m_point_1);
		painter->drawLine(m_point_2, m_point_hover);
		pen.setColor(QColor("#336699"));
		painter->setPen(pen);
		painter->drawLine(m_point_1, m_point_2);
		painter->drawLine(m_point_0, m_point_hover);
		break;

	case 4:
	{
		painter->drawLine(m_point_0, m_point_1);
		painter->drawLine(m_point_2, m_point_3);
		pen.setColor(QColor("#336699"));
		painter->setPen(pen);
		painter->drawLine(m_point_1, m_point_2);
		painter->drawLine(m_point_0, m_point_3);

		std::vector<cv::Point2f> input;
		std::vector<cv::Point2f> output;

		for (int x(-m_distance_0); x < 2 * m_distance_0; ++x)
		{
			for (int y(-m_distance_1); y < 2 * m_distance_1; ++y)
			{
				input.push_back(cv::Point2f(x, y));
			}
		}

		cv::perspectiveTransform(input, output, m_homography_p2i);

		//const float ratio_X = 1.f;
		//const float ratio_Y = 1.f;
		const float ratio_X = property("width").toInt() / (float)MediaPlayer::m_videoMeta.WIDTH;
		const float ratio_Y = property("height").toInt() / (float)MediaPlayer::m_videoMeta.HEIGHT;

		pen.setColor(QColor("yellow"));
		painter->setPen(pen);
		for (auto point : output)
			painter->drawPoint(QPoint(point.x * ratio_X, point.y * ratio_Y));

		break;
	}

	default:
		break;
	}
}
