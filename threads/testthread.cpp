#include "testthread.h"
#include <QDebug>

TestThread::TestThread(QVector<QVector<bool> >&mat, QQueue<u8> &inQueRecv, QObject *parent)
    : QThread(parent),
      mart(mat),
      queRecv(inQueRecv)
{

}

void TestThread::run()
{
    while(ena)
    {
        if(queRecv.size() > 0)
        {
            u8 ch(queRecv.dequeue());
            if(ch == 0x01 || ch == 0xfe)
            {
                if(queRecv.size() > 0)
                {
                    u8 chn(queRecv.dequeue());
                    if(ch == 0x01)
                    {
                        //if(chn == 0xfe)
                        //    sta = 1;
                        //else

                    }
                    else if(ch == 0xfe && chn == 0x01)
                    {
                        sta = 0;
                    }
                }
            }
        }
        for(int i=0;i<WIDTH;i++)
        {
            for(int j=0;j<HEIGHT;j++)
            {
                mart[i][j] = !((i*WIDTH+j+cnt)%30);
            }
        }
        cnt ++;
        emit rep();
        qDebug() << "rep";
        msleep(10);
    }
}
