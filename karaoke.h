#ifndef KARAOKE_H
#define KARAOKE_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QSqlDatabase>

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
    QSqlDatabase dbase;

private slots:
    void playPushed(bool state);
    void selectFile();
    void playerStateChanged(QMediaPlayer::State state);
    void playerPositionChanged(qint64 pos);
    void addLibraryPath();
    void refreshPath();
    void librarySelect();
    void libraryPlay();
};

#endif // KARAOKE_H
