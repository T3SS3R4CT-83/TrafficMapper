#include "FrameProvider.hpp"

#include <QDebug>

#include <TrafficMapper/Globals>

FrameProviderThread::FrameProviderThread(FrameProvider *parent)
    : QThread(parent), m_provider_ptr(parent)
{
    m_video = cv::VideoCapture(GlobalMeta::getInstance()->VIDEO_URL().toStdString());
}

void FrameProviderThread::run()
{
	cv::Mat frame;

	while (true)
	{
		m_provider_ptr->m_frameBufferMutex.lock();
        if (m_provider_ptr->m_frameBuffer.size() == Settings::FRAME_BUFFER_SIZE)
			m_provider_ptr->m_frameBufferNotFull.wait(&m_provider_ptr->m_frameBufferMutex);
		m_provider_ptr->m_frameBufferMutex.unlock();

		m_video >> frame;
		if (frame.empty()) break;

        m_provider_ptr->m_frameBuffer.enqueue(frame);

		m_provider_ptr->m_frameBufferMutex.lock();
		m_provider_ptr->m_frameBufferNotEmpty.wakeAll();
		m_provider_ptr->m_frameBufferMutex.unlock();
	}
}

void FrameProviderThread::setFrameIdx(int _frameIdx)
{
    m_video.set(cv::CAP_PROP_POS_FRAMES, _frameIdx);
}



FrameProvider::FrameProvider(const int _firstFrameIdx)
{
    m_nextFrameIdx = 0;

    m_workerThread_ptr = new FrameProviderThread(this);
    if (_firstFrameIdx > 0)
        m_workerThread_ptr->setFrameIdx(_firstFrameIdx);
    m_workerThread_ptr->start();
}

FrameProvider::~FrameProvider()
{
    m_workerThread_ptr->terminate();
    m_workerThread_ptr->wait();
}

void FrameProvider::getNextFrame(cv::Mat &_frame, int _frameIdx)
{
    // Many cases we request the same frame in a row.
    // Let's take a shortcut in these cases.
    if (_frameIdx == m_nextFrameIdx - 1)
    {
        _frame = m_prevFrame;
        return;
    }

    // If the frame index is set and different from the buffered one,
    // then clear the buffer and load the new frame.
    if (_frameIdx >= 0 && m_nextFrameIdx != _frameIdx)
    {
        QMutexLocker locker(&m_frameBufferMutex);

        m_workerThread_ptr->setFrameIdx(_frameIdx);
        m_frameBuffer.clear();
        m_nextFrameIdx = _frameIdx;

        m_frameBufferNotFull.wakeAll();
    }

    // If the buffer is empty, wait for the worker thread to fill it.
    m_frameBufferMutex.lock();
    if (m_frameBuffer.size() == 0)
        m_frameBufferNotEmpty.wait(&m_frameBufferMutex);
    m_frameBufferMutex.unlock();

    // Provide the cached frame and save it for later use. (To speed up the consecutive
    // requests of the same frame.)
    _frame = m_frameBuffer.dequeue();
    m_prevFrame = _frame;
    ++m_nextFrameIdx;

    // Wakeing the worker thread, if it's waiting for free space in the buffer.
    m_frameBufferMutex.lock();
    m_frameBufferNotFull.wakeAll();
    m_frameBufferMutex.unlock();
}
