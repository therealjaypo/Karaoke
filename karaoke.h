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
    void testPushed();
    void playPushed(bool state);
    void selectFile();
};

#endif // KARAOKE_H
