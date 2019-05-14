#ifndef SENDTHREAD_H
#define SENDTHREAD_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QString>
#include <QTimer>

typedef unsigned char u8;

class SendThread : public QThread
{
    Q_OBJECT
public:
    SendThread(QObject *parent);
    void run(void);
    QQueue<u8>	*queSend;
    QQueue<u8>	*queRecv;
    QQueue<u8>	querea[241];
    int			trans[240];
    QString		txtRecv;
    int			cmd;
    bool		ena;
    bool		retry;
    QTimer		*timer;

signals:
    void ok(int id);

public slots:
    void sltOper(int id,bool retry);
};

#endif // SENDTHREAD_H
