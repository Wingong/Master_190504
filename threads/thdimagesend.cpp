#include "thdimagesend.h"
#include <QDebug>

ThdImageSend::ThdImageSend(u8 *queSend, u8 *address, QByteArray &arri, QObject *parent)
    : QThread(parent),
      send_busy(false),
      ques(queSend),
      addr(address),
      arr(arri)
{

}

ThdImageSend::ThdImageSend(QObject *parent)
    : QThread(parent),
      send_busy(false)
{

}

void ThdImageSend::run()
{
    do
    {
        if(arr.size() < threhold)
        {
            tosend = arr;
            arr.clear();
            emit readOK();
        }
        else
        {
            tosend = arr.left(threhold);
            arr.remove(0,threhold);
            send_busy = true;
            emit readOK();
            while(send_busy);
        }
    }
    while(arr.size());
}
