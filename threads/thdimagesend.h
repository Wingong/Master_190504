#ifndef TESTTHREAD_H
#define TESTTHREAD_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QQueue>
#include <QRegion>
#include "user.h"
#include "serial.h"

class ThdImageSend : public QThread
{
    Q_OBJECT
public:
    u8 cnt;
    bool    ena = false;
    int     sta = 0;
    QVector<QVector<bool> >&mart;
    QQueue<u8>  &queRecv;
    ThdImageSend(QVector<QVector<bool> >&mat,QQueue<u8>&inQueRecv,QObject *parent);
    void run();
signals:
    void rep();
};

#endif // TESTTHREAD_H
