#include <QDateTime>
#include <QDebug>
#include "sendthread.h"

SendThread::SendThread()
{

}

void SendThread::run()
{
    QDateTime dt(QDateTime::currentDateTime());
    qDebug() << dt.time().hour() << ':' << dt.time().minute() << ':' << dt.time().second();
}
