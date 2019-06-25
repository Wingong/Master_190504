#ifndef TESTTHREAD_H
#define TESTTHREAD_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QQueue>
#include <QRegion>
#include <QByteArray>
#include <QFile>
#include "user.h"
#include "serial.h"

class ThdImageSend : public QThread
{
    Q_OBJECT
public:
    u8 cnt;
    bool    ena = false;
    bool    send_busy;
    int     sta = 0;
    u8      *ques;
    u8      *addr;
    QByteArray arr;
    QString *path;
    QFile   *file;
    QByteArray tosend;
    //QQueue<u8>  &queRecv;
    ThdImageSend(u8 *queSend, u8 *address, QByteArray &arri,QObject *parent);
    ThdImageSend(QObject *parent);
    void    run();
protected:
    const int threhold = 0xBBF;
signals:
    void    readOK();
};

#endif // TESTTHREAD_H
