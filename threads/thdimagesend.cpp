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
//    Serial *serTemp(rbtDirR->isChecked()?serBt:serCh);
//    if(queSend.size()<200)
//        return;
//    qDebug() << queSend.size();
//    QByteArray arr;
//    for(int i=0;i<queSend.size();i++)
//    {
//        arr.append(queSend.dequeue());
//    }
//    arr.push_front(0x17);
//    arr.push_front(addr[1]);
//    arr.push_front(addr[0]);
//    serTemp->write(arr);
//    queSend.clear();
//    qDebug() << "send";
    arr.clear();
    int time=WIDTH*HEIGHT/8/200;
    int count = 0;
    for(int i=0;i<time;i++)
    {
        arr.push_front(0x17);
        arr.push_front(addr[1]);
        arr.push_front(addr[0]);
        for(int j=0;j<200;j++)
            arr.append(ques[count++]);
        emit readOK();
        msleep(200);
    }
    arr.append(addr[0]);
    arr.append(addr[1]);
    arr.append(0x17);
    while(count != WIDTH*HEIGHT/8+4)
    {
        arr.append(ques[count++]);
    }
    qDebug() << "arr: " << arr.size();
    emit readOK();
    msleep(200);
    send_busy = false;
}
