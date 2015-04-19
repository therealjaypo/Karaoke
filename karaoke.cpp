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
}

Karaoke::~Karaoke()
{
    delete ui;
}

void Karaoke::testPushed() {
    QString file = ui->lineEdit->text();
    _player->setMedia(QUrl::fromLocalFile(file));

    this->ui->statusBar->showMessage("Media loaded.");
    this->ui->btnPlay->setEnabled(true);
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
