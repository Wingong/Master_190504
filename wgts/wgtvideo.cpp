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
      labPort(new QLabel("蓝牙串口",this)),
      labPaint(new QLabel(this)),
      labFPS(new QLabel("帧率：",this)),
      labInfo(new QLabel("输出信息：",this)),
      txtFPS(new QLineEdit(this)),
      txtInfo(new QTextEdit(this)),
      txtData(new QTextEdit(this)),
      cbxPort(new MyComboBox(0,this)),
      btnRefresh(new QPushButton("刷新",this)),
      btnToggle(new QPushButton("打开",this)),
      btnClear(new QPushButton("清除",this)),
      labSPort(new QLabel("链路串口",this)),
      cbxSPort(new MyComboBox(1,this)),
      rbtDirR(new QRadioButton("接收",this)),
      rbtDirS(new QRadioButton("发送",this)),
      rbtDatVideo(new QRadioButton("视频",this)),
      rbtDatVoice(new QRadioButton("语音",this)),
      btgDir(new QButtonGroup(this)),
      btgDat(new QButtonGroup(this)),
      btnSToggle(new QPushButton("打开",this)),
      serRecv(new Serial),
      serSend(new Serial),
      drawTimer(new QTimer(this)),
      fpsTimer(new QTimer(this)),
      mat(WIDTH, QVector<bool>(HEIGHT,false)),
      thread(new TestThread(mat,queRecv,this))
{
    (*cbxPort) << serRecv->name;
    (*cbxSPort) << serSend->name;
    labPort->resize(80,25);
    labPaint->move(20,20);
    labPaint->installEventFilter(this);
    labFPS->resize(80,25);
    labInfo->resize(80,25);
    txtFPS->resize(80,25);
    txtFPS->setText("0");
    txtFPS->setReadOnly(true);
    txtData->move(20,20);
    txtData->setEnabled(false);
    txtData->setVisible(false);
    //txtInfo->setReadOnly(true);
    cbxPort->resize(90,25);
    btnRefresh->resize(80,25);
    btnToggle->resize(80,25);
    btnToggle->setEnabled(false);
    btnClear->resize(80,25);
    labSPort->resize(80,25);
    cbxSPort->resize(90,25);
    rbtDirR->resize(80,25);
    rbtDirS->resize(80,25);
    rbtDatVideo->resize(80,25);
    rbtDatVoice->resize(80,25);
    btgDir->addButton(rbtDirR);
    btgDir->addButton(rbtDirS);
    btgDat->addButton(rbtDatVideo);
    btgDat->addButton(rbtDatVoice);
    rbtDirR->setChecked(true);
    rbtDatVideo->setChecked(true);
    btnSToggle->resize(80,25);
    connect(btnRefresh,SIGNAL(clicked(bool)),this,SLOT(sltRefresh()));
    connect(btnToggle,SIGNAL(clicked(bool)),this,SLOT(sltToggle()));
    connect(btnClear,SIGNAL(clicked(bool)),this,SLOT(sltClear()));
    connect(btnSToggle,SIGNAL(clicked(bool)),this,SLOT(sltToggle()));
    connect(btgDir,SIGNAL(buttonToggled(int,bool)),this,SLOT(sltDirTog(int,bool)));
    connect(btgDat,SIGNAL(buttonToggled(int,bool)),this,SLOT(sltDatTog(int,bool)));
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
    serSend->refresh();
    cbxPort->clear();
    cbxSPort->clear();
    (*cbxPort) << serRecv->name;
    (*cbxSPort) << serSend->name;
    qDebug() << "refreshed\n";
}

void WgtVideo::sltToggle()
{
    QPushButton *act = qobject_cast<QPushButton *>(sender());
    if(act == btnToggle)
    {
        if(opened)
        {
            fpsTimer->stop();
            //thread->ena = false;
            cbxPort->setEnabled(true);
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
            cbxPort->setEnabled(false);
            btnRefresh->setEnabled(false);
            btnToggle->setText("关闭");
            opened = true;
            fpsTimer->start();
        }
    }
    else
    {
        if(btnSToggle->text() == "关闭")
        {
            cbxSPort->setEnabled(true);
            btnRefresh->setEnabled(true);
            rbtDirR->setEnabled(true);
            rbtDirS->setEnabled(true);
            rbtDatVideo->setEnabled(true);
            rbtDatVoice->setEnabled(true);
            btnToggle->setTabletTracking("打开");
        }
        else
        {
            serSend->setPortName(cbxSPort->currentText());
            if(!serSend->open(QIODevice::ReadWrite))
            {
                QMessageBox::critical(this,"错误","打开失败！",QMessageBox::Ok);
                return;
            }
            cbxSPort->setEnabled(false);
            btnRefresh->setEnabled(false);
            rbtDirR->setEnabled(false);
            rbtDirS->setEnabled(false);
            rbtDatVideo->setEnabled(false);
            rbtDatVoice->setEnabled(false);
            serSend->setBaudRate(115200);
            serSend->setDataBits(QSerialPort::Data8);
            serSend->setFlowControl(QSerialPort::NoFlowControl);
            serSend->setParity(QSerialPort::NoParity);
            serSend->setStopBits(QSerialPort::OneStop);
            btnSToggle->setText("关闭");
        }
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
    int prev_status(0);
    while(queRecv.size())
    {
        u8 ch(queRecv.dequeue());
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
}

void WgtVideo::sltReadBuf()
{
    if(rbtDatVideo->isChecked() && rbtDirS->isChecked())
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
    else if(rbtDatVideo->isChecked() && rbtDirR->isChecked())
    {
        QByteArray src = serSend->readAll();
        for(auto i:src)
        {
            queRecv.enqueue(i);
            queSend.enqueue(i);
        }
        emit readOK();
        sltRecv();
    }
    else if(rbtDatVoice->isChecked() && rbtDirS->isChecked())
    {
        QByteArray src = serRecv->readAll();
        QString txt(txtData->toPlainText());
        for(auto i:src)
        {
            queSend.enqueue(i);
            txt.append(HEX(i/16));
            txt.append(HEX(i%16));
            txt.append(32);
        }
        txtData->setText(txt);
        emit readOK();
    }
    else
    {
        QByteArray src = serSend->readAll();
        QString txt(txtData->toPlainText());
        for(auto i:src)
        {
            queSend.enqueue(i);
            txt.append(HEX(i/16));
            txt.append(HEX(i%16));
            txt.append(32);
        }
        emit readOK();
    }
}

void WgtVideo::sltDirTog(int index,bool b)
{
    if(b)
    {
        if(index == -2)
        {
            if(rbtDatVideo->isChecked())
            {
                btnToggle->setEnabled(false);
            }
            else
            {
                btnToggle->setEnabled(true);
            }
        }
        else
        {
            btnToggle->setEnabled(true);
        }
    }
}

void WgtVideo::sltDatTog(int index,bool b)
{
    if(b)
    {
        if(index == -2)
        {
            if(rbtDirR->isChecked())
            {
                btnToggle->setEnabled(false);
            }
            else
            {
                btnToggle->setEnabled(true);
            }
            labPaint->setEnabled(true);
            labPaint->setVisible(true);
            txtData->setEnabled(false);
            txtData->setVisible(false);
        }
        else
        {
            labPaint->setEnabled(false);
            labPaint->setVisible(false);
            txtData->setEnabled(true);
            txtData->setVisible(true);
            btnToggle->setEnabled(true);
        }
    }
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
    labPort->move(20,siz.height()-65);
    labPaint->resize(siz.width()-273,siz.height()-127);
    labFPS->move(siz.width()-233,20);
    txtFPS->move(siz.width()-183,20);
    labInfo->move(siz.width()-233,60);
    txtInfo->setGeometry(siz.width()-233,92,213,siz.height()-152);
    txtData->resize(siz.width()-273,siz.height()-127);
    cbxPort->move(90,siz.height()-65);
    btnRefresh->move(20,siz.height()-100);
    btnToggle->move(190,siz.height()-65);
    btnClear->move(siz.width()-100,siz.height()-40);
    labSPort->move(20,siz.height()-30);
    cbxSPort->move(90,siz.height()-30);
    btnSToggle->move(190,siz.height()-30);
    rbtDirR->move(110,siz.height()-100);
    rbtDirS->move(175,siz.height()-100);
    rbtDatVideo->move(280,siz.height()-65);
    rbtDatVoice->move(280,siz.height()-30);
    genRects();
}

#ifdef DEBUG
void WgtVideo::fps()
{
    txtFPS->setText(QString::number(cnt));
    cnt = 0;
}
#endif
