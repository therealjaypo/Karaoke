#include "karaoke.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Karaoke w;
    w.show();

    return a.exec();
}
