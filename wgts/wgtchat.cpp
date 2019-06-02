#include <QSize>
#include <QResizeEvent>
#include <QString>
#include <QByteArray>
#include <QMessageBox>
#include <QDebug>
#include <QRegExp>
#include "wgtchat.h"

WgtChat::WgtChat(QWidget *parent)
    : QWidget(parent),
      hist(new QTextEdit(this)),
      edit(new QTextEdit(this)),
      labPort(new QLabel("串口",this)),
      cbxPort(new MyComboBox(0,this)),
      btnRefresh(new QPushButton("刷新",this)),
      btnToggle(new QPushButton("打开",this)),
      btnSend(new QPushButton("发送",this)),
      btnClr(new QPushButton("清除",this)),
      labAddr(new QLabel("地址",this)),
      txtAddr(new QLineEdit("1234",this)),
      //serRecv(new Serial),
      serSend(new Serial),
      th1(new ThdChatRecv(this)),
      th2(new ThdChatSend(this))
{
    (*cbxPort) << serSend->name;
                //
    th1->queRecv = th2->queRecv = &queRecv;
    th1->queSend = th2->queSend = &queSend;
    hist->setReadOnly(true);
    btnSend->setEnabled(false);
    labPort->resize(80,25);
    cbxPort->resize(60,25);
    btnRefresh->resize(80,25);
    btnSend->resize(80,25);
    btnToggle->resize(80,25);
    btnClr->resize(80,25);
    labAddr->resize(80,25);
    txtAddr->resize(50,25);
    txtAddr->setValidator(new QRegExpValidator(QRegExp("[0-9A-Fa-f]{1,4}$")));
    connect(btnRefresh,SIGNAL(clicked(bool)),this,SLOT(sltRefresh()));
    connect(btnToggle,SIGNAL(clicked(bool)),this,SLOT(sltToggle()));
    connect(btnSend,SIGNAL(clicked(bool)),this,SLOT(sltSend()));
    connect(serSend,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
    connect(th2,SIGNAL(ok(int)),this,SLOT(sltS(int)));
    connect(th1,SIGNAL(recv()),this,SLOT(sltRecv()));
    connect(th1,SIGNAL(oper(int,bool)),th2,SLOT(sltOper(int,bool)));
    connect(btnClr,SIGNAL(clicked(bool)),hist,SLOT(clear()));
    connect(btnClr,SIGNAL(clicked(bool)),edit,SLOT(clear()));
    serSend->setBaudRate(230400);
    serSend->setDataBits(QSerialPort::Data8);
    serSend->setFlowControl(QSerialPort::NoFlowControl);
    serSend->setParity(QSerialPort::NoParity);
    serSend->setStopBits(QSerialPort::OneStop);

}

void WgtChat::sltRefresh(void)
{
    if(!serSend->isOpen())
    {
        serSend->refresh();
        QString str(cbxPort->currentText());
        cbxPort->clear();
        (*cbxPort) << serSend->name;
        cbxPort->setCurrentText(str);
    }
    qDebug() << "refreshed\n";
}

void WgtChat::sltRecv(void)
{
    hist->setText(th1->txtRecv);
}

void WgtChat::sltToggle()
{
    if(btnSend->isEnabled())
    {
        th1->ena = false;
        th2->ena = false;
        cbxPort->setEnabled(true);
        btnRefresh->setEnabled(true);
        btnSend->setEnabled(false);
        btnToggle->setText("打开");
        serSend->close();
        txtAddr->setEnabled(true);
    }
    else
    {
        serSend->setPortName(cbxPort->currentText());
        if(!serSend->open(QIODevice::ReadWrite))
        {
            QMessageBox::critical(this,"错误","打开失败！",QMessageBox::Ok);
            return;
        }
        if(txtAddr->text() == "")
        {
            QMessageBox::critical(this,"错误","地址不能为空！",QMessageBox::Ok);
            serSend->close();
            return;
        }
        bool intok=false;
        int id = txtAddr->text().toInt(&intok,16);
        if(id == 0xffff)
        {
            QMessageBox::critical(this,"错误","地址不能为0xFFFF！",QMessageBox::Ok);
            serSend->close();
            return;
        }
        addr[0] = id>>8;
        addr[1] = id&0xff;
        th1->start();
        th2->start();
        serSend->setBaudRate(230400);
        serSend->setDataBits(QSerialPort::Data8);
        serSend->setFlowControl(QSerialPort::NoFlowControl);
        serSend->setParity(QSerialPort::NoParity);
        serSend->setStopBits(QSerialPort::OneStop);
        cbxPort->setEditable(false);
        btnRefresh->setEnabled(false);
        btnSend->setEnabled(true);
        btnToggle->setText("关闭");
        txtAddr->setEnabled(false);
    }
}

void WgtChat::sltSend()
{
   // hist->append("");
   // QString str;
   // QDateTime time(QDateTime::currentDateTime());
   // str += QString::number(time.time().hour());
   // str += ':';
   // str += QString::number(time.time().minute());
   // str += ':';
   // str += QString::number(time.time().second());
   // hist->append(str);
   // hist->append(edit->toPlainText());
   // edit->clear();
    for(auto i:edit->toPlainText())
    {
        queSend.enqueue(i.toLatin1());
    }

}

void WgtChat::sltS(int id)
{
    QByteArray array;
    for(int i=0;i<th2->querea[id].size();i++)
    {
        u8 ch = th2->querea[id][i];
        array.append(ch);
        if(i%32 == 1)
        {
            array.push_front(0x17);
            array.push_front(addr[1]);
            array.push_front(addr[0]);
            serSend->write(array);
            array.clear();
        }
    }
    array.push_front(0x17);
    array.push_front(addr[1]);
    array.push_front(addr[0]);
    serSend->write(array);
    th2->cmd = 0;
    //for(int i=1;i!=0;i++);
}

void WgtChat::sltReadBuf()
{
    QByteArray src = serSend->readAll();
    for(auto i:src)
        queRecv.enqueue(i);
}


void WgtChat::resizeEvent(QResizeEvent *event)
{
    QSize siz(event->size());
    hist->setGeometry(10,10,siz.width()-20,siz.height()-180);
    edit->setGeometry(10,siz.height()-165,siz.width()-20,110);
    labPort->move(10,siz.height()-40);
    cbxPort->move(50,siz.height()-40);
    btnRefresh->move(siz.width()-360,siz.height()-40);
    btnToggle->move(siz.width()-270,siz.height()-40);
    btnSend->move(siz.width()-180,siz.height()-40);
    btnClr->move(siz.width()-90,siz.height()-40);
    labAddr->move(120,siz.height()-40);
    txtAddr->move(160,siz.height()-40);
}
