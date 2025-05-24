#ifndef VIDEORECORDER_H
#define VIDEORECORDER_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <atomic>
#include <opencv2/opencv.hpp>

class VideoRecorder : public QObject
{
    Q_OBJECT

public:
    explicit VideoRecorder(QObject* parent = nullptr);
    ~VideoRecorder();

    bool start(const QString& filename, QSize size, double fps);
    void stop();
    bool isRecording() const;

public slots:
    void writeFrame(const QImage& frame);

private:
    QImage toRgbImage(const QImage& src);
    cv::VideoWriter writer_;
    std::atomic<bool> recording_;
    QMutex mutex_;
    QSize frameSize_;
    double fps_;
};

#endif // VIDEORECORDER_H
