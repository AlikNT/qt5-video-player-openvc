#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <atomic>
#include <opencv2/opencv.hpp>

class VideoPlayer : public QObject
{
    Q_OBJECT

public:
    explicit VideoPlayer(QObject* parent = nullptr);
    ~VideoPlayer();

    bool openFile(const QString& filename);
    void play();
    void pause();
    void stop();
    void seek(double position); // 0.0 - 1.0

    double duration() const;
    double currentPosition() const;
    double getFps() const;

signals:
    void frameReady(const QImage& frame);
    void playbackFinished();

private:
    void decodeLoop();
    QImage matToQImage(const cv::Mat& frame);

    std::thread worker_;
    std::atomic<bool> playing_;
    std::atomic<bool> stopRequested_;
    QMutex mutex_;

    cv::VideoCapture cap_;
    double totalFrames_ = 0;
    double fps_ = 0;
};

#endif // VIDEOPLAYER_H
