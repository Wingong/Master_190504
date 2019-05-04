#ifndef SENDTHREAD_H
#define SENDTHREAD_H

#include <QObject>
#include <QThread>

class SendThread : public QThread
{
public:
    SendThread();
    void run(void);
};

#endif // SENDTHREAD_H
