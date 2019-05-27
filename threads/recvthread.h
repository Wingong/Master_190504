#ifndef RECVTHREAD_H
#define RECVTHREAD_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QString>
#include "user.h"

class RecvThread : public QThread
{
    Q_OBJECT
public:
    RecvThread(QObject *parent);
    void run();
    QQueue<u8>  *queSend;
    QQueue<u8>  *queRecv;
    QString     txtRecv;
    bool        ena;
signals:
    void recv(void);                //接收完成信号
    void oper(int id,bool retry);   //接收ACK帧或ERR帧的操作信号
};

#endif // RECVTHREAD_H
