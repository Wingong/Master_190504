#include "recvthread.h"
#include <QDebug>

#define HEX(X) ((X)<10?(X)+48:(X)+55)

RecvThread::RecvThread(QObject *parent)
    : QThread(parent),
      ena(false)
{
}

void RecvThread::run()
{
    int count(0);
    int id=(0);
    int sta(-1);
    u8 xi(0);
    u8 xo(0);
    QQueue<u8> queproc;
    ena = true;
    while(1)
    {
        if(!ena)
            return;
        if(queRecv->size() != 0)
        {
            u8 ch(queRecv->dequeue());
            if(sta == -1 && ch > 0xf0)
            {
                switch(ch)
                {
                case 0xff:
                    sta = 0x01;
                    break;
                case 0xfd:
                    sta = 0x05;
                    break;
                case 0xfc:
                    sta = 0x07;
                    break;
                }
                continue;
            }
            else if(sta == 0 && ch == 0xfe)
            {
                if(xo == xi && count == 0)
                {
                    for(auto i:queproc)
                    {
                        txtRecv.append(HEX(i/16));
                        txtRecv.append(HEX(i%16));
                        txtRecv.append(32);
                    }
                    emit recv();
                    queSend->append(0xfd);
                    queSend->append(id);
                }
                else
                {
                    queSend->append(0xfc);
                    queSend->append(id);
                }
                sta = -1;
                continue;
            }
            else if(sta == 0x00 && ch == 0xf0)
            {
                sta = 0xf0;
                count --;
                xo ^= ch;
                continue;
            }
            else if(sta == 0xf0 || sta == 0x00)
            {
                queproc.append(ch);
                count --;
                xo ^= ch;
                sta = 0;
                continue;
            }
            switch(sta)
            {
            case 0x01:
                if(ch <= 240)
                {
                    count = ch;
                    sta ++;
                }
                else
                {
                    queSend->append(0xfd);
                    queSend->append(id);
                    sta = -1;
                }
                break;
            case 0x02:
                id = ch;
                sta ++;
                break;
            case 0x03:
                xi = ch;
                xo = 0;
                queproc.clear();
                sta = 0;
                break;
            case 0x05:
                emit oper(ch,false);
                sta = -1;
                break;
            case 0x07:
                emit oper(ch,true);
                sta = -1;
                break;
            }
        }
        else
        {
            msleep(1);
        }
    }
}
