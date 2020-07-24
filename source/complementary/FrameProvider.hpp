#pragma once

#include <opencv2/videoio.hpp>

#include <QMediaPlayer>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

#include <TrafficMapper/Types/Types>
//#include <TrafficMapper/Modules/Tracker>

class QAbstractVideoSurface;
class Tracker;
class FrameProviderWorker;

class FrameProvider : public QMediaPlayer
{
	Q_OBJECT

	Q_PROPERTY(QAbstractVideoSurface * videoSurface READ getVideoSurface WRITE setVideoSurface)
	Q_PROPERTY(QString positionLabel READ getPositionLabel NOTIFY positionLabelChanged)
	Q_PROPERTY(VideoMeta videoMeta READ getVideoMeta NOTIFY videoMetaChanged)
	
	QAbstractVideoSurface * m_surface;

	QString m_positionLabel;

	static cv::VideoCapture m_video;

	FrameProviderWorker * m_worker_ptr;
	QQueue<cv::Mat> m_frameBuffer;

	bool m_isRunning;
	QMutex m_bufferMutex;
	QWaitCondition m_bufferNotEmpty;
	QWaitCondition m_bufferNotFull;

	friend class Tracker;
	friend class FrameProviderWorker;

public:

	static VideoMeta m_videoMeta;

	FrameProvider(QObject * parent = nullptr, Flags flags = 0);

	void getNextFrame(cv::Mat & frame);
	static void getRandomFrame(cv::Mat & frame);

public slots:

	void loadVideo(QUrl videoUrl);

	void onAnalysisStarted();
	void onAnalysisEnded();

private:

	void setVideoSurface(QAbstractVideoSurface * surface);
	QAbstractVideoSurface * getVideoSurface();

//	void setPositionLabel(QString label);
	QString getPositionLabel() const;

	//void setVideoMeta(VideoMeta metaData);
	VideoMeta getVideoMeta() const;

private slots:

	void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
	void onPositionChanged(qint64 position);

signals:

	void videoMetaChanged();
	void positionLabelChanged();
};
