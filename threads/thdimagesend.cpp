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

void ThdImageSend::run()
{
    send_busy = true;
    emit readOK();
    msleep(300);
    send_busy = false;
}
