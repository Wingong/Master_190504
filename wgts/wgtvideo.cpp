#include <QSize>
#include <QPalette>
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
      dw(1),dh(1),tx(0),cnt(0),cntv(0),index(0),status(0),recv_count(0),opened(false),automode(false),dir(Recv),
      mat(WIDTH, QVector<bool>(HEIGHT,false)),

      //frames and groups
      scrollArea(new QScrollArea(this)),
      wgtScroll(new WgtScroll(this)),
      fraCom(new QGroupBox("通信配置",this)),
      fraDiag(new QGroupBox("网络",this)),
      fraRight(new QGroupBox(this)),
      btgDir(new QButtonGroup(this)),
      btgDat(new QButtonGroup(this)),

      //frameOld objects
      labPort(new QLabel("串口",fraCom)),
      labSPort(new QLabel("链路串口",fraCom)),
      labAddr(new QLabel("地址",fraCom)),
      txtAddr(new QLineEdit("1234",fraCom)),
      btnRefresh(new QPushButton("刷新",fraCom)),
      btnToggle(new QPushButton("打开",fraCom)),
      btnSToggle(new QPushButton("打开",fraCom)),
      cbxPort(new MyComboBox(0,fraCom)),
      cbxSPort(new MyComboBox(1,fraCom)),
      rbtDirR(new QRadioButton("接收",fraCom)),
      rbtDirS(new QRadioButton("发送",fraCom)),
      rbtDatVideo(new QRadioButton("视频",fraCom)),
      rbtDatVoice(new QRadioButton("语音",fraCom)),

      //frameServer objects
      labIP(new QLabel("IP：",fraDiag)),
      labTcpPort(new QLabel("端口：",fraDiag)),
      txtIP(new QLineEdit(fraDiag)),
      txtTcpPort(new QLineEdit(fraDiag)),
      btnListen(new QPushButton("监听",fraDiag)),

      //right objects
      labFPS(new QLabel("帧率：",fraRight)),
      labInfo(new QLabel("输出信息：",fraRight)),
      txtFPS(new QLineEdit(fraRight)),
      txtInfo(new QTextEdit(fraRight)),
      btnClear(new QPushButton("清除",fraRight)),
      chbAuto(new QCheckBox("自动模式",fraRight)),

      //other objects
      labPaint(new QLabel(this)),
      txtData(new QTextEdit(this)),

      //non-gui objects
      voiceTimer(new QTimer(this)),
      fpsTimer(new QTimer(this)),
      thDisp(new ThdImageDisp(this)),
      thread(new ThdImageSend(ques,addr,arr,this)),
      serBt(new Serial),
      serCh(new Serial),
      server(new QTcpServer(this)),
      client(nullptr)
{
    ques[0] = 0x01;
    ques[1] = 0xfe;
    ques[WIDTH*HEIGHT/8+2] = 0xfe;
    ques[WIDTH*HEIGHT/8+3] = 0x01;
    (*cbxPort) << serBt->name;
    (*cbxSPort) << serCh->name;

    scrollArea->setWidget(wgtScroll);
    QPalette pal(wgtScroll->palette());         //设置色盘
    pal.setColor(QPalette::Background,0xffffff);
    wgtScroll->setAutoFillBackground(true);     //上底色
    wgtScroll->setPalette(pal);
    pal = scrollArea->palette();         //设置色盘
    pal.setColor(QPalette::Background,0xffffff);
    scrollArea->setAutoFillBackground(true);     //上底色
    scrollArea->setPalette(pal);
    fraCom->resize(330,122);
    fraDiag->resize(180,122);
    //fraCom->setsetFrameStyle(QFrame::Box | QFrame::Sunken);
    //fraDiag->setFrameStyle(QFrame::Box | QFrame::Sunken);
    //fraRight->setFrameStyle(QFrame::Box | QFrame::Sunken);
    btgDir->addButton(rbtDirR);
    btgDir->addButton(rbtDirS);
    btgDat->addButton(rbtDatVideo);
    btgDat->addButton(rbtDatVoice);

    labPort->setGeometry(5,55,80,25);
    labSPort->setGeometry(5,90,80,25);
    labAddr->setGeometry(230,20,80,25);
    txtAddr->setGeometry(268,20,50,25);
    txtAddr->setValidator(new QRegExpValidator(QRegExp("[0-9A-Fa-f]{1,4}$")));
    txtAddr->setEnabled(false);
    btnRefresh->setGeometry(5,20,80,25);
    btnToggle->setGeometry(175,55,80,25);
    btnToggle->setEnabled(false);
    btnSToggle->setGeometry(175,90,80,25);
    cbxPort->setGeometry(75,55,90,25);
    cbxSPort->setGeometry(75,90,90,25);
    rbtDirR->setGeometry(95,20,80,25);
    rbtDirR->setChecked(true);
    rbtDirS->setGeometry(160,20,80,25);
    rbtDatVideo->setGeometry(265,55,80,25);
    rbtDatVideo->setChecked(true);
    rbtDatVoice->setGeometry(265,90,80,25);
    cbxPort->setEnabled(false);
    btnToggle->setEnabled(false);

    labIP->setGeometry(5,20,80,25);
    labTcpPort->setGeometry(5,55,80,25);
    txtIP->setGeometry(45,20,125,25);
    txtIP->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$")));
    txtTcpPort->setGeometry(45,55,125,25);
    txtTcpPort->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,5}$")));
    btnListen->setGeometry(5,90,80,25);

    labFPS->setGeometry(10,10,80,25);
    labInfo->setGeometry(10,50,80,25);
    txtInfo->move(10,80);
    txtFPS->setGeometry(60,10,80,25);
    txtFPS->setText("0");
    txtFPS->setReadOnly(true);
    //txtInfo->setReadOnly(true);
    btnClear->resize(80,25);
    chbAuto->resize(100,25);

    labPaint->move(20,20);
    labPaint->installEventFilter(this);
    txtData->move(20,20);
    txtData->setEnabled(false);
    txtData->setVisible(false);

    voiceTimer->setInterval(15);
    fpsTimer->setInterval(500);
    serRecv=serCh;
    serSend=serBt;
    qDebug() << "IMG Read: " << thDisp->image.load("a.jpg","JPG");
    thDisp->labPaint = this->labPaint;

    connect(btnRefresh,SIGNAL(clicked(bool)),this,SLOT(sltRefresh()));
    connect(btnToggle,SIGNAL(clicked(bool)),this,SLOT(sltToggle()));
    //connect(btnClear,SIGNAL(clicked(bool)),this,SLOT(sltClear()));
    connect(btnClear,&QPushButton::clicked,[=](){
        wgtScroll->newRecord(User::SEND,QString(txtInfo->toPlainText()));
    });
    connect(btnSToggle,SIGNAL(clicked(bool)),this,SLOT(sltToggle()));
    connect(btgDir,SIGNAL(buttonToggled(int,bool)),this,SLOT(sltDirTog(int,bool)));
    connect(btgDat,SIGNAL(buttonToggled(int,bool)),this,SLOT(sltDatTog(int,bool)));
    connect(chbAuto,SIGNAL(toggled(bool)),this,SLOT(sltAutoTog(bool)));
    connect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
    connect(serBt,SIGNAL(bytesWritten(qint64)),this,SLOT(sltWriteOver(qint64)));
    connect(serCh,SIGNAL(bytesWritten(qint64)),this,SLOT(sltWriteOver(qint64)));

    //connect(thread,&ThdImageSend::readOK,this,&WgtVideo::repaint);
#ifdef DEBUG
    connect(fpsTimer,SIGNAL(timeout()),this,SLOT(fps()));

#endif
    connect(btnListen,SIGNAL(clicked(bool)),this,SLOT(sltLisTog()));
    connect(server,SIGNAL(newConnection()),this,SLOT(sltConnected()));
}

WgtVideo::~WgtVideo()
{
    thread->ena = false;
    thread->exit();
    fpsTimer->stop();
    voiceTimer->stop();
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
    if(!serBt->isOpen())
    {
        serBt->refresh();
        QString str(cbxPort->currentText());
        cbxPort->clear();
        (*cbxPort) << serBt->name;
        cbxPort->setCurrentText(str);
    }
    if(!serCh->isOpen())
    {
        serCh->refresh();
        QString str(cbxSPort->currentText());
        cbxSPort->clear();
        (*cbxSPort) << serCh->name;
        cbxSPort->setCurrentText(str);
    }
    qDebug() << "port refreshed\n";
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
            //cbxPort->setEnabled(true);
            btnRefresh->setEnabled(true);
            btnToggle->setText("打开");
            serBt->close();
            if(rbtDirR->isChecked())
                opened = false;
            if(btnSToggle->text()=="打开" && (rbtDatVoice->isChecked() || rbtDirS->isChecked()))
                txtAddr->setEnabled(true);
            if(serSend == serBt)
            {
                //voiceTimer->stop();
                //disconnect(voiceTimer,SIGNAL(timeout()),this,SLOT(sltVoice()));
            }
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
            serBt->setBaudRate(baudBt);
            serBt->setDataBits(QSerialPort::Data8);
            serBt->setFlowControl(QSerialPort::NoFlowControl);
            serBt->setParity(QSerialPort::NoParity);
            serBt->setStopBits(QSerialPort::OneStop);
            cbxPort->setEnabled(false);
            btnRefresh->setEnabled(false);
            btnToggle->setText("关闭");
            fpsTimer->start();
            if(serSend == serBt)
            {
                //voiceTimer->start();
                //connect(voiceTimer,SIGNAL(timeout()),this,SLOT(sltVoice()));
            }
            //if(rbtDirR->isChecked())
            //    opened = true;
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
            //if(rbtDirS->isChecked())
            //    opened = false;
            txtAddr->setEnabled(false);
            if(btnToggle->text()=="打开" && (rbtDatVoice->isChecked() || rbtDirS->isChecked()))
                txtAddr->setEnabled(true);
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
            txtAddr->setEnabled(false);
            cbxSPort->setEnabled(false);
            btnRefresh->setEnabled(false);
            rbtDirR->setEnabled(false);
            rbtDirS->setEnabled(false);
            rbtDatVideo->setEnabled(false);
            rbtDatVoice->setEnabled(false);
            serCh->setBaudRate(baudCh);
            serCh->setDataBits(QSerialPort::Data8);
            serCh->setFlowControl(QSerialPort::NoFlowControl);
            serCh->setParity(QSerialPort::NoParity);
            serCh->setStopBits(QSerialPort::OneStop);
            txtInfo->append(QString::number(serCh->baudRate()));
            btnSToggle->setText("关闭");
            //if(rbtDirS->isChecked())
            //    opened = true;
        }
    }
}

void WgtVideo::sltClear()
{
    index = 0;
    txtInfo->setText("");
    for(int i=0;i<WIDTH;i++)
    {
        for(int j=0;j<HEIGHT;j++)
            mat[i][j] = 0;
    }
    repaint();
    recv_count = 0;
    status = 0;
}

void WgtVideo::funRecv()
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
                repaint();
                if(!thread->send_busy && serSend->isOpen())
                    thread->start();
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
                repaint();
                if(!thread->send_busy && serSend->isOpen())
                    thread->start();
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
    if(rbtDatVideo->isChecked())
    {
        QByteArray array(serCh->readAll());
        arr.append(array);
        if(array.right(2) == jpgend)
        {
            thDisp->resize = false;
            thDisp->arr = this->arr;
            thDisp->start();
            arr.clear();
        }
        //for(auto i:qba)
        //    queRecv.enqueue(i);
        //thread->start();
    }
    else if(rbtDatVoice->isChecked())
    {
        QByteArray qba = serRecv->readAll();

        //if(arr.size() == 300 && server->isListening())
            client->write(qba);
        txtInfo->append(QString::number(qba.size()));
        qDebug() << "Time: " << QTime::currentTime().toString("hh:mm:ss.zzz");
    }
}

void WgtVideo::sltAutoRead()
{
    QByteArray read_array(serRecv->readAll());
    switch(auto_dat)
    {
    case 0:
        if(read_array.size() == 3 && read_array[0] == (char)0xFF && read_array[1] == (char)0xCC)
        {
            if(read_array[2] == (char)1)
            {
                rbtDatVideo->setChecked(true);
                auto_dat = 1;
            }
            else if(read_array[2] == (char)2)
            {
                rbtDatVoice->setChecked(true);
                auto_dat = 2;
                qDebug() << "Auto mode 2";
            }
            else
                qDebug() << "Auto mode Error!";
        }
        break;
    case 1:
        if(read_array[0] == (char)0x01 && read_array[1] == (char)0xfe)
        {
            auto_dat = 3;
            index = 0;
            for(int i=2;i<read_array.size();i++)
            {}
        }
        else
            txtInfo->append("Error: wrong data");
        break;

    case 2:
        if(auto_dir == true)
        {
            read_array.push_front(0x17);
            read_array.push_front(addr[1]);
            read_array.push_front(addr[0]);
        }
        if(serSend->isOpen())
            serSend->write(read_array);
        auto_dat = 0;
        break;
    case 3:
        break;

    }
}

void WgtVideo::sltWriteOver(qint64 bytes)
{
    txtInfo->append(QString::number(bytes));
    qDebug() << "Write time: " << QTime::currentTime().toString("hh:mm:ss.zzz");
}

void WgtVideo::sltDirTog(int index,bool b)
{
    if(b)
    {
        if(index == -2)
        {
            dir = Recv;
            if(rbtDatVideo->isChecked())
            {
                btnToggle->setEnabled(false);
            }
            else
            {
                //btnToggle->setEnabled(true);
            }
            txtAddr->setEnabled(false);
            disconnect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
            serRecv = serCh;
            serSend = serBt;
            connect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
        }
        else
        {
            dir = Send;
            //btnToggle->setEnabled(true);
            txtAddr->setEnabled(true);
            disconnect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
            serRecv = serBt;
            serSend = serCh;
            connect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
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
                //btnToggle->setEnabled(true);
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
            //btnToggle->setEnabled(true);
        }
    }
}

void WgtVideo::sltAutoTog(bool b)
{
    automode = b;
    auto_dir = rbtDirS->isChecked();
    auto_dat = 0;
    rbtDirR->setEnabled(!automode);
    rbtDirS->setEnabled(!automode);
    rbtDatVideo->setEnabled(!automode);
    rbtDatVoice->setEnabled(!automode);
    if(automode)
    {
        disconnect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
        connect(serRecv,SIGNAL(readyRead()),this,SLOT(sltAutoRead()));
    }
    else
    {
        disconnect(serRecv,SIGNAL(readyRead()),this,SLOT(sltAutoRead()));
        connect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
    }
}

void WgtVideo::sltVoice()
{
    txtData->append(QString::number(arr.size()));
    if(arr.size() == 150)
        serSend->write(arr);
}

void WgtVideo::sltLisTog()
{
    if(server->isListening())
    {
        if(client != nullptr)
        {
            disconnect(client,SIGNAL(readyRead()),this,SLOT(sltTcpRecv()));
            disconnect(client,SIGNAL(disconnected()),this,SLOT(sltDisconnected()));
            client->close();
            delete client;
            client = nullptr;
        }
        server->close();
        btnListen->setText("监听");
        txtIP->setEnabled(true);
        txtTcpPort->setEnabled(true);
    }
    else
    {
        if(!server->listen(QHostAddress(txtIP->text()),txtTcpPort->text().toInt()))
        {
            QMessageBox::critical(this,"错误",server->errorString(),QMessageBox::Ok);
            return;
        }
        btnListen->setText("停止监听");
        txtIP->setEnabled(false);
        txtTcpPort->setEnabled(false);
        connect(serSend,SIGNAL(bytesWritten(qint64)),this,SLOT(sltWriteOver(qint64)));
    }
}

void WgtVideo::sltConnected(void)
{
    client = server->nextPendingConnection();
    connect(client,SIGNAL(readyRead()),this,SLOT(sltTcpRecv()));
    connect(client,SIGNAL(disconnected()),this,SLOT(sltDisconnected()));
    txtInfo->append("[New connection]");
    txtInfo->append(QString("Time: ").append(QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss")));
    txtInfo->append(QString("IP: %1").arg(client->peerAddress().toString()));
    txtInfo->append(QString("Port: %1\n").arg(client->peerPort()));
}

void WgtVideo::sltDisconnected()
{
    txtInfo->append("Client offline.\n");
    disconnect(client,SIGNAL(readyRead()),this,SLOT(sltTcpRecv()));
    disconnect(client,SIGNAL(disconnected()),this,SLOT(sltDisconnected()));
    client->close();
    delete client;
    client = nullptr;
}

void WgtVideo::sltTcpRecv()
{
    QByteArray array(client->readAll());
    arr.append(array);
    if(array.right(2) == jpgend)
    {
        thDisp->resize = false;
        thDisp->arr = this->arr;
        thDisp->start();
        arr.clear();
    }
    //if(rbtDatVideo->isChecked() && array.size() == 1200)
    //{
    //    //queRecv.append(array);
    //    funRecv();
    //}
    if(serSend->isOpen())
        serSend->write(array);
}

void WgtVideo::sltSend()
{
    if(serSend->isOpen())
        serSend->write(arr);
    arr.clear();
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
    int width(event->size().width());
    int height(event->size().height());
    qDebug() << "Video: " << event->size();
    fraCom->move(20,height-132);
    fraDiag->move(360,height-132);
    fraRight->setGeometry(width-243,20,233,height-30);

    txtInfo->resize(213,height-152);
    btnClear->move(10,height-60);
    chbAuto->move(110,height-60);

    labPaint->resize(width-273,height-162);
    labPaint->setVisible(false);
    qDebug() << "Paint label size: " << labPaint->size();
    txtData->resize(width-273,height-162);

    scrollArea->setGeometry(20,10,width-273,height-250);
    wgtScroll->resize(scrollArea->width()-25,wgtScroll->height());
    qDebug() << scrollArea->verticalScrollBarPolicy();
    qDebug() << scrollArea->horizontalScrollBarPolicy();
    qDebug() << scrollArea->sizeAdjustPolicy();
    qDebug() << scrollArea->widgetResizable();
    qDebug() << scrollArea->alignment();
    thDisp->resize = true;
    thDisp->start();
    genRects();
}

#ifdef DEBUG
void WgtVideo::fps()
{
    txtFPS->setText(QString::number(cntv));
    cntv = 0;
}
#endif
