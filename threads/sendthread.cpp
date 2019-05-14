#include <QDateTime>
#include <QDebug>
#include "sendthread.h"

SendThread::SendThread(QObject *parent)
    : QThread(parent),
      cmd(0),
      ena(false),
      timer(new QTimer(this))
{
    for(int i=0;i<240;i++)
        trans[i] = 0;
}

void SendThread::run()
{
    ena = true;
    int id(0);
    u8 xo(0);
    while(1)
    {
        if(!ena)
            return;
        if(queSend->size() != 0)
        {
            if(cmd == 0)
            {
                u8 ch(queSend->dequeue());
                if((ch == 0xfd || ch == 0xfc) && queSend->size() > 0)
                {
                    querea[240].clear();
                    querea[240].enqueue(ch);
                    querea[240].enqueue(queSend->dequeue());
                    cmd = 1;
                    emit ok(240);
                }
                else if(querea[id].size() == 240)
                {
                    querea[id].push_front(xo);
                    querea[id].push_front(id);
                    querea[id].push_front(240);
                    querea[id].push_front(0xff);
                    querea[id].enqueue(0xfe);
                    cmd = 1;
                    emit ok(id);
                    trans[id] ++;
                    while(trans[++id] != 0)
                        if(id == 240)
                            id = -1;
                    xo = 0;
                }
                else if(queSend->size() == 0 && ch < 240)
                {
                    querea[id].enqueue(ch);
                    xo ^= ch;
                    querea[id].push_front(xo);
                    querea[id].push_front(id);
                    querea[id].push_front(querea[id].size()-2);
                    querea[id].push_front(0xff);
                    querea[id].enqueue(0xfe);
                    cmd = 1;
                    emit ok(id);
                    trans[id] ++;
                    while(trans[++id] != 0)
                        if(id == 240)
                            id = -1;
                    xo = 0;
                }
                else if(ch <= 240)
                {
                    querea[id].enqueue(ch);
                    xo ^= ch;
                }
                else
                {
                    msleep(1);
                }
            }
        }
        else
        {
            msleep(1);
        }
    }
}

void SendThread::sltOper(int id, bool retry)
{
    if(!retry)
    {
        querea[id].clear();
        trans[id] = 0;
    }
    else
    {
        while(cmd == 1)
            msleep(1);
        emit ok(id);
        trans[id] ++;
        if(trans[id] == 5)
        {
            querea[id].clear();
            trans[id] = 0;
        }
    }
}
