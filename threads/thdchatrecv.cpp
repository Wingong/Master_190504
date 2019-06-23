#include "thdchatrecv.h"
#include <QDebug>

ThdChatRecv::ThdChatRecv(QObject *parent)
    : QThread(parent),
      ena(false)
{
}

void ThdChatRecv::run()
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
                case User::BEG:
                    sta = User::LEN;
                    break;
                case User::ACK:
                    sta = User::AOP;
                    break;
                case User::ERR:
                    sta = User::EOP;
                    break;
                }
                continue;
            }
                        //当前在接收状态，接到结束指令，开始帧终末裁决
            else if(sta == User::REV && ch == User::END)
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
                    queSend->append(User::ACK);
                    queSend->append(id);
                }
                else
                {
                    queSend->append(User::ERR);
                    queSend->append(id);
                }
                sta = User::IDL;
                continue;
            }
                        //转义字符，跳过该字节，下一个字节无脑接收
            else if(sta == User::REV && ch == User::TRS)
            {
                sta = User::TRS;
                count --;
                xo ^= ch;
                continue;
            }
            else if(sta == User::TRS || sta == User::REV)
            {
                queproc.append(ch);
                count --;
                xo ^= ch;
                sta = User::REV;
                continue;
            }
                        //根据次态选择下一个状态
            switch(sta)
            {
            case User::LEN:
                if(ch <= 240)
                {
                    count = ch;
                    sta ++;
                }
                else
                {
                    queSend->append(User::ACK);
                    queSend->append(id);
                    sta = User::IDL;
                }
                break;
            case User::RID:
                id = ch;
                sta ++;
                break;
            case User::XOR:
                xi = ch;
                xo = 0;
                queproc.clear();
                sta = User::REV;
                break;
            case User::AOP:
                emit oper(ch,false);
                sta = User::IDL;
                break;
            case User::EOP:
                emit oper(ch,true);
                sta = User::IDL;
                break;
            }
        }
        else
        {
            msleep(1);
        }
    }
}
