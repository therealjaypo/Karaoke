#include "karaoke.h"
#include "ui_karaoke.h"

#include <QDebug>
#include <QUrl>
#include <QFileDialog>
#include <QSqlTableModel>

#include "util/musiclibrary.h"

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

    // TODO Settingize this
    dbase = QSqlDatabase::addDatabase("QMYSQL");
    dbase.setHostName("localhost");
    dbase.setDatabaseName("karaoke");
    dbase.setUserName("pharout");
    dbase.setPassword("outph4r");

    QSqlTableModel *sqltbl = new QSqlTableModel;
    sqltbl->setTable("library_path");
    sqltbl->select();

    ui->listPaths->setModel(sqltbl);
    ui->listPaths->setModelColumn(1);

    sqltbl = new QSqlTableModel;
    sqltbl->setTable("song");
    sqltbl->select();

    ui->listLibrary->setModel(sqltbl);
    ui->listLibrary->setColumnHidden(0,true);
    ui->listLibrary->setColumnHidden(1,true);
    ui->listLibrary->setColumnHidden(4,true);
    ui->listLibrary->verticalHeader()->setVisible(false);
    ui->listLibrary->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
            ui->progressBar->setValue(0);
            ui->btnStartReq->setEnabled(false);
            ui->btnLibStart->setEnabled(false);
            break;

        case QMediaPlayer::StoppedState:
            ui->lineEdit->setEnabled(true);
            ui->btnFile->setEnabled(true);
            ui->btnPlay->setChecked(false);
            ui->progressBar->setValue(0);
            ui->progressBar->setEnabled(false);
            if( ui->requestsList->currentIndex().row() != -1 )
                ui->btnStartReq->setEnabled(true);

            ui->btnLibStart->setEnabled(true);
            break;

        default:
            break;
    }
}

void Karaoke::playerPositionChanged(qint64 pos) {
    ui->progressBar->setValue(pos);
}

void Karaoke::addLibraryPath() {
    QString path = QFileDialog::getExistingDirectory(
        this,
        tr("Add library path"),
        QString()
    );
    int newidx;
    QModelIndex idx;
    QAbstractItemModel *model;

    if( !path.isNull() ) {
        model = ui->listPaths->model();
        newidx = model->rowCount();
        model->insertRow(newidx);

        idx = model->index(newidx,1);
        model->setData(idx,path);
        model->submit();
    }
}

void Karaoke::refreshPath() {
    QModelIndex idx = ui->listPaths->currentIndex();
    if( idx.row() != -1 ) {
        MusicLibrary::updateSongsInPath(idx.data().toString());
    }
}

void Karaoke::librarySelect() {
    QModelIndex idx = ui->listLibrary->currentIndex();
    QModelIndex nidx;

    if( idx.row() != -1 ) {
        nidx = ui->listLibrary->model()->index(idx.row(),4);
        qDebug() << "Playing" << nidx.data().toString();

        ui->lineEdit->setText(nidx.data().toString());
        _player->setMedia(QUrl::fromLocalFile(ui->lineEdit->text()));
        this->ui->btnPlay->setEnabled(true);
    }
}
