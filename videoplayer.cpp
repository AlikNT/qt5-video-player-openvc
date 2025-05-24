#include "videoplayer.h"
#include <QDebug>
#include <QThread>

VideoPlayer::VideoPlayer(QObject* parent)
    : QObject(parent), playing_(false), stopRequested_(false)
{}

VideoPlayer::~VideoPlayer()
{
    stop();
}

bool VideoPlayer::openFile(const QString& filename)
{
    stop();

    cap_.open(filename.toStdString());
    if (!cap_.isOpened()) {
        qWarning() << "Failed to open video:" << filename;
        return false;
    }

    totalFrames_ = cap_.get(cv::CAP_PROP_FRAME_COUNT);
    fps_ = cap_.get(cv::CAP_PROP_FPS);

    qDebug() << "Opened:" << filename << "fps =" << fps_ << "total frames =" << totalFrames_;

    return true;
}

void VideoPlayer::play()
{
    if (playing_ || !cap_.isOpened())
        return;

    playing_ = true;
    stopRequested_ = false;

    // поток ещё не запущен или завершён
    if (!worker_.joinable()) {
        worker_ = std::thread([this] { decodeLoop(); });
    }
}

void VideoPlayer::pause()
{
    playing_ = false;
}

void VideoPlayer::stop()
{
    playing_ = false;
    stopRequested_ = true;

    if (worker_.joinable())
        worker_.join();

    cap_.release();
}

void VideoPlayer::seek(double position)
{
    QMutexLocker locker(&mutex_);
    if (cap_.isOpened()) {
        int frame = static_cast<int>(position * totalFrames_);
        cap_.set(cv::CAP_PROP_POS_FRAMES, frame);
    }
}

double VideoPlayer::duration() const
{
    return totalFrames_ / fps_;
}

double VideoPlayer::currentPosition() const
{
    return cap_.get(cv::CAP_PROP_POS_FRAMES) / totalFrames_;
}

double VideoPlayer::getFps() const
{
    return fps_;
}

void VideoPlayer::decodeLoop()
{
    while (!stopRequested_) {
        if (!playing_) {
            QThread::msleep(10);
            continue;
        }

        mutex_.lock();
        cv::Mat frame;
        bool success = cap_.read(frame);
        mutex_.unlock();

        if (!success) {
            emit playbackFinished();
            break;
        }

        QImage image = matToQImage(frame);
        emit frameReady(image);

        QThread::msleep(static_cast<int>(1000 / fps_));
    }
}

QImage VideoPlayer::matToQImage(const cv::Mat& mat)
{
    if (mat.channels() == 3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    } else if (mat.channels() == 1) {
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8).copy();
    }
    return {};
}
