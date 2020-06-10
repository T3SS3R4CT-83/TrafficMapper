#include "FrameProvider.hpp"


#include <TrafficMapper/Globals>


FrameProviderThread::FrameProviderThread(FrameProvider * parent)
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



FrameProvider::FrameProvider()
{
    m_workerThread_ptr = new FrameProviderThread(this);

    m_workerThread_ptr->start();
}

FrameProvider::~FrameProvider()
{
    m_workerThread_ptr->terminate();
    m_workerThread_ptr->wait();
}

void FrameProvider::getNextFrame(cv::Mat & frame)
{
    // If the buffer is empty, wait for the worker thread to fill it.
    m_frameBufferMutex.lock();
    if (m_frameBuffer.size() == 0)
        m_frameBufferNotEmpty.wait(&m_frameBufferMutex);
    m_frameBufferMutex.unlock();

    // Provide the cached frame.
    frame = m_frameBuffer.dequeue();

    // Wakeing the worker thread, if it's waiting for free space in the buffer.
    m_frameBufferMutex.lock();
    m_frameBufferNotFull.wakeAll();
    m_frameBufferMutex.unlock();
}
