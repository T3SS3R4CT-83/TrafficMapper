#pragma once


#include <opencv2/videoio.hpp>
#include <opencv2/core/mat.hpp>

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>


class FrameProvider;


class FrameProviderThread : public QThread
{
    FrameProvider * m_provider_ptr;
    cv::VideoCapture m_video;

public:
    FrameProviderThread(FrameProvider * parent = nullptr);

    void run() override;
};



class FrameProvider : public QObject
{
    FrameProviderThread * m_workerThread_ptr;
    QQueue<cv::Mat> m_frameBuffer;

    QMutex m_frameBufferMutex;
    QWaitCondition m_frameBufferNotEmpty;
    QWaitCondition m_frameBufferNotFull;

    friend class FrameProviderThread;

public:
    FrameProvider();
    ~FrameProvider();

    void getNextFrame(cv::Mat & frame);
};
