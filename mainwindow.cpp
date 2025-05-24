#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    player_(new VideoPlayer(this)),
    position_timer_(new QTimer(this)),
    is_playing_(false),
    recorder_(new VideoRecorder(this))
{
    ui->setupUi(this);
    connect(player_, &VideoPlayer::frameReady, this, &MainWindow::updateFrame);
    connect(player_, &VideoPlayer::playbackFinished, this, &MainWindow::onPlaybackFinished);
    connect(position_timer_, &QTimer::timeout, this, [this]() {
        if (is_playing_) {
            double pos = player_->currentPosition();
            ui->sld_pos->setValue(static_cast<int>(pos * 1000));
        }
    });
    position_timer_->start(200);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_choose_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть видео"), "", tr("Video Files (*.mp4 *.avi *.mkv)"));
    if (!fileName.isEmpty()) {
        if (player_->openFile(fileName)) {
            ui->btn_pause->setText("⏸");
            ui->btn_pause->setEnabled(true);
            is_playing_ = true;
            player_->play();
        }
    }
}

void MainWindow::on_btn_pause_clicked()
{
    if (is_playing_) {
        player_->pause();
        ui->btn_pause->setText("▶");
        is_playing_ = false;
    } else {
        player_->play();
        ui->btn_pause->setText("⏸");
        is_playing_ = true;
    }
}

void MainWindow::on_btn_stop_clicked()
{
    // player_->stop();
    // ui->video_label->clear();
    // ui->btn_pause->setText("▶");
    // ui->sld_pos->setValue(0);
    // is_playing_ = false;

    if (recorder_->isRecording()) {
        recorder_->stop();
        disconnect(player_, &VideoPlayer::frameReady, recorder_, &VideoRecorder::writeFrame);
        ui->statusBar->showMessage("Запись остановлена.");
    }
}

void MainWindow::on_sld_pos_sliderMoved(int position)
{
    double percent = position / 1000.0;
    player_->seek(percent);
}

void MainWindow::on_btn_record_clicked()
{
    if (!recorder_->isRecording()) {
        QString filename = QFileDialog::getSaveFileName(this, "Сохранить видео", "output.mp4");
        if (!filename.isEmpty()) {
            QSize size = currentFrame_.size();
            double fps = player_->getFps();
            if (fps < 1.0 || fps > 240.0) {
                fps = 30;
            }
            if (recorder_->start(filename, size, fps)) {
                connect(player_, &VideoPlayer::frameReady, recorder_, &VideoRecorder::writeFrame);
                ui->statusBar->showMessage("Запись началась...");
            }
            ui->btn_stop->setEnabled(true);
        }
    }
}

void MainWindow::updateFrame(const QImage& frame)
{
    currentFrame_ = frame;
    if (!frame.isNull()) {
        ui->video_label->clear();
        ui->video_label->setMinimumSize(1, 1);
        QPixmap pixmap = QPixmap::fromImage(frame).scaled(ui->video_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->video_label->setPixmap(pixmap);
    }
}

void MainWindow::onPlaybackFinished()
{
    is_playing_ = false;
    ui->btn_pause->setText("▶");
}
