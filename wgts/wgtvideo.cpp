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
      dw(1),dh(1),tx(0),cnt(0),cntv(0),index(0),status(0),recv_count(0),opened(false),automode(false),
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
      chbAuto(new QCheckBox("自动模式",this)),
      btnSToggle(new QPushButton("打开",this)),
      fraOld(new QFrame(this)),
      fraServer(new QFrame(this)),
      labIP(new QLabel("IP：",this)),
      labTcpPort(new QLabel("端口：",this)),
      txtIP(new QLineEdit(this)),
      txtTcpPort(new QLineEdit(this)),
      btnListen(new QPushButton("监听",this)),
      serBt(new Serial),
      serCh(new Serial),
      voiceTimer(new QTimer(this)),
      fpsTimer(new QTimer(this)),
      server(new QTcpServer(this)),
      client(nullptr),
      mat(WIDTH, QVector<bool>(HEIGHT,false)),
      thread(new ThdImageSend(ques,addr,arr,this))
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
    labIP->resize(80,25);
    labTcpPort->resize(80,25);
    txtIP->resize(125,25);
    txtIP->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$")));
    txtTcpPort->resize(125,25);
    txtTcpPort->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,5}$")));
    btnListen->resize(80,25);
    btgDir->addButton(rbtDirR);
    btgDir->addButton(rbtDirS);
    btgDat->addButton(rbtDatVideo);
    btgDat->addButton(rbtDatVoice);
    rbtDirR->setChecked(true);
    rbtDatVideo->setChecked(true);
    chbAuto->resize(100,25);
    btnSToggle->resize(80,25);

    fraOld->resize(330,107);
    fraServer->resize(180,107);
    fraOld->setFrameStyle(QFrame::Box | QFrame::Sunken);
    fraServer->setFrameStyle(QFrame::Box | QFrame::Sunken);


    serRecv=serCh;
    serSend=serBt;
    connect(btnRefresh,SIGNAL(clicked(bool)),this,SLOT(sltRefresh()));
    connect(btnToggle,SIGNAL(clicked(bool)),this,SLOT(sltToggle()));
    connect(btnClear,SIGNAL(clicked(bool)),this,SLOT(sltClear()));
    connect(btnSToggle,SIGNAL(clicked(bool)),this,SLOT(sltToggle()));
    connect(btgDir,SIGNAL(buttonToggled(int,bool)),this,SLOT(sltDirTog(int,bool)));
    connect(btgDat,SIGNAL(buttonToggled(int,bool)),this,SLOT(sltDatTog(int,bool)));
    connect(chbAuto,SIGNAL(toggled(bool)),this,SLOT(sltAutoTog(bool)));
    connect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
    serBt->setBaudRate(BTBAUD);
    serBt->setFlowControl(QSerialPort::NoFlowControl);
    serBt->setParity(QSerialPort::NoParity);
    serBt->setStopBits(QSerialPort::OneStop);

    connect(thread,SIGNAL(readOK()),this,SLOT(sltSend()));
    voiceTimer->setInterval(15);
#ifdef DEBUG
    connect(fpsTimer,SIGNAL(timeout()),this,SLOT(fps()));
    fpsTimer->setInterval(500);

#endif
    connect(btnListen,SIGNAL(clicked(bool)),this,SLOT(sltLisTog(bool)));
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
            serBt->setBaudRate(BTBAUD);
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
            serCh->setBaudRate(CHBAUD);
            serCh->setDataBits(QSerialPort::Data8);
            serCh->setFlowControl(QSerialPort::NoFlowControl);
            serCh->setParity(QSerialPort::NoParity);
            serCh->setStopBits(QSerialPort::OneStop);
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
        QByteArray src = serRecv->readAll();
        for(auto i:src)
            queRecv.enqueue(i);
        funRecv();
    }
    else if(rbtDatVoice->isChecked())
    {
        QByteArray qba = serRecv->readAll();
        int i=0;
        if(qba.size() == 30 && qba[0]==(char)0x59)
        {
            arr = qba;
            serSend->write(arr);
            cntv ++;
            i=1;
        }
        else if(qba.size()+tempArr.size() == 30)
        {
            arr = tempArr.append(qba);
            tempArr.clear();
            serSend->write(arr);
            cntv ++;
            i=2;
        }
        else if(qba.size()+tempArr.size() > 30)
        {
            tempArr.clear();
            i=3;
        }
        else
        {
            i=4;
            tempArr.append(qba);
        }
        QString str;
        for(u8 i:qba)
        {
            str.append(HEX(i/16));
            str.append(HEX(i%16));
            str.append(' ');
        }
        txtData->append(str);
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
                qDebug() << "Error!";
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
            disconnect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
            serRecv = serCh;
            serSend = serBt;
            connect(serRecv,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
        }
        else
        {
            btnToggle->setEnabled(true);
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
    if(arr.size() == 30)
        serSend->write(arr);
}

void WgtVideo::sltLisTog(bool b)
{
    if(server->isListening())
    {
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
    disconnect(client,SIGNAL(readyRead()),this,SLOT(sltTcpRecv()));
    disconnect(client,SIGNAL(disconnected()),this,SLOT(sltDisconnected()));
}

void WgtVideo::sltTcpRecv()
{
    txtInfo->append(client->readAll());
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
    QSize siz(event->size());
    labPort->move(25,siz.height()-70);
    labPaint->resize(siz.width()-273,siz.height()-140);
    labFPS->move(siz.width()-233,20);
    txtFPS->move(siz.width()-183,20);
    labInfo->move(siz.width()-233,60);
    txtInfo->setGeometry(siz.width()-233,92,213,siz.height()-152);
    txtData->resize(siz.width()-273,siz.height()-127);
    cbxPort->move(95,siz.height()-70);
    btnRefresh->move(25,siz.height()-105);
    btnToggle->move(195,siz.height()-70);
    btnClear->move(siz.width()-100,siz.height()-40);
    chbAuto->move(siz.width()-200,siz.height()-40);
    labSPort->move(25,siz.height()-35);
    labAddr->move(250,siz.height()-105);
    txtAddr->move(290,siz.height()-105);
    cbxSPort->move(95,siz.height()-35);
    btnSToggle->move(195,siz.height()-35);
    fraOld->move(20,siz.height()-110);
    fraServer->move(360,siz.height()-110);
    labIP->move(365,siz.height()-105);
    labTcpPort->move(365,siz.height()-70);
    txtIP->move(405,siz.height()-105);
    txtTcpPort->move(405,siz.height()-70);
    btnListen->move(365,siz.height()-35);
    rbtDirR->move(115,siz.height()-100);
    rbtDirS->move(180,siz.height()-100);
    rbtDatVideo->move(285,siz.height()-70);
    rbtDatVoice->move(285,siz.height()-35);
    genRects();
}

#ifdef DEBUG
void WgtVideo::fps()
{
    txtFPS->setText(QString::number(cntv*2));
    if(cntv < 10)
        arr.clear();
    cntv = 0;
}
#endif
