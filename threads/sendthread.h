#ifndef SENDTHREAD_H
#define SENDTHREAD_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QString>
#include <QTimer>
#include "user.h"

class SendThread : public QThread
{
    Q_OBJECT
public:
    SendThread(QObject *parent);
    void run(void);
    QQueue<u8>  *queSend;
    QQueue<u8>  *queRecv;
    QQueue<u8>  querea[241];    //接收队列数组，按ID排列
    int         trans[240];     //对应数组是否传送完毕，true=还没结束
    QString     txtRecv;
    int         cmd;            //是否工作
    bool        ena;            //使能
    bool        retry;          //重试发送
    QTimer      *timer;

signals:
    void ok(int id);            //内容全部传送至querea[id]后的信号

public slots:                   //根据ACK或ERR进行操作
    void sltOper(int id,bool retry);
};

#endif // SENDTHREAD_H
