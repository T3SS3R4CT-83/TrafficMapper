#include "MediaPlayer.hpp"

#include <bitset>
#include <string>

#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>

#include <QThread>
#include <QAbstractVideoSurface>
#include <QtConcurrent/QtConcurrent>

#include <TrafficMapper/Types/Vehicle>
#include <TrafficMapper/Types/Detection>  // TODO: Remove later
#include <TrafficMapper/Modules/VehicleModel>



VideoMeta MediaPlayer::m_videoMeta = {};
cv::VideoCapture MediaPlayer::m_video = cv::VideoCapture();



MediaPlayer::MediaPlayer(QObject * parent, Flags flags)
	: QMediaPlayer(parent, flags), m_positionLabel("0:00 / 0:00")
{
	connect(this, &QMediaPlayer::mediaStatusChanged,
		this, &MediaPlayer::onMediaStatusChanged);
	connect(this, &QMediaPlayer::positionChanged,
		this, &MediaPlayer::onPositionChanged);
}



void MediaPlayer::setVehicleModel(VehicleModel * vehicleModel_ptr)
{
	m_vehicleModel_ptr = vehicleModel_ptr;
}

void MediaPlayer::setGateModel(GateModel * gateModel_ptr)
{
	m_gateModel_ptr = gateModel_ptr;
}



void MediaPlayer::getNextFrame(cv::Mat & frame)
{
	m_bufferMutex.lock();
	if (m_frameBuffer.size() == 0)
		m_bufferNotEmpty.wait(&m_bufferMutex);
	frame = m_frameBuffer.dequeue();
	m_bufferNotFull.wakeAll();
	m_bufferMutex.unlock();
}

void MediaPlayer::getRandomFrame(cv::Mat & frame)
{
	const int frameIdx = rand() % static_cast<int>(m_video.get(cv::CAP_PROP_FRAME_COUNT));
	m_video.set(cv::CAP_PROP_POS_FRAMES, frameIdx);

	m_video >> frame;
}



void MediaPlayer::exportVideo(QUrl fileUrl)
{
	startWorker();

	QtConcurrent::run([this, fileUrl]()
	{
		qDebug() << "Video exporter worker started!";

		m_exporterRunning = true;

		cv::VideoWriter writer(
			fileUrl.toLocalFile().toStdString(),
			cv::VideoWriter::fourcc('M', 'P', '4', '2'),
			m_videoMeta.FPS,
			cv::Size(m_videoMeta.WIDTH, m_videoMeta.HEIGHT));
		//std::vector<Vehicle *> * vehicles_ptr = &(m_vehicleModel_ptr->m_vehicleMap[frameIdx]);
		cv::Mat frame;

		auto playbackSettings = std::move(std::bitset<4>{}.set());

		for (int frameIdx(0); m_exporterRunning && frameIdx < m_videoMeta.FRAMECOUNT; ++frameIdx)
		{
			emit progressUpdated(frameIdx, m_videoMeta.FRAMECOUNT);

			auto vehicles_ptr = &(m_vehicleModel_ptr->m_vehicleMap[frameIdx]);

			getNextFrame(frame);

			QImage _frame = QImage((uchar *)frame.data, frame.cols, frame.rows, frame.step, QImage::Format_BGR888);
			QPainter painter;

			painter.begin(&_frame);
			for (auto vehicle_ptr : *vehicles_ptr)
			{
				QPen pen;
				QFont painterFont("Arial", MediaPlayer::m_videoMeta.HEIGHT / 46, 700);
				QFontMetrics fm(painterFont);
				int marginSize = MediaPlayer::m_videoMeta.HEIGHT * 0.005f;
				QMargins margin(marginSize, marginSize, marginSize, marginSize);

				if (true)  // TRAJECTORY
				{
					pen.setColor(QColor("#EEEEEE"));
					pen.setWidth(3);
					painter.setPen(pen);

					for (const auto & line : vehicle_ptr->m_trajectory)
						painter.drawLine(line.second);
				}

				if (true)  // POSITION
				{
					pen.setColor(QColor("#CD5555"));
					pen.setWidth(5);
					painter.setPen(pen);

					painter.drawPoint(vehicle_ptr->m_positions[frameIdx]);
				}

				if (true)  // LABELS
				{
					pen.setColor("#FFFFFF");
					painter.setPen(pen);
					painter.setFont(painterFont);

					//QRect labelBackground = fm.tightBoundingRect(vehicle_ptr->className());
					QRectF labelBackground(0, 0, 100, 20);
					labelBackground += margin;
					labelBackground.translate(vehicle_ptr->m_positions[frameIdx]);

					painter.fillRect(labelBackground, QColor("#CD2222"));
					painter.drawText(
						vehicle_ptr->m_positions[frameIdx].x() + 5,
						vehicle_ptr->m_positions[frameIdx].y() + 20,
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
						pen.setColor(QColor("blue"));
						break;
					case VehicleType::BUS:
						pen.setColor(QColor("yellow"));
						break;
					case VehicleType::TRUCK:
						pen.setColor(QColor("red"));
						break;
					case VehicleType::MOTORCYCLE:
						pen.setColor(QColor("purple"));
						break;
					case VehicleType::BICYCLE:
						pen.setColor(QColor("green"));
						break;
					default:
						pen.setColor(QColor("white"));
						break;
					}
					pen.setWidth(3);
					painter.setPen(pen);

					painter.drawRect(
						vehicle_ptr->m_detections[frameIdx].x() * MediaPlayer::m_videoMeta.WIDTH,
						vehicle_ptr->m_detections[frameIdx].y() * MediaPlayer::m_videoMeta.HEIGHT,
						vehicle_ptr->m_detections[frameIdx].width() * MediaPlayer::m_videoMeta.WIDTH,
						vehicle_ptr->m_detections[frameIdx].height() * MediaPlayer::m_videoMeta.HEIGHT
					);
				}

				//vehicle_ptr->drawOnFrame(painter, frameIdx, playbackSettings);
			}
			painter.end();

			std::stringstream fileName;
			fileName << "e:/temp/" << std::setw(5) << std::setfill('0') << frameIdx << ".png";
			//_frame.save(fileName.str().c_str());
			cv::imwrite(fileName.str(), frame);

			//writer << frame;
		}

		writer.release();

		emit videoExportFinished();

		qDebug() << "Video exporter finished!";
	});
}

void MediaPlayer::stop()
{
	m_exporterRunning = false;
}



QAbstractVideoSurface * MediaPlayer::getVideoSurface()
{
	return m_surface;
}

void MediaPlayer::onAnalysisStarted()
{
	startWorker();
}

void MediaPlayer::onAnalysisEnded()
{
	qDebug() << "MediaPlayer worker received stop signal!";

	m_frameProviderRunning = false;
	m_bufferNotFull.wakeAll();
}

void MediaPlayer::setVideoSurface(QAbstractVideoSurface * surface)
{
	m_surface = surface;
	setVideoOutput(m_surface);
}

QString MediaPlayer::getPositionLabel() const
{
	return m_positionLabel;
}

VideoMeta MediaPlayer::getVideoMeta() const
{
	return m_videoMeta;
}

inline void MediaPlayer::startWorker()
{
	m_video.set(cv::CAP_PROP_POS_FRAMES, 0);
	m_frameBuffer.clear();
	m_frameProviderRunning = true;

	QtConcurrent::run([this]()
	{
		qDebug() << "MediaPlayer worker started!";

		cv::Mat frame;

		for (; m_frameProviderRunning;)
		{
			m_video >> frame;

			if (frame.empty()) break;

			m_bufferMutex.lock();
			m_frameBuffer.enqueue(frame);
			m_bufferNotEmpty.wakeAll();
			if (m_frameBuffer.size() == 5)
				m_bufferNotFull.wait(&m_bufferMutex);
			m_bufferMutex.unlock();
		}

		qDebug() << "MediaPlayer worker finished!";
	});
}

void MediaPlayer::onPositionChanged(qint64 position)
{
	const uint currentMin = position / 60000;
	const uint currentSec = (position - (currentMin * 60000)) / 1000;

	m_positionLabel = QString("%1:%2 / %3:%4").arg(currentMin).arg(currentSec, 2, 10, QChar('0'))
		.arg(m_videoMeta.LENGTH_MIN).arg(m_videoMeta.LENGTH_SEC, 2, 10, QChar('0'));

	emit positionLabelChanged();
}

void MediaPlayer::loadVideo(QUrl videoUrl)
{
	setMedia(videoUrl);
}

void MediaPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
	if (status == QMediaPlayer::LoadedMedia)
	{
		play();
		pause();

		std::string fileUrl = media().resources().first().url().toLocalFile().toStdString();

		m_video = cv::VideoCapture(fileUrl);

		m_videoMeta.FPS = m_video.get(cv::CAP_PROP_FPS);
		m_videoMeta.FRAMECOUNT = m_video.get(cv::CAP_PROP_FRAME_COUNT);
		m_videoMeta.WIDTH = m_video.get(cv::CAP_PROP_FRAME_WIDTH);
		m_videoMeta.HEIGHT = m_video.get(cv::CAP_PROP_FRAME_HEIGHT);
		m_videoMeta.LENGTH = duration();
		m_videoMeta.LENGTH_MIN = m_videoMeta.LENGTH / 60000;
		m_videoMeta.LENGTH_SEC = (m_videoMeta.LENGTH - (m_videoMeta.LENGTH_MIN * 60000)) / 1000;

		emit videoMetaChanged();
	}
	else if (status == QMediaPlayer::InvalidMedia)
	{
		m_videoMeta.clear();

		emit videoMetaChanged();
	}
}
