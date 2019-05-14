#ifndef RECVTHREAD_H
#define RECVTHREAD_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QString>

typedef unsigned char u8;

class RecvThread : public QThread
{
    Q_OBJECT
public:
    RecvThread(QObject *parent);
    void run();
    QQueue<u8>	*queSend;
    QQueue<u8>	*queRecv;
    QString		txtRecv;
    bool		ena;
signals:
    void recv(void);
    void oper(int id,bool retry);
};

#endif // RECVTHREAD_H
