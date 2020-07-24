#include "FrameProvider.hpp"

#include <opencv2/videoio.hpp>

#include <QThread>
#include <QAbstractVideoSurface>
#include <QtConcurrent/QtConcurrent>



VideoMeta FrameProvider::m_videoMeta = {};
cv::VideoCapture FrameProvider::m_video = cv::VideoCapture();



FrameProvider::FrameProvider(QObject * parent, Flags flags)
	: QMediaPlayer(parent, flags), m_positionLabel("0:00 / 0:00")
{
	//m_videoMeta.LENGTH_MIN = 0;
	//m_videoMeta.LENGTH_SEC = 0;

	connect(this, &QMediaPlayer::mediaStatusChanged,
		this, &FrameProvider::onMediaStatusChanged);
	connect(this, &QMediaPlayer::positionChanged,
		this, &FrameProvider::onPositionChanged);
}



void FrameProvider::getNextFrame(cv::Mat & frame)
{
	m_bufferMutex.lock();
	if (m_frameBuffer.size() == 0)
		m_bufferNotEmpty.wait(&m_bufferMutex);
	frame = m_frameBuffer.dequeue();
	m_bufferNotFull.wakeAll();
	m_bufferMutex.unlock();
}

void FrameProvider::getRandomFrame(cv::Mat & frame)
{
	const int frameIdx = rand() % static_cast<int>(m_video.get(cv::CAP_PROP_FRAME_COUNT));
	m_video.set(cv::CAP_PROP_POS_FRAMES, frameIdx);

	m_video >> frame;
}

QAbstractVideoSurface * FrameProvider::getVideoSurface()
{
	return m_surface;
}

void FrameProvider::onAnalysisStarted()
{
	m_video.set(cv::CAP_PROP_POS_FRAMES, 0);
	m_frameBuffer.clear();
	m_isRunning = true;

	QtConcurrent::run([this]()
	{
		qDebug() << "FrameProvider worker started!";

		cv::Mat frame;

		for (; m_isRunning;)
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

		qDebug() << "FrameProvider worker finished!";
	});
}

void FrameProvider::onAnalysisEnded()
{
	qDebug() << "FrameProvider worker received stop signal!";

	m_isRunning = false;
	m_bufferNotFull.wakeAll();
}

void FrameProvider::setVideoSurface(QAbstractVideoSurface * surface)
{
	m_surface = surface;
	setVideoOutput(m_surface);
}

QString FrameProvider::getPositionLabel() const
{
	return m_positionLabel;
}

//void FrameProvider::setVideoMeta(VideoMeta metaData)
//{
//	m_videoMeta = metaData;
//}

VideoMeta FrameProvider::getVideoMeta() const
{
	return m_videoMeta;
}

void FrameProvider::onPositionChanged(qint64 position)
{
	const uint currentMin = position / 60000;
	const uint currentSec = (position - (currentMin * 60000)) / 1000;

	m_positionLabel = QString("%1:%2 / %3:%4").arg(currentMin).arg(currentSec, 2, 10, QChar('0'))
		.arg(m_videoMeta.LENGTH_MIN).arg(m_videoMeta.LENGTH_SEC, 2, 10, QChar('0'));

	emit positionLabelChanged();
}

void FrameProvider::loadVideo(QUrl videoUrl)
{
	setMedia(videoUrl);
}

void FrameProvider::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
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
