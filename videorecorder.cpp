#include "videorecorder.h"
#include <QDebug>

VideoRecorder::VideoRecorder(QObject* parent)
    : QObject(parent), recording_(false), fps_(30.0)
{}

VideoRecorder::~VideoRecorder()
{
    stop();
}

bool VideoRecorder::start(const QString& filename, QSize size, double fps)
{
    stop();

    frameSize_ = size;
    fps_ = fps;

    writer_.open(filename.toStdString(),
                 cv::VideoWriter::fourcc('a', 'v', 'c', '1'), // H264
                 fps,
                 cv::Size(size.width(), size.height()));

    if (!writer_.isOpened()) {
        qWarning() << "Failed to open file for recording:" << filename;
        return false;
    }

    recording_ = true;
    return true;
}

void VideoRecorder::stop()
{
    QMutexLocker locker(&mutex_);
    if (recording_) {
        writer_.release();
        recording_ = false;
    }
}

bool VideoRecorder::isRecording() const
{
    return recording_;
}

void VideoRecorder::writeFrame(const QImage& frame)
{
    QMutexLocker locker(&mutex_);
    if (!recording_ || frame.isNull())
        return;

    QImage rgb = toRgbImage(frame);
    cv::Mat mat(rgb.height(), rgb.width(), CV_8UC3,
                const_cast<uchar*>(rgb.bits()), rgb.bytesPerLine());

    cv::Mat bgr;
    cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR);
    writer_.write(bgr);
}

QImage VideoRecorder::toRgbImage(const QImage& src)
{
    if (src.format() != QImage::Format_RGB888) {
        return src.convertToFormat(QImage::Format_RGB888);
    }
    return src;
}
