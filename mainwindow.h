#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "videoplayer.h"
#include "videorecorder.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btn_choose_clicked();
    void on_btn_pause_clicked();
    void on_btn_stop_clicked();
    void on_sld_pos_sliderMoved(int position);
    void on_btn_record_clicked();

    void updateFrame(const QImage& frame);
    void onPlaybackFinished();

private:
    Ui::MainWindow *ui;

    VideoPlayer* player_;
    QTimer* position_timer_;
    bool is_playing_;
    VideoRecorder* recorder_;
    QImage currentFrame_;
};
#endif // MAINWINDOW_H
