#ifndef KARAOKE_H
#define KARAOKE_H

#include <QMainWindow>
#include <QMediaPlayer>

namespace Ui {
class Karaoke;
}

class Karaoke : public QMainWindow
{
    Q_OBJECT

public:
    explicit Karaoke(QWidget *parent = 0);
    ~Karaoke();

protected:
    QMediaPlayer *_player;

private:
    Ui::Karaoke *ui;

private slots:
    void playPushed(bool state);
    void selectFile();
    void playerStateChanged(QMediaPlayer::State state);
};

#endif // KARAOKE_H
