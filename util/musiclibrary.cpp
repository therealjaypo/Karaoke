#include "util/musiclibrary.h"

#include <QDir>
#include <QDebug>
#include <QSqlQuery>
#include <QDirIterator>
#include <QStringList>

QSqlDatabase *MusicLibrary::database = NULL;

void MusicLibrary::setDatabase(QSqlDatabase *db) {
    database = db;
}

void MusicLibrary::_scanFiles(QList<QString> *foundFiles, QString path) {
    QDirIterator dirIt(path,QDirIterator::Subdirectories);
    QFileInfo qfi;
    QString fpath;
    QString ext;
    int x;

    while( dirIt.hasNext() ) {
        dirIt.next();

        fpath = dirIt.filePath();
        qfi =QFileInfo(fpath);
        if( qfi.isFile() ) {
            x = fpath.lastIndexOf('.');
            ext = fpath.left(x);

            if( fpath.right(fpath.length()-x) == ".mp3" && QFileInfo(QString(ext).append(".cdg")).exists() ) {
                foundFiles->append(fpath);
            }
        } else if( qfi.isDir() && qfi.fileName() != "." && qfi.fileName() != ".." ) {
            qDebug() << "Scan directory" << qfi.fileName();
            _scanFiles(foundFiles,dirIt.filePath());
        }
    }
}

void MusicLibrary::updateSongsInPath(QString path) {
    int pathid = -1;
    QSqlQuery query;
    QList<QString> *files = new QList<QString>();
    QList<QString> dbFiles;
    QString fname;
    QStringList parts;
    QString title, artist;
    int x,pos;

    query.prepare("SELECT id FROM library_path WHERE path = ?");
    query.addBindValue(path);
    query.exec();

    while( query.next() ) {
        if( pathid != -1 ) {
            qDebug() << "Too many results for path!";
            pathid = -1;
            break;
         } else {
            pathid = query.value("id").toInt();
        }
    }

    if( pathid == -1 ) {
        qWarning() << "Directory" << path << "not found in database";
    }
    qDebug() << "Working with path" << pathid;

    _scanFiles(files,path);
    qDebug() << "Found" << files->length() << "files";

    query.prepare("SELECT file FROM song WHERE path = ?");
    query.addBindValue(pathid);
    query.exec();

    while( query.next() ) {
        dbFiles.append(query.value("file").toString());
    }
    qDebug() << "Found" << dbFiles.length() << "songs in DB";

    // ADD FILES THAT DON'T EXIST TO DATABASE
    for( x=0;x<files->length();x++ ) {
        fname = files->at(x);

        if( dbFiles.indexOf(fname) == -1 ) {
            if( fname.indexOf(" - ") == -1 ) {
                qWarning() << "Don't know how to parse" << fname;
            } else {
                parts = fname.split(" - ");
                artist = parts.at(0);
                pos = artist.lastIndexOf(QDir::separator()) + 1;
                artist = artist.right(artist.length() - pos);

                parts.removeAt(0);
                title = parts.join(" - ");

                title = title.replace(".mp3","");
                title = title.replace(" [karaoke]","");

                qDebug() << "Add" << fname << "(" << title << "by" << artist << "to database";
                query.prepare("INSERT INTO song (path,artist,title,file) VALUES (?,?,?,?)");
                query.addBindValue(pathid);
                query.addBindValue(artist);
                query.addBindValue(title);
                query.addBindValue(fname);
                if( !query.exec() ) {
                    qWarning() << "Insert error:" << query.executedQuery();
                }

            }
        }
    }
}

