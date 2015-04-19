#include "karaoke.h"
#include "ui_karaoke.h"

#include <QDebug>
#include <QUrl>
#include <QFileDialog>

Karaoke::Karaoke(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Karaoke)
{
    ui->setupUi(this);

    _player = new QMediaPlayer;
    _player->setVolume(75);
    ui->cdgDisplay->setPlayer(_player);
    connect(_player,SIGNAL(stateChanged(QMediaPlayer::State)),this,SLOT(playerStateChanged(QMediaPlayer::State)));
    connect(_player,SIGNAL(positionChanged(qint64)),this,SLOT(playerPositionChanged(qint64)));
}

Karaoke::~Karaoke()
{
    delete ui;
}

void Karaoke::playPushed(bool state) {
    if( state )
        _player->play();
    else
        _player->stop();
}

void Karaoke::selectFile() {
    QString fil = QFileDialog::getOpenFileName(this,tr("Open File"),QString(),tr("MP3 Files (*.mp3)"));
    if( !fil.isNull() ) {
        this->ui->lineEdit->setText(fil);
        _player->setMedia(QUrl::fromLocalFile(fil));
        this->ui->btnPlay->setEnabled(true);
    }
}

void Karaoke::playerStateChanged(QMediaPlayer::State state) {
    switch( state ) {
        case QMediaPlayer::PlayingState:
            ui->lineEdit->setEnabled(false);
            ui->btnFile->setEnabled(false);
            ui->btnPlay->setChecked(true);
            ui->progressBar->setMaximum(_player->duration());
            ui->progressBar->setEnabled(true);
            break;

        case QMediaPlayer::StoppedState:
            ui->lineEdit->setEnabled(true);
            ui->btnFile->setEnabled(true);
            ui->btnPlay->setChecked(false);
            ui->progressBar->setValue(0);
            ui->progressBar->setEnabled(false);
            break;

        default:
            break;
    }
}

void Karaoke::playerPositionChanged(qint64 pos) {
    ui->progressBar->setValue(pos);
}
