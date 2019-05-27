#include <QSize>
#include <QResizeEvent>
#include <QString>
#include <QByteArray>
#include <QMessageBox>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include "wgtvideo.h"

WgtVideo::WgtVideo(QWidget *parent)
    : QWidget(parent),
      dw(1),dh(1),tx(0),cnt(0),index(0),status(0),recv_count(0),opened(false),
      labPort(new QLabel("选择串口",this)),
      labPaint(new QLabel(this)),
      labFPS(new QLabel("帧率：",this)),
      labInfo(new QLabel("输出信息：",this)),
      txtFPS(new QLineEdit(this)),
      txtInfo(new QTextEdit(this)),
      cbxPort(new MyComboBox(0,this)),
      btnRefresh(new QPushButton("刷新",this)),
      btnToggle(new QPushButton("打开",this)),
      btnClear(new QPushButton("清除",this)),
      serRecv(new Serial),
      serSend(new Serial),
      drawTimer(new QTimer(this)),
      fpsTimer(new QTimer(this)),
      mat(WIDTH, QVector<bool>(HEIGHT,false)),
      thread(new TestThread(mat,queRecv,this))
{
    (*cbxPort) << serRecv->name;
    labPort->resize(80,25);
    labPaint->move(20,20);
    labPaint->installEventFilter(this);
    labFPS->resize(80,25);
    labInfo->resize(80,25);
    txtFPS->resize(80,25);
    txtFPS->setText("0");
    txtFPS->setReadOnly(true);
    //txtInfo->setReadOnly(true);
    cbxPort->resize(90,25);
    btnRefresh->resize(80,25);
    btnToggle->resize(80,25);
    btnClear->resize(80,25);
    connect(btnRefresh,SIGNAL(clicked(bool)),this,SLOT(sltRefresh()));
    connect(btnToggle,SIGNAL(clicked(bool)),this,SLOT(sltToggle()));
    connect(btnClear,SIGNAL(clicked(bool)),this,SLOT(sltClear()));
    connect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
    serRecv->setBaudRate(115200);
    serRecv->setFlowControl(QSerialPort::NoFlowControl);
    serRecv->setParity(QSerialPort::NoParity);
    serRecv->setStopBits(QSerialPort::OneStop);

    connect(thread,SIGNAL(rep()),this,SLOT(update()));
    connect(this,SIGNAL(readOK()),this,SLOT(sltSend()));
    drawTimer->setInterval(10);
#ifdef DEBUG
    connect(fpsTimer,SIGNAL(timeout()),this,SLOT(fps()));
    fpsTimer->setInterval(1000);
#endif
}

WgtVideo::~WgtVideo()
{
    thread->ena = false;
    thread->exit();
    fpsTimer->stop();
    drawTimer->stop();
}

void WgtVideo::genRects()
{
    dw = (qreal)labPaint->width()/WIDTH;
    dh = (qreal)labPaint->height()/HEIGHT;
    for(int i=0;i<WIDTH;i++)
    {
        for(int j=0;j<HEIGHT;j++)
        {
            rects[i][j].setRect((int)(i*dw),
                                (int)(j*dh),
                                (int)(dw),
                                (int)(dh));
        }
    }
}

void WgtVideo::sltRefresh()
{
    serRecv->refresh();
    cbxPort->clear();
    (*cbxPort) << serRecv->name;
    qDebug() << "refreshed\n";
}

void WgtVideo::sltToggle()
{
    if(opened)
    {
        fpsTimer->stop();
        //thread->ena = false;
        btnRefresh->setEnabled(true);
        btnToggle->setText("打开");
        serRecv->close();
        opened = false;
    }
    else
    {
        serRecv->setPortName(cbxPort->currentText());
        if(!serRecv->open(QIODevice::ReadWrite))
        {
            QMessageBox::critical(this,"错误","打开失败！",QMessageBox::Ok);
            return;
        }
        status = 0;
        thread->ena = true;
        //thread->start();
        serRecv->setBaudRate(115200);
        serRecv->setDataBits(QSerialPort::Data8);
        serRecv->setFlowControl(QSerialPort::NoFlowControl);
        serRecv->setParity(QSerialPort::NoParity);
        serRecv->setStopBits(QSerialPort::OneStop);
        btnRefresh->setEnabled(false);
        btnToggle->setText("关闭");
        opened = true;
        fpsTimer->start();
    }
}

void WgtVideo::sltClear()
{
    index = 0;
    txtInfo->setText("");
    recv_count = 0;
    status = 0;
}

void WgtVideo::sltRecv()
{
    //disconnect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
    int prev_status(0);
    while(queRecv.size())
    {
        u8 ch(queRecv.dequeue());
        //if(queRecv.size() > 1 && ch == 0x01 && queRecv[1] == 0xfe)
        //{
        //    if(status == 1)
        //        txtInfo->append("Expected end!\n");
        //    index = 0;
        //    queRecv.dequeue();
        //    queRecv.dequeue();
        //    qDebug() << "start\n";
        //    status = 1;
        //    continue;
        //}
        //else if(queRecv.size() > 1 && queRecv[0] == 0xfe && queRecv[1] == 0x01)
        //{
        //    if(index == WIDTH*HEIGHT)
        //        emit repaint();
        //    else
        //        txtInfo->append("Length wrong!\n");
        //    status = 0;
        //    queRecv.dequeue();
        //    queRecv.dequeue();
        //    qDebug() << "end1" << '\t' << index << '\n';
        //    continue;
        //}
        //else
        //{
        recv_count ++;
        if(index == 13512)
            qDebug() << "Break!";
        switch(status) {
        case 0:
            if(ch == 0x01)
            {
                status = 0x04;
            }
            break;
        case 1:
            if(ch == 0x01)
                status = 0x02;
            else if(ch == 0xfe)
                status = 0x03;
            break;
        case 2:
            if(ch == 0xfe && index > WIDTH*HEIGHT-32)
            {
                index = 0;
                status = 1;
                qDebug() << "start\n";
                continue;
            }
            else
            {
                status = 1;
                prev_status = 0x01;
            }
            break;
        case 3:
            if(ch == 0x01 && queRecv.size()>1 && queRecv[0] == 0x01 && queRecv[1] == 0xfe)
            {
                emit repaint();
                if(index != WIDTH*HEIGHT)
                    txtInfo->append("Length wrong!\n");
                status = 1;
                index = 0;
                ch = queRecv.dequeue();
                ch = queRecv.dequeue();
                qDebug() << "end" << '\t' << index << '\n';
                continue;
            }
            else if(ch == 0x01 && index > WIDTH*HEIGHT-32)
            {
                emit repaint();
                if(index != WIDTH*HEIGHT)
                    txtInfo->append("Length wrong!\n");
                status = 0;
                index = 0;
                qDebug() << "end" << '\t' << index << '\n';
                continue;
            }
            else
            {
                status = 1;
                prev_status = 0xfe;
            }
            break;
        case 4:
            if(ch == 0xfe)
            {
                index = 0;
                status = 1;
                qDebug() << "start_0\n";
            }
            else if(ch != 0x01)
            {
                status = 0;
            }
            continue;
            break;
        default:
            break;
        }
        if(status == 1)
        {
            if(prev_status != 0)
            {
                //qDebug() << prev_status << '\t' << index;
                if(index < WIDTH*HEIGHT)
                {
                    for(int i=0;i<8;i++)
                    {
                        mat[index%WIDTH][index/WIDTH] = prev_status & 0x80;
                        prev_status <<= 1;
                        index ++;
                    }
                    prev_status = 0;
                }
                else
                {
                    status = 0;
                    txtInfo->append("Too much data!\n");
                    index = 0;
                    prev_status = 0;
                    continue;
                }
            }

            //qDebug() << ch << '\t' << index;
            if(index < WIDTH*HEIGHT)
            {
                for(int i=0;i<8;i++)
                {
                    mat[index%WIDTH][index/WIDTH] = ch & 0x80;
                    ch <<= 1;
                    index ++;
                }
            }
            else
            {
                status = 0;
                txtInfo->append("Too much data!\n");
                index = 0;
                status = 0;
            }
        }
        //}
    }
    //connect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
}

void WgtVideo::sltReadBuf()
{
    QByteArray src = serRecv->readAll();
    for(auto i:src)
    {
        queRecv.enqueue(i);
        queSend.enqueue(i);
    }
    emit readOK();
    sltRecv();
}

void WgtVideo::sltSend()
{}

void WgtVideo::updateBoard()
{
    QPainter painter(this->labPaint);
    QPen pen(Qt::black,1);
    QBrush brush(Qt::black);
    QBrush backbrush(Qt::white);
    painter.setBrush(backbrush);
    painter.drawRect(this->labPaint->rect());
    painter.setBackground(brush);
    painter.setPen(pen);
    painter.setBrush(brush);
    painter.setBackgroundMode(Qt::OpaqueMode);
    for(int i=0;i<WIDTH;i++)
        for(int j=0;j<HEIGHT;j++)
            if(mat[i][j])
                painter.drawRect(rects[i][j]);
    cnt ++;
}

bool WgtVideo::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == labPaint && event->type() == QEvent::Paint)
    {
        updateBoard();
    }
    return QWidget::eventFilter(watched, event);
}

void WgtVideo::resizeEvent(QResizeEvent *event)
{
    QSize siz(event->size());
    labPort->move(20,siz.height()-40);
    labPaint->resize(siz.width()-190,siz.height()-80);
    labFPS->move(siz.width()-150,20);
    txtFPS->move(siz.width()-100,20);
    labInfo->move(siz.width()-150,60);
    txtInfo->setGeometry(siz.width()-150,92,130,siz.height()-150);
    cbxPort->move(90,siz.height()-40);
    btnRefresh->move(190,siz.height()-40);
    btnToggle->move(280,siz.height()-40);
    btnClear->move(370,siz.height()-40);
    genRects();
    qDebug() << labPaint->width() << '\t' << labPaint->height();
}

#ifdef DEBUG
void WgtVideo::fps()
{
    txtFPS->setText(QString::number(cnt));
    cnt = 0;
}
#endif
