#include "recvthread.h"
#include <QDebug>

RecvThread::RecvThread(QObject *parent)
    : QThread(parent),
      ena(false)
{
}

void RecvThread::run()
{
    int count(0);       //数据长度
    int id=(0);         //数据ID
    int sta(-1);        //决定下一个数据的状态
    u8 xi(0);           //输入的按位异或值
    u8 xo(0);           //实际计算的按位异或值
    QQueue<u8> queproc; //处理中的数据队列
    ena = true;
    while(1)
    {
        if(!ena)
            return;     //如果输入缓冲不为空
        if(queRecv->size() != 0)
        {               //输出队列弹出至ch
            u8 ch(queRecv->dequeue());
                        //帧序列
                        //数据帧：0xff  len  id  xor  Contents  0xfe
                        //ACK：  0xfd  id
                        //ERR：  0xfc  id

                        //当前在等待状态，且接到指令(>0xf0)，则根据指令选择次态
            if(sta == -1 && ch > 0xf0)
            {
                switch(ch)
                {
                case BEG:
                    sta = LEN;
                    break;
                case ACK:
                    sta = AOP;
                    break;
                case ERR:
                    sta = EOP;
                    break;
                }
                continue;
            }
                        //当前在接收状态，接到结束指令，开始帧终末裁决
            else if(sta == REV && ch == END)
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
                    queSend->append(ACK);
                    queSend->append(id);
                }
                else
                {
                    queSend->append(ERR);
                    queSend->append(id);
                }
                sta = IDL;
                continue;
            }
                        //转义字符，跳过该字节，下一个字节无脑接收
            else if(sta == REV && ch == TRS)
            {
                sta = TRS;
                count --;
                xo ^= ch;
                continue;
            }
            else if(sta == TRS || sta == REV)
            {
                queproc.append(ch);
                count --;
                xo ^= ch;
                sta = REV;
                continue;
            }
                        //根据次态选择下一个状态
            switch(sta)
            {
            case LEN:
                if(ch <= 240)
                {
                    count = ch;
                    sta ++;
                }
                else
                {
                    queSend->append(ACK);
                    queSend->append(id);
                    sta = IDL;
                }
                break;
            case RID:
                id = ch;
                sta ++;
                break;
            case XOR:
                xi = ch;
                xo = 0;
                queproc.clear();
                sta = REV;
                break;
            case AOP:
                emit oper(ch,false);
                sta = IDL;
                break;
            case EOP:
                emit oper(ch,true);
                sta = IDL;
                break;
            }
        }
        else
        {
            msleep(1);
        }
    }
}
