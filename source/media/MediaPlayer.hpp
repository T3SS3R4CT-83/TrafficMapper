#pragma once

#include <opencv2/videoio.hpp>

#include <QMediaPlayer>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

#include <TrafficMapper/Types/Types>

class QAbstractVideoSurface;
class Tracker;
class VehicleModel;
class GateModel;

class MediaPlayer : public QMediaPlayer
{
	Q_OBJECT

	Q_PROPERTY(QAbstractVideoSurface * videoSurface READ getVideoSurface WRITE setVideoSurface)
	Q_PROPERTY(QString positionLabel READ getPositionLabel NOTIFY positionLabelChanged)
	Q_PROPERTY(VideoMeta videoMeta READ getVideoMeta NOTIFY videoMetaChanged)
	
	QAbstractVideoSurface * m_surface;

	VehicleModel * m_vehicleModel_ptr;
	GateModel * m_gateModel_ptr;

	QString m_positionLabel;

	static cv::VideoCapture m_video;

	QQueue<cv::Mat> m_frameBuffer;

	bool m_frameProviderRunning, m_exporterRunning;
	QMutex m_bufferMutex;
	QWaitCondition m_bufferNotEmpty;
	QWaitCondition m_bufferNotFull;

	friend class Tracker;

public:

	static VideoMeta m_videoMeta;

	MediaPlayer(QObject * parent = nullptr, Flags flags = 0);

	void setVehicleModel(VehicleModel * vehicleModel_ptr);
	void setGateModel(GateModel * gateModel_ptr);

	void getNextFrame(cv::Mat & frame);
	static void getRandomFrame(cv::Mat & frame);

	Q_INVOKABLE void exportVideo(QUrl fileUrl);
	Q_INVOKABLE void stop();

public slots:

	void loadVideo(QUrl videoUrl);

	void onAnalysisStarted();
	void onAnalysisEnded();

private:

	void setVideoSurface(QAbstractVideoSurface * surface);
	QAbstractVideoSurface * getVideoSurface();

	QString getPositionLabel() const;

	VideoMeta getVideoMeta() const;

	inline void startWorker();

private slots:

	void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
	void onPositionChanged(qint64 position);

signals:

	void videoMetaChanged();
	void positionLabelChanged();
	void videoExportFinished();

	void progressUpdated(const int &, const int &);
};
