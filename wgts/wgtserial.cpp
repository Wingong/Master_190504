#include <QDebug>
#include <QChar>
#include <QByteArray>
#include <QString>
#include <QResizeEvent>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include "wgtserial.h"

WgtSerial::WgtSerial(QWidget *parent)
    : QWidget(parent),
      labPort(new QLabel("选择串口",this)),
      labBaud(new QLabel("波特率",this)),
      labStop(new QLabel("停止位",this)),
      labExam(new QLabel("校验位",this)),
      cbxPort(new MyComboBox(0, this)),
      cbxBaud(new MyComboBox(1, this)),
      cbxStop(new MyComboBox(2, this)),
      cbxExam(new MyComboBox(3, this)),
      labRecv(new QLabel("接收区",this)),
      labSend(new QLabel("发送区",this)),
      rbtRecHex(new QRadioButton("HEX",this)),
      rbtRecTxt(new QRadioButton("文本",this)),
      rbtSenHex(new QRadioButton("HEX",this)),
      rbtSenTxt(new QRadioButton("文本",this)),
      btgRecv(new QButtonGroup(this)),
      btgSend(new QButtonGroup(this)),
      btnRecv(new QPushButton("清除",this)),
      btnSend(new QPushButton("清除",this)),
      txtRecv(new QPlainTextEdit(this)),
      txtSend(new QPlainTextEdit(this)),
      btnToggle(new QPushButton("打开",this)),
      btnRefresh(new QPushButton("刷新",this)),
      btnSenMsg(new QPushButton("发送",this)),
      xTendSerial(new Serial)
{
    this->setGeometry(0,0,parent->geometry().width(),parent->geometry().height());

    //this->wgtTab->addTab(new QWidget(),"Tab 1");
    (*cbxBaud) << "1200" << "2400" << "4800" << "9600"
               << "19200" << "38400" << "57600" << "115200";
    (*cbxExam) << "无校验" << "奇校验" << "偶校验";
    (*cbxStop) << "1位" << "2位" << "1.5位";
    labPort->resize(80,25);
    cbxPort->resize(90,25);
    labBaud->resize(45,25);
    cbxBaud->resize(100,25);
    labStop->resize(45,25);
    cbxStop->resize(100,25);
    labExam->resize(45,25);
    cbxExam->resize(100,25);
    btnRecv->resize(60,25);
    btnSend->resize(60,25);
    (*cbxPort) << xTendSerial->name;

    txtRecv->setReadOnly(true);

    btnToggle->resize(76,30);
    btnRefresh->resize(76,30);

    btgRecv->addButton(rbtRecHex);
    btgRecv->addButton(rbtRecTxt);
    btgSend->addButton(rbtSenHex);
    btgSend->addButton(rbtSenTxt);
    rbtRecTxt->setChecked(true);
    rbtSenTxt->setChecked(true);
    btnSenMsg->setEnabled(false);

    connect(cbxPort,SIGNAL(activated(int)),this,SLOT(sltComboClick(int)));
    connect(cbxBaud,SIGNAL(activated(int)),this,SLOT(sltComboClick(int)));
    connect(cbxStop,SIGNAL(activated(int)),this,SLOT(sltComboClick(int)));
    connect(cbxExam,SIGNAL(activated(int)),this,SLOT(sltComboClick(int)));
    connect(btnRefresh,SIGNAL(clicked(bool)),this,SLOT(sltRefresh()));
    connect(btnToggle,SIGNAL(clicked(bool)),this,SLOT(sltOpen()));
    connect(btnRecv,SIGNAL(clicked(bool)),this,SLOT(sltRecClr()));
    connect(btnSend,SIGNAL(clicked(bool)),this,SLOT(sltSenClr()));
    connect(btnSenMsg,SIGNAL(clicked(bool)),this,SLOT(sltSenMsg()));
    connect(btgRecv,SIGNAL(buttonToggled(int,bool)),this,SLOT(sltRecTog(int,bool)));
    connect(btgSend,SIGNAL(buttonToggled(int,bool)),this,SLOT(sltSenTog(int,bool)));

    qDebug() << xTendSerial->baudRate() << '\t'
             << xTendSerial->dataBits() << '\t'
             << xTendSerial->stopBits() << '\t'
             << xTendSerial->flowControl();
}

WgtSerial::~WgtSerial()
{
    delete      labPort;
    delete      labBaud;
    delete      labStop;
    delete      labExam;
    delete      cbxPort;
    delete      cbxBaud;
    delete      cbxStop;
    delete      cbxExam;
    delete      btnToggle;
    delete      txtRecv;
    delete      txtSend;
    delete      btnRefresh;
    delete      xTendSerial;

    labPort     = nullptr;
    labBaud     = nullptr;
    labStop     = nullptr;
    labExam     = nullptr;
    cbxPort     = nullptr;
    cbxBaud     = nullptr;
    cbxStop     = nullptr;
    cbxExam     = nullptr;
    txtRecv     = nullptr;
    txtSend     = nullptr;
    btnToggle   = nullptr;
    btnRefresh  = nullptr;
    xTendSerial = nullptr;
}

void WgtSerial::sltComboClick(int cbx)
{
    MyComboBox *act = qobject_cast<MyComboBox *>(sender());
    switch(act->index)
    {
    case 0:
        qDebug() << "Port\t";
        break;
    case 1:
        qDebug() << "Baud\t";
        break;
    case 2:
        qDebug() << "Stop\t";
        break;
    case 3:
        qDebug() << "Exam\t";
        break;
    default:
        qDebug() << "Other\t";
    }
    qDebug() << act->currentText() << '\t'
             << act->currentIndex() << '\t'
             << cbx;
}

void WgtSerial::sltRefresh(void)
{
    if(!xTendSerial->isOpen())
    {
        xTendSerial->refresh();
        QString str(cbxPort->currentText());
        cbxPort->clear();
        (*cbxPort) << xTendSerial->name;
        cbxPort->setCurrentText(str);
    }
    qDebug() << "refreshed\n";
}

void WgtSerial::sltOpen(void)
{
    xTendSerial->setPortName(cbxPort->currentText());
    if(!xTendSerial->open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(this,"错误","打开失败！",QMessageBox::Ok);
        return;
    }

    xTendSerial->setBaudRateIndex(cbxBaud->currentIndex());
    xTendSerial->setDataBits(QSerialPort::Data8);
    xTendSerial->setFlowControl(QSerialPort::NoFlowControl);
    xTendSerial->setParity(QSerialPort::Parity(cbxExam->currentIndex()));
    xTendSerial->setStopBits(QSerialPort::StopBits(cbxStop->currentIndex()+1));
    qDebug() << xTendSerial->baudRate();
    qDebug() << xTendSerial->dataBits();
    qDebug() << xTendSerial->parity();
    qDebug() << xTendSerial->stopBits();

    btnRefresh->setEnabled(false);
    cbxPort->setEnabled(false);
    cbxBaud->setEnabled(false);
    cbxStop->setEnabled(false);
    cbxExam->setEnabled(false);
    btnSenMsg->setEnabled(true);
    btnToggle->setText("关闭");
    disconnect(btnToggle,SIGNAL(clicked(bool)),this,SLOT(sltOpen()));
    connect(btnToggle,SIGNAL(clicked(bool)),this,SLOT(sltClose()));
    connect(xTendSerial,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
    qDebug() << "opened\n";
}

void WgtSerial::sltClose(void)
{
    xTendSerial->close();
    btnRefresh->setEnabled(true);
    cbxPort->setEnabled(true);
    cbxBaud->setEnabled(true);
    cbxStop->setEnabled(true);
    cbxExam->setEnabled(true);
    btnToggle->setText("打开");
    btnSenMsg->setEnabled(false);
    disconnect(btnToggle,SIGNAL(clicked(bool)),this,SLOT(sltClose()));
    connect(btnToggle,SIGNAL(clicked(bool)),this,SLOT(sltOpen()));
    disconnect(xTendSerial,SIGNAL(readyRead()),this,SLOT(sltReadBuf()));
    qDebug() << "closed\n";
}

void WgtSerial::sltRecClr()
{
    txtRecv->setPlainText("");
}

void WgtSerial::sltSenClr()
{
    txtSend->setPlainText("");
}

void WgtSerial::sltSenMsg()
{
    QString src(txtSend->toPlainText());
    QByteArray qbr;
    if(rbtSenHex->isChecked())
    {
        int count(0);
        char c[2]={0,0};
        for(QChar i:src)
        {
            if((count >= 2 && i != ' ' )|| !(i.isDigit() || i.isLetter() || i==' '))
            {
                QMessageBox::critical(this,"错误","Hex格式错误！",QMessageBox::Ok);
                return;
            }
            else if(i == ' ' && count)
            {
                qbr += (char)(c[1]+c[0]*16);
                count = 0;
            }
            else if(i != ' ' && count < 2)
            {
                c[count++] = i.isDigit()?i.toLatin1()-48:i.toUpper().toLatin1()-55;
            }
        }
        if(count)
        {
            qbr += (char)(c[1]+c[0]*16);
        }
    }
    else
        qbr = src.toLatin1();
    xTendSerial->write(qbr);

}

void WgtSerial::sltRecTog(int index,bool b)
{
    if(b)
    {
        int count(0);
        QString src(txtRecv->toPlainText());
        QString str;
        qDebug() << index;
        if(index == -3)
        {
            char c[2]={0,0};
            for(QChar i:src)
            {
                if((count >= 2 && i != ' ' )|| !(i.isDigit() || i.isLetter() || i==' '))
                {
                    QMessageBox::critical(this,"错误","Hex格式错误！",QMessageBox::Ok);
                    rbtRecHex->setChecked(true);
                    txtRecv->setPlainText(src);
                    return;
                }
                else if(i == ' ' && count)
                {
                    str += (char)(c[1]+c[0]*16);
                    count = 0;
                }
                else if(i != ' ' && count < 2)
                {
                    c[count++] = i.isDigit()?i.toLatin1()-48:i.toUpper().toLatin1()-55;
                }
            }
            if(count)
            {
                str += (char)(c[1]+c[0]*16);
            }
        }
        else if(index == -2)
        {
            for(QChar i:src)
            {
                str += (char)HEX(i.toLatin1()/16);
                str += (char)HEX(i.toLatin1()%16);
                str += ' ';
            }
        }
        txtRecv->setPlainText(str);
    }
}

void WgtSerial::sltSenTog(int index,bool b)
{
    if(b)
    {
        int count(0);
        QString src(txtSend->toPlainText());
        QString str;
        qDebug() << index;
        if(index == -3)
        {
            char c[2]={0,0};
            for(QChar i:src)
            {
                if((count >= 2 && i != ' ' )|| !(i.isDigit() || i.isLetter() || i==' '))
                {
                    QMessageBox::critical(this,"错误","Hex格式错误！",QMessageBox::Ok);
                    rbtSenHex->setChecked(true);
                    txtSend->setPlainText(src);
                    return;
                }
                else if(i == ' ' && count)
                {
                    str += (char)(c[1]+c[0]*16);
                    count = 0;
                }
                else if(i != ' ' && count < 2)
                {
                    c[count++] = i.isDigit()?i.toLatin1()-48:i.toUpper().toLatin1()-55;
                }
            }
            if(count)
            {
                str += (char)(c[1]+c[0]*16);
            }
        }
        else if(index == -2)
        {
            for(QChar i:src)
            {
                char j;
                str += (char)((j=i.toLatin1()/16)<10?j+48:j+55);
                str += (char)((j=i.toLatin1()%16)<10?j+48:j+55);
                str += ' ';
            }
        }
        txtSend->setPlainText(str);
    }
}

void WgtSerial::sltReadBuf()
{
    if(rbtRecTxt->isChecked())
        txtRecv->insertPlainText(xTendSerial->readAll());
    else
    {
        QByteArray src = xTendSerial->readAll();
        QString str;
        for(u8 i:src)
        {
            u8 j;
            qDebug("%x",i);
            str += (char)((j=i/16)<10?j+48:j+55);
            str += (char)((j=i%16)<10?j+48:j+55);
            str += ' ';
        }
        txtRecv->insertPlainText(str);
    }
}

void WgtSerial::resizeEvent(QResizeEvent *event)
{
    QSize siz(event->size());
    labPort->move(siz.width()-180,20);
    cbxPort->move(siz.width()-110,20);
    labBaud->move(siz.width()-180,110);
    cbxBaud->move(siz.width()-120,110);
    labStop->move(siz.width()-180,155);
    cbxStop->move(siz.width()-120,155);
    labExam->move(siz.width()-180,200);
    cbxExam->move(siz.width()-120,200);
    labRecv->move(5,15);
    labSend->move(5,siz.height()/2+5);
    rbtRecHex->move(5,37);
    rbtRecTxt->move(5,59);
    rbtSenHex->move(5,siz.height()/2+27);
    rbtSenTxt->move(5,siz.height()/2+49);
    btnRecv->move(5,81);
    btnSend->move(5,siz.height()/2+71);
    txtRecv->setGeometry(70,10,siz.width()-260,siz.height()/2-20);
    txtSend->setGeometry(70,siz.height()/2,siz.width()-260,siz.height()/2-20);
    btnToggle->move(siz.width()-180,60);
    btnRefresh->move(siz.width()-98,60);
    btnSenMsg->move(siz.width()-180,245);
}
