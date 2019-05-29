#include <QSize>
#include <QCoreApplication>
#include <QTime>
#include <QResizeEvent>
#include <QString>
#include <QByteArray>
#include <QMessageBox>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QRegExp>
#include "wgtvideo.h"

WgtVideo::WgtVideo(QWidget *parent)
    : QWidget(parent),
      dw(1),dh(1),tx(0),cnt(0),index(0),status(0),recv_count(0),send_busy(0),opened(false),
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
      labAddr(new QLabel("地址",this)),
      txtAddr(new QLineEdit("1234",this)),
      cbxSPort(new MyComboBox(1,this)),
      rbtDirR(new QRadioButton("接收",this)),
      rbtDirS(new QRadioButton("发送",this)),
      rbtDatVideo(new QRadioButton("视频",this)),
      rbtDatVoice(new QRadioButton("语音",this)),
      btgDir(new QButtonGroup(this)),
      btgDat(new QButtonGroup(this)),
      btnSToggle(new QPushButton("打开",this)),
      serBt(new Serial),
      serCh(new Serial),
      drawTimer(new QTimer(this)),
      fpsTimer(new QTimer(this)),
      delayTimer(new QTimer(this)),
      mat(WIDTH, QVector<bool>(HEIGHT,false)),
      thread(new ThdImageSend(mat,queRecv,this))
{
    ques[0] = 0x01;
    ques[1] = 0xfe;
    ques[WIDTH*HEIGHT/8+2] = 0xfe;
    ques[WIDTH*HEIGHT/8+3] = 0x01;
    (*cbxPort) << serBt->name;
    (*cbxSPort) << serCh->name;
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
    labAddr->resize(80,25);
    txtAddr->resize(50,25);
    txtAddr->setValidator(new QRegExpValidator(QRegExp("[0-9A-Fa-f]{1,4}$")));
    txtAddr->setEnabled(false);
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
    connect(serBt,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
    connect(serCh,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
    serBt->setBaudRate(115200);
    serBt->setFlowControl(QSerialPort::NoFlowControl);
    serBt->setParity(QSerialPort::NoParity);
    serBt->setStopBits(QSerialPort::OneStop);

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

void WgtVideo::msleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

void WgtVideo::sltRefresh()
{
    serBt->refresh();
    serCh->refresh();
    cbxPort->clear();
    cbxSPort->clear();
    (*cbxPort) << serBt->name;
    (*cbxSPort) << serCh->name;
    qDebug() << "refreshed\n";
}

void WgtVideo::sltToggle()
{
    QPushButton *act = qobject_cast<QPushButton *>(sender());
    if(act == btnToggle)
    {
        if(btnToggle->text() == "关闭")
        {
            fpsTimer->stop();
            //thread->ena = false;
            cbxPort->setEnabled(true);
            btnRefresh->setEnabled(true);
            btnToggle->setText("打开");
            serBt->close();
            if(rbtDirR->isChecked())
                opened = false;
        }
        else
        {
            serBt->setPortName(cbxPort->currentText());
            if(!serBt->open(QIODevice::ReadWrite))
            {
                QMessageBox::critical(this,"错误","打开失败！",QMessageBox::Ok);
                return;
            }
            status = 0;
            thread->ena = true;
            //thread->start();
            serBt->setBaudRate(115200);
            serBt->setDataBits(QSerialPort::Data8);
            serBt->setFlowControl(QSerialPort::NoFlowControl);
            serBt->setParity(QSerialPort::NoParity);
            serBt->setStopBits(QSerialPort::OneStop);
            cbxPort->setEnabled(false);
            btnRefresh->setEnabled(false);
            btnToggle->setText("关闭");
            fpsTimer->start();
            if(rbtDirR->isChecked())
                opened = true;
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
            btnSToggle->setText("打开");
            serCh->close();
            if(rbtDirS->isChecked())
                opened = false;
        }
        else
        {
            serCh->setPortName(cbxSPort->currentText());
            if(!serCh->open(QIODevice::ReadWrite))
            {
                QMessageBox::critical(this,"错误","打开失败！",QMessageBox::Ok);
                return;
            }
            bool intok(false);
            if(txtAddr->text() == "")
            {
                QMessageBox::critical(this,"错误","地址不能为空！",QMessageBox::Ok);
                serCh->close();
                return;
            }
            int id = txtAddr->text().toInt(&intok,16);
            if(id == 0xffff)
            {
                QMessageBox::critical(this,"错误","地址不能为0xFFFF！",QMessageBox::Ok);
                serCh->close();
                return;
            }
            addr[0] = id>>8;
            addr[1] = id&0xff;
            cbxSPort->setEnabled(false);
            btnRefresh->setEnabled(false);
            rbtDirR->setEnabled(false);
            rbtDirS->setEnabled(false);
            rbtDatVideo->setEnabled(false);
            rbtDatVoice->setEnabled(false);
            serCh->setBaudRate(115200);
            serCh->setDataBits(QSerialPort::Data8);
            serCh->setFlowControl(QSerialPort::NoFlowControl);
            serCh->setParity(QSerialPort::NoParity);
            serCh->setStopBits(QSerialPort::OneStop);
            btnSToggle->setText("关闭");
            if(rbtDirS->isChecked())
                opened = true;
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
            else if(ch == 0x33 && queRecv[1]==0x33 && queRecv[2]==0x17)
            {
                queRecv.dequeue();
                queRecv.dequeue();
                continue;
            }
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
                if(!send_busy)
                    emit readOK();
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
                if(!send_busy)
                    emit readOK();
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
                    ques[index/8+2] = prev_status;
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
                ques[index/8+2] = ch;
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
    }
}

void WgtVideo::sltReadBuf()
{
    if(rbtDatVideo->isChecked() && rbtDirS->isChecked())
    {
        QByteArray src = serBt->readAll();
        for(auto i:src)
            queRecv.enqueue(i);
        if(opened)
        {
            //for(auto i:src)
            //    queSend.enqueue(i);
            //emit readOK();
        }
        sltRecv();
    }
    else if(rbtDatVideo->isChecked() && rbtDirR->isChecked())
    {
        QByteArray src = serCh->readAll();
        for(auto i:src)
            queRecv.enqueue(i);
        sltRecv();
    }
    else if(rbtDatVoice->isChecked() && rbtDirS->isChecked())
    {
        QByteArray src = serBt->readAll();
        /*QString txt(txtData->toPlainText());
        for(auto i:src)
        {
            txt.append(HEX(i/16));
            txt.append(HEX(i%16));
            txt.append(32);
        }*/
        if(opened)
        {
            //for(auto i:src)
            //    queSend.enqueue(i);
            //emit readOK();
        }
        //txtData->setText(txt);
    }
    else
    {
        QByteArray src = serCh->readAll();
        //QString txt(txtData->toPlainText());
        //for(auto i:src)
        //{
        //    txt.append(HEX(i/16));
        //    txt.append(HEX(i%16));
        //    txt.append(32);
        //}
        if(opened)
        {
            //for(auto i:src)
            //    queSend.enqueue(i);
            //emit readOK();
        }
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
            txtAddr->setEnabled(false);
        }
        else
        {
            btnToggle->setEnabled(true);
            txtAddr->setEnabled(true);
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
    Serial *serTemp(rbtDirR->isChecked()?serBt:serCh);
    QByteArray arr;
    int time=WIDTH*HEIGHT/8/200;
    int count = 0;
    for(int i=0;i<time;i++)
    {
        arr.push_front(0x17);
        arr.push_front(addr[1]);
        arr.push_front(addr[0]);
        for(int j=0;j<200;j++)
            arr.append(ques[count++]);
        serTemp->write(arr);
        arr.clear();
    }
    arr.append(addr[0]);
    arr.append(addr[1]);
    arr.append(0x17);
    while(count != WIDTH*HEIGHT/8+4)
    {
        arr.append(ques[count++]);
    }
    qDebug() << "arr: " << arr.size();
    serTemp->write(arr);
    send_busy = false;
}

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
    labAddr->move(245,siz.height()-100);
    txtAddr->move(285,siz.height()-100);
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
