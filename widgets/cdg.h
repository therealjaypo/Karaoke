#ifndef CDG_H
#define CDG_H

#include <QWidget>
#include <QFile>
#include <QMediaPlayer>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QRect>

#define SC_MASK     0x3F
#define CDG_COMMAND 0x09

#define CDG_MEMORY_PRESET   1
#define CDG_BORDER_PRESET   2
#define CDG_TILE_NORMAL     6
#define CDG_SCROLL_PRESET   20
#define CDG_SCROLL_COPY     24
#define CDG_SET_TRANSPARENT 28
#define CDG_COLOR_LOADLOW   30
#define CDG_COLOR_LOADHIGH  31
#define CDG_TILE_XOR        38

typedef struct {
    unsigned char command;
    unsigned char instruction;
    unsigned char parityQ[2];
    unsigned char data[16];
    unsigned char parityP[4];
} cdg_packet;

namespace Ui {
class CDG;
}

class CDG : public QWidget
{
    Q_OBJECT

public:
    explicit CDG(QWidget *parent = 0);
    ~CDG();
    void setPlayer(QMediaPlayer *qmp);

    // THIS IS PLATFORM DEPENDENT
    // Hacking will be needed if you're not on UNIX
    void handlePacket(cdg_packet pkt);

    void resetImage();
    void updateImage();
    QRectF rectForTile(int x, int y);

    // CDG instructions
    void cdgMemoryPreset(cdg_packet dat);
    void cdgSetTransparent(cdg_packet dat);
    void cdgLoadLowColours(cdg_packet dat);
    void cdgLoadHighColours(cdg_packet dat);
    void cdgDrawTile(cdg_packet dat);
    void cdgDrawTileXOR(cdg_packet dat);

    void resizeEvent(QResizeEvent *);


protected slots:
    void playerPositionChanged(qint64);
    void playerMediaChanged(QMediaContent);
    void playerStateChanged(QMediaPlayer::State);

protected:
    int _stream;
    QMediaPlayer *_player;
    QImage *_image;
    QGraphicsPixmapItem *_item;
    QGraphicsScene *_scene;
    int _xparentIdx;
    qint64 _curSector;

private:
    Ui::CDG *ui;

};

#endif // CDG_H
