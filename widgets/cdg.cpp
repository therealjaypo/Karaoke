#include "widgets/cdg.h"
#include "ui_cdg.h"

#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QPainter>

#ifdef Q_OS_UNIX
extern "C" {
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <fcntl.h>
}
#endif

CDG::CDG(QWidget *parent) :
    QWidget(parent),
    _stream(-1),
    _player(NULL),
    _image(NULL),
    _item(NULL),
    _scene(NULL),
    _xparentIdx(-1),
    ui(new Ui::CDG)
{
    ui->setupUi(this);

    _image = new QImage(QSize(300,216),QImage::Format_Indexed8);
    _scene = new QGraphicsScene();
    _item =_scene->addPixmap(QPixmap::fromImage(*_image));
    _scene->setSceneRect(_image->rect());

    ui->graphicsView->setScene(_scene);
    resetImage();
}

CDG::~CDG()
{
    delete ui;
}

void CDG::resizeEvent(QResizeEvent *) {
    if( _scene != NULL )
        ui->graphicsView->fitInView(_scene->itemsBoundingRect(),Qt::KeepAspectRatio);
}

void CDG::setPlayer(QMediaPlayer *qmp) {
    if( _player != NULL ) {
        // TODO Disconnect from previous player
    }

    _player = qmp;
    connect(_player,SIGNAL(positionChanged(qint64)), this, SLOT(playerPositionChanged(qint64)));
    connect(_player,SIGNAL(currentMediaChanged(QMediaContent)), this, SLOT(playerMediaChanged(QMediaContent)));
    connect(_player,SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(playerStateChanged(QMediaPlayer::State)));

    // 75 sectors per second = 1000 / 75  = 13 milliseconds per sector
    _player->setNotifyInterval(13);
}

void CDG::playerStateChanged(QMediaPlayer::State stat) {
    switch( stat ) {
        case QMediaPlayer::StoppedState:
        case QMediaPlayer::PlayingState:
            resetImage();
            break;

        default:
            break;
    }
}

void CDG::playerMediaChanged(QMediaContent media) {
    QString cdg = media.canonicalUrl().toLocalFile();
    int ext = cdg.lastIndexOf('.');
    cdg = cdg.left(ext);
    cdg.append(".cdg");

    qDebug() << "Looking for" << cdg;

#ifdef Q_OS_UNIX
    if( _stream != -1 ) {
        ::close(_stream);
    }

    _stream = ::open(cdg.toStdString().c_str(),O_RDONLY);
    if( _stream < 0 ) {
        qWarning() << "Failed to open" << cdg;
        _stream = -1;
    } else {
        ::lseek(_stream,0,SEEK_SET);
        qDebug() << "Opened" << cdg;
    }
#endif

    resetImage();
}

void CDG::playerPositionChanged(qint64 pos) {
    unsigned int x;

    if( pos == 0 )
        _curSector = 0;
    else
        _curSector++;

    // 96 bytes per sector
    qint64 bytepos = _curSector * 96;

    // 4 24 byte packets per sector
    int curpack = 4;

    // A packet
    cdg_packet packet;

#ifdef Q_OS_UNIX
    // Since we'll use it a lot...
    ::ssize_t sz = sizeof(cdg_packet);
#endif

#ifdef Q_OS_UNIX
    // Make sure we have our stream open
    if( _stream != -1 ) {
        if( ::lseek(_stream,bytepos,SEEK_SET) < 0 ) {
            qWarning() << "CDG failed seek to" << bytepos << "bytes!";
        } else {
#endif
            // Four 24 byte packets per sector
            while( curpack-- ) {
                ::memset(&packet,0,sz);
#ifdef Q_OS_UNIX
                if( ::read(_stream,&packet,sz) != sz ) {
//                    qDebug() << "CDG failed to read" << sz << "bytes";
                }
#endif

                packet.command &= SC_MASK;

                if( packet.command == CDG_COMMAND ) {
                    packet.instruction &= SC_MASK;

                    for(x=0;x<sizeof(packet.data);x++) {
                        packet.data[x] &= SC_MASK;
                    }

                    handlePacket(packet);
                }
            }
        }
    }

}

void CDG::handlePacket(cdg_packet packet) {
    switch( packet.instruction ) {
        case CDG_MEMORY_PRESET:
            cdgMemoryPreset(packet);
            break;

        case CDG_SET_TRANSPARENT:
//            cdgSetTransparent(packet);
            break;

        case CDG_COLOR_LOADLOW:
            cdgLoadLowColours(packet);
            break;

        case CDG_COLOR_LOADHIGH:
            cdgLoadHighColours(packet);
            break;

        case CDG_TILE_NORMAL:
            cdgDrawTile(packet);
            break;

        case CDG_TILE_XOR:
            cdgDrawTileXOR(packet);
            break;

        default:
            qDebug() << "Unhandled CDG instruction" << packet.instruction;
            break;
    }
}

void CDG::resetImage() {
    int x,y;

    resizeEvent(NULL);
    _xparentIdx = -1;
    for( x=0;x<16;x++ )
        _image->setColor(x,qRgb(0,0,0));

    for(y=0;y<_image->height();y++) {
        for(x=0;x<_image->width();x++) {
            _image->setPixel(x,y,0);
        }
    }

    updateImage();
}

QRectF CDG::rectForTile(int x, int y) {
    return QRectF(x*6,y*12,6,12);
}

void CDG::updateImage() {
    _item->setPixmap(QPixmap::fromImage(*_image));
}

void CDG::cdgMemoryPreset(cdg_packet dat) {
    int x,y,col = dat.data[0] & 0x0F,repeat = dat.data[1] & 0x0F;

    if( repeat == 0 ) {
        for(y=0;y<_image->height();y++) {
            for( x=0;x<_image->width();x++ ) {
                _image->setPixel(x,y,col);
            }
        }
    }

    updateImage();
}

void CDG::cdgSetTransparent(cdg_packet dat) {
    int col = dat.data[0] & 0x0F;

    if( col != _xparentIdx ) {
        _xparentIdx = col;
        _image->setColor(col,qRgba(0,0,0,0));
        updateImage();
    }
}

void CDG::cdgLoadLowColours(cdg_packet dat) {
    int col,r,g,b,pos=0;

    for( col=0;col<8;col++ ) {
        // First byte = XXrrrrgg
        // Second byte = XXggbbbb
        if( col != _xparentIdx ) {
            r = 255 * ((float)((dat.data[pos] & 0x3C) >> 2)/15);
            g = 255 * ((float)(((dat.data[pos] & 0x3) << 2) | ((dat.data[pos+1] & 0x30) >> 4))/15);
            b = 255 * ((float)(dat.data[pos+1] & 0xF)/15);
            pos+=2;

            _image->setColor(col,qRgb(r,g,b));
        }
    }

    updateImage();
}

void CDG::cdgLoadHighColours(cdg_packet dat) {
    int col,r,g,b,pos=0;

    for( col=8;col<16;col++ ) {
        // First byte = XXrrrrgg
        // Second byte = XXggbbbb
        if( col != _xparentIdx ) {
            r = 255 * ((float)((dat.data[pos] & 0x3C) >> 2)/15);
            g = 255 * ((float)(((dat.data[pos] & 0x3) << 2) | ((dat.data[pos+1] & 0x30) >> 4))/15);
            b = 255 * ((float)(dat.data[pos+1] & 0xF)/15);
            pos+=2;

            _image->setColor(col,qRgb(r,g,b));
        }
    }

    updateImage();
}

void CDG::cdgDrawTileXOR(cdg_packet dat) {
    int lowCol = dat.data[0] & 0x0F,
            highCol = dat.data[1] & 0x0F,
            row = dat.data[2] & 0x1F,
            column = dat.data[3] & 0x3F,
            x,col;
    QRectF where = rectForTile(column,row);

    for( x=4;x<16;x++ ) {
        col = (dat.data[x] & 0x20)?highCol:lowCol;
        col ^= _image->pixelIndex(where.left(),where.top()+(x-4));
        _image->setPixel(where.left(),where.top()+(x-4), col );

        col = (dat.data[x] & 0x10)?highCol:lowCol;
        col ^= _image->pixelIndex(where.left()+1,where.top()+(x-4));
        _image->setPixel(where.left()+1,where.top()+(x-4), col );

        col = (dat.data[x] & 0x8)?highCol:lowCol;
        col ^= _image->pixelIndex(where.left()+2,where.top()+(x-4));
        _image->setPixel(where.left()+2,where.top()+(x-4), col );

        col = (dat.data[x] & 0x4)?highCol:lowCol;
        col ^= _image->pixelIndex(where.left()+3,where.top()+(x-4));
        _image->setPixel(where.left()+3,where.top()+(x-4), col );

        col = (dat.data[x] & 0x2)?highCol:lowCol;
        col ^= _image->pixelIndex(where.left()+4,where.top()+(x-4));
        _image->setPixel(where.left()+4,where.top()+(x-4), col );

        col = (dat.data[x] & 0x1)?highCol:lowCol;
        col ^= _image->pixelIndex(where.left()+5,where.top()+(x-4));
        _image->setPixel(where.left()+5,where.top()+(x-4), col );
    }

    updateImage();
}

void CDG::cdgDrawTile(cdg_packet dat) {
    int lowCol = dat.data[0] & 0x0F,
            highCol = dat.data[1] & 0x0F,
            row = dat.data[2] & 0x1F,
            column = dat.data[3] & 0x3F,
            x;
    QRectF where = rectForTile(column,row);

    for( x=4;x<16;x++ ) {
        _image->setPixel(where.left(),where.top()+(x-4), (dat.data[x] & 0x20)?highCol:lowCol );
        _image->setPixel(where.left()+1,where.top()+(x-4), (dat.data[x] & 0x10)?highCol:lowCol );
        _image->setPixel(where.left()+2,where.top()+(x-4), (dat.data[x] & 0x8)?highCol:lowCol );
        _image->setPixel(where.left()+3,where.top()+(x-4), (dat.data[x] & 0x4)?highCol:lowCol );
        _image->setPixel(where.left()+4,where.top()+(x-4), (dat.data[x] & 0x2)?highCol:lowCol );
        _image->setPixel(where.left()+5,where.top()+(x-4), (dat.data[x] & 0x1)?highCol:lowCol );
    }

    updateImage();
}
