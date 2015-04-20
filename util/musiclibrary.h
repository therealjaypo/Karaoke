#ifndef MUSICLIBRARY_H
#define MUSICLIBRARY_H

#include <QSqlDatabase>
#include <QList>
#include <QString>

class MusicLibrary
{
public:
    static void setDatabase(QSqlDatabase *db);
    static void updateSongsInPath(QString path);

private:
    static QSqlDatabase *database;

    static void _scanFiles(QList<QString> *foundFiles, QString path);
};

#endif // MUSICLIBRARY_H
