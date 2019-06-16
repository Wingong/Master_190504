#include "wgtsettings.h"
#include <QStringList>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QRegExp>

WgtSettings::WgtSettings(QWidget *parent)
    : QWidget(parent),
      tabAT(new QTabWidget(this)),
      treeESPAT(new QTreeWidget(this)),
      treeXTENDAT(new QTreeWidget(this)),
      groupPort(new QGroupBox("串口配置",this)),
      itemESP(CMDCNT,nullptr),
      btnER(CMDCNT,nullptr),
      btnEW(CMDCNT,nullptr),
      cbxUART(new MyComboBox(2,this)),
      cbxCWMODE(new MyComboBox(3,this)),
      cbxCWJAP(new MyComboBox(4,this)),
      txtCIFSR(new QLineEdit(this)),
      //test(new QPushButton("测试",this)),

      labPort(new QLabel("选择串口",groupPort)),
      labBaud(new QLabel("波特率",groupPort)),
      labStop(new QLabel("停止位",groupPort)),
      labExam(new QLabel("校验位",groupPort)),
      cbxPort(new MyComboBox(5,groupPort)),
      cbxBaud(new MyComboBox(6,groupPort)),
      cbxStop(new MyComboBox(7,groupPort)),
      cbxExam(new MyComboBox(8,groupPort)),
      btnRefresh(new QPushButton("刷新",groupPort)),
      btnToggle(new QPushButton("打开",groupPort)),

      //regexp(new QLineEdit("\\+CWMODE",this)),

      hist(new QTextEdit(this)),
      tose(new QTextEdit(this)),
      btnOnePush(new QPushButton("一键连接",this)),
      btnSend(new QPushButton("发送",this)),

      serial(new Serial),
      server(new QTcpServer(this))
{
    tabAT->move(50,30);
    tabAT->addTab(treeESPAT,"ESP8266");
    tabAT->addTab(treeXTENDAT,"XTend");
    treeESPAT->setGeometry(100,100,440,200);
    //treeESPAT->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint);
    treeESPAT->setColumnCount(4);
    treeESPAT->setColumnWidth(0,100);
    treeESPAT->setColumnWidth(1,220);
    treeESPAT->setColumnWidth(2,40);
    treeESPAT->setColumnWidth(3,40);
    treeESPAT->setStyleSheet("QTreeWidget::item{height:30px}");
    QHeaderView *head(treeESPAT->header());
    head->setSectionResizeMode(0,QHeaderView::Fixed);
    head->setSectionResizeMode(1,QHeaderView::Fixed);
    head->setSectionResizeMode(2,QHeaderView::Fixed);
    head->setSectionResizeMode(3,QHeaderView::Fixed);
    head->setStretchLastSection(false);
    QStringList strl;
    strl.append("AT指令");
    strl.append("内容");
    strl.append("读取");
    strl.append("写入");
    treeESPAT->setHeaderLabels(strl);

    groupPort->resize(350,135);
    dialog=new MyProgressDialog(this);

    for(int i=0;i<CMDCNT;i++)
    {
        itemESP[i] = new QTreeWidgetItem(treeESPAT);
        itemESP[i]->setToolTip(0,strHint[i]);
        itemESP[i]->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        itemESP[i]->setText(0,strItem[i]);
        itemESP[i]->setTextAlignment(2,Qt::AlignCenter);
        itemESP[i]->setTextAlignment(3,Qt::AlignCenter);
        if(boolER[i])
        {
            btnER[i] = new QPushButton(this);
            treeESPAT->setItemWidget(itemESP[i],2,btnER[i]);
            btnER[i]->setFixedSize(QSize(25,25));
            connect(btnER[i],SIGNAL(clicked(bool)),this,SLOT(sltRead()));
        }
        if(boolEW[i])
        {
            btnEW[i] = new QPushButton(this);
            treeESPAT->setItemWidget(itemESP[i],3,btnEW[i]);
            btnEW[i]->setFixedSize(QSize(25,25));
            connect(btnEW[i],SIGNAL(clicked(bool)),this,SLOT(sltWrite()));
        }
    }
    treeESPAT->setItemWidget(itemESP[2],1,cbxUART);
    treeESPAT->setItemWidget(itemESP[3],1,cbxCWMODE);
    treeESPAT->setItemWidget(itemESP[4],1,cbxCWJAP);
    treeESPAT->setItemWidget(itemESP[5],1,txtCIFSR);
    cbxUART->setFixedSize(170,25);
    cbxCWMODE->setFixedSize(170,25);
    cbxCWJAP->setFixedSize(170,25);
    txtCIFSR->setFixedSize(170,25);
    (*cbxUART) << "1200" << "2400" << "4800" << "9600" << "19200"
               << "38400" << "57600" << "115200" << "230400" << "460800";
    (*cbxCWMODE) << "" << "Station模式" << "SoftAP模式" << "SoftAP+Station模式";

    //test->setGeometry(600,400,80,25);

    labPort->setGeometry(10,20,80,25);
    labBaud->setGeometry(10,60,80,25);
    labStop->setGeometry(180,20,80,25);
    labExam->setGeometry(180,60,80,25);
    cbxPort->setGeometry(80,20,80,25);
    cbxBaud->setGeometry(80,60,80,25);
    cbxStop->setGeometry(230,20,80,25);
    cbxExam->setGeometry(230,60,80,25);
    btnRefresh->setGeometry(10,100,80,25);
    btnToggle->setGeometry(110,100,80,25);

    //regexp->setGeometry(600,100,120,25);

    (*cbxPort) << serial->name;
    (*cbxBaud) << "1200" << "2400" << "4800" << "9600" << "19200"
               << "38400" << "57600" << "115200" << "230400" << "460800";
    (*cbxExam) << "无校验" << "奇校验" << "偶校验";
    (*cbxStop) << "1位" << "2位" << "1.5位";

    tose->setText("192.168.137.1:8080,1");

    connect(btnRefresh,SIGNAL(clicked(bool)),this,SLOT(sltRefresh()));
    connect(btnToggle,SIGNAL(clicked(bool)),this,SLOT(sltToggle()));
    connect(serial,SIGNAL(readyRead()),this,SLOT(sltSerialRead()));
    connect(cbxBaud,SIGNAL(currentIndexChanged(int)),this,SLOT(sltCbxTog()));
    connect(cbxCWJAP,SIGNAL(currentIndexChanged(int)),this,SLOT(sltCbxTog()));
    connect(cbxCWMODE,SIGNAL(currentIndexChanged(int)),this,SLOT(sltCbxTog()));
    connect(cbxExam,SIGNAL(currentIndexChanged(int)),this,SLOT(sltCbxTog()));
    connect(cbxPort,SIGNAL(currentIndexChanged(int)),this,SLOT(sltCbxTog()));
    connect(cbxStop,SIGNAL(currentIndexChanged(int)),this,SLOT(sltCbxTog()));
    connect(cbxUART,SIGNAL(currentIndexChanged(int)),this,SLOT(sltCbxTog()));
    connect(btnSend,&QPushButton::clicked,[=](){
        if(serial->isOpen())
        {
            QByteArray arr = tose->toPlainText().toLatin1();
            arr.append("\r\n");
            serial->write(arr);
            qDebug() << arr.toHex();
        }
    });
    connect(btnOnePush,&QPushButton::clicked,[=](){
        if(serial->isOpen())
        {
            cmd = 11;
            emit next();
        }
    });
    connect(this,&WgtSettings::next,[=](){
        QRegExp rx("([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}):([0-9]{1,5}),([0-1])");
        switch(cmd){
        case 11:
            rx.indexIn(tose->toPlainText());
            serial->write(QString("AT+CIPSTART=\"TCP\",\"%1\",%2\r\n").arg(rx.cap(1)).arg(rx.cap(2).toInt()).toLatin1());
            break;
        case 12:
            serial->write(QString("AT+CIPMODE=%1\r\n").arg(rx.cap(3).toInt()).toLatin1());
            break;
        case 13:
            if(rx.cap(3).toInt())
                serial->write(QString("AT+CIPSEND\r\n").toLatin1());
            break;
        default:
            break;
        }
    });
}

void WgtSettings::sltRead()
{
    if(!serial->isOpen())
    {
        QMessageBox::critical(this,"错误","串口未打开！",QMessageBox::Ok);
        return;
    }
    QPushButton *act = qobject_cast<QPushButton *>(sender());
    for(cmd=0;cmd<CMDCNT;cmd++)
    {
        if(act == btnER[cmd])
            break;
    }
    qDebug() << "CMD = " << cmd;
    QString str(strRead[cmd]);
    serial->write(str.toLatin1());
           qDebug() << str.toLatin1().toHex();
    dialog=new MyProgressDialog(this);
    QWidget *par = qobject_cast<QWidget *>(parent()->parent()->parent());
    dialog->move(par->x()+par->width()/2-150,par->y()+par->height()/2-75);
    switch(dialog->exec())
    {
    case -1:
        QMessageBox::critical(this,"错误","等待超时",QMessageBox::Ok);
        switch(cmd & ~0x8000){
        case 0:
        case 1:
            itemESP[cmd]->setText(1,"ERROR");
            break;
        case 3:
            cbxCWMODE->setCurrentIndex(0);
            break;
        case 4:
            break;
        default:
            break;
        }
        break;
    case 0:
        QMessageBox::information(this,"提示","用户取消等待",QMessageBox::Ok);
        break;
    }
}

void WgtSettings::sltWrite()
{
    if(!serial->isOpen())
    {
        QMessageBox::critical(this,"错误","串口未打开！",QMessageBox::Ok);
        return;
    }
    QPushButton *act = qobject_cast<QPushButton *>(sender());
    for(cmd=0;cmd<CMDCNT;cmd++)
    {
        if(act == btnEW[cmd])
            break;
    }
    qDebug() << "CMD = " << cmd;
    QString str(strWrite[cmd]);
    bool ok(true);
    switch(cmd)
    {
    case 2:
        str = str.arg(cbxUART->currentText());
        break;
    case 3:
        str = str.arg(cbxCWMODE->currentIndex());
        break;
    case 4:
        str = str.arg(cbxCWJAP->currentText()).arg(QInputDialog::getText(this,"Wi-Fi连接",QString("请输入无线网络%1的密码：").arg(cbxCWJAP->currentText()),QLineEdit::Normal,"",&ok));
        break;
    default:
        break;
    }
    if(!ok)
        return;
    qDebug() << str;
    cmd |= 0x8000;
    serial->write(str.toLatin1());
    dialog=new MyProgressDialog(this);
    QWidget *par = qobject_cast<QWidget *>(parent()->parent()->parent());
    dialog->move(par->x()+par->width()/2-150,par->y()+par->height()/2-75);
    switch(dialog->exec())
    {
    case -1:
        QMessageBox::critical(this,"错误","等待超时",QMessageBox::Ok);
        break;
    case 0:
        QMessageBox::information(this,"提示","用户取消等待",QMessageBox::Ok);
        break;
    case 2:
        QMessageBox::information(this,"提示","写入成功！",QMessageBox::Ok);
        break;
    }
}

void WgtSettings::sltRefresh()
{
    if(!serial->isOpen())
    {
        serial->refresh();
        QString str(cbxPort->currentText());
        cbxPort->clear();
        (*cbxPort) << serial->name;
        cbxPort->setCurrentText(str);
    }
    qDebug() << "refreshed\n";
}

void WgtSettings::sltToggle()
{
    if(serial->isOpen())
    {
        serial->close();
        cbxPort->setEnabled(true);
        btnRefresh->setEnabled(true);
        btnToggle->setText("打开");
    }
    else
    {
        serial->setPortName(cbxPort->currentText());//cbxPort->currentText());
        if(!serial->open(QIODevice::ReadWrite))
        {
            QMessageBox::critical(this,"错误","打开失败！",QMessageBox::Ok);
            return;
        }
        serial->setBaudRateIndex(cbxBaud->currentIndex());
        serial->setDataBits(QSerialPort::Data8);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        serial->setParity(QSerialPort::Parity(cbxExam->currentIndex()));
        serial->setStopBits(QSerialPort::StopBits(cbxStop->currentIndex()));
        qDebug() << serial->stopBits();
        cbxPort->setEnabled(false);
        btnRefresh->setEnabled(false);
        btnToggle->setText("关闭");
    }
}

void WgtSettings::sltCbxTog()
{
    MyComboBox *act = qobject_cast<MyComboBox *>(sender());
    switch(act->index)
    {
    case 6:
        serial->setBaudRateIndex(cbxBaud->currentIndex());
        break;
    case 7:
        serial->setStopBits(QSerialPort::StopBits(cbxStop->currentIndex()));
        break;
    case 8:
        serial->setParity(QSerialPort::Parity(cbxExam->currentIndex()));
        break;
    default:
        break;
    }
}

void WgtSettings::sltSerialRead()
{
    QByteArray tempBuffer(serial->readAll());
    if(cmd == -1)
    {
        hist->append(tempBuffer);
        return;
    }
    hist->append(tempBuffer);
    buffer.append(tempBuffer);
    QRegExp re("OK\\r\\n$");
    QRegExp re2("ERROR\r\n");
    QRegExp re3("FAIL\\r\\n");
    qDebug() << dialog->bar->value();
    if(re.indexIn(tempBuffer) != -1)
    {
        QRegExp re3;
        QString str;
        int pos(0);
        if(cmd & 0x8000)
        {
            switch(cmd & ~0x8000){
            case 1:
                itemESP[1]->setText(1,"OK");
                break;
            case 2:
                cbxBaud->setCurrentText(cbxUART->currentText());
                serial->setBaudRateIndex(cbxUART->currentIndex());
            default:
                break;
            }
        }
        else
        {
            switch(cmd)
            {
            case 0:
                itemESP[0]->setText(1,"OK");
                break;
            case 3:
                re3=QRegExp("\\+CWMODE:[1-3]");
                re3.indexIn(buffer);
                str=re3.capturedTexts()[0];
                cbxCWMODE->setCurrentIndex(str.right(1).toInt());
                break;
            case 4:
                re3=QRegExp("\\+CWLAP:\\(([0-5])\\,\"([^\"]*)\",\\-");
                cbxCWJAP->clear();
                re3.setMinimal(true);
                while((pos=re3.indexIn(buffer,pos+1)) != -1)
                {
                    cbxCWJAP->addItem(re3.cap(2),QVariant(re3.cap(1).toInt()));
                }
            case 5:
                re3=QRegExp(":STAIP\\,\"([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})");
                qDebug() << re3.indexIn(buffer);
                str=re3.cap(1);
                txtCIFSR->setText(str);
                break;
            case 11:
                cmd = 12;
                emit next();
                break;
            case 12:
                cmd = 13;
                emit next();
                break;
            default:
                break;
            }
        }
            if(cmd > (0x8000|1))
            {
                dialog->done(2);
                cmd = -1;
            }
            else{
                dialog->done(1);
                cmd = -1;
            }
        //dialog->cancel();
        buffer.clear();
    }
    else if(re2.indexIn(tempBuffer) != -1)
    {
        if(cmd == 0)
        {
            itemESP[0]->setText(1,"ERROR");
        }
        else if(cmd == 1)
        {
            itemESP[1]->setText(1,"ERROR");
        }
            dialog->done(-2);
        cmd = -1;
        QMessageBox::critical(this,"错误","指令错误",QMessageBox::Ok);
        hist->append(buffer);
        hist->append("");
    }
    else if(re3.indexIn(tempBuffer) != -1)
    {
        if(cmd == 0)
        {
            itemESP[0]->setText(1,"FAIL");
        }
        else if(cmd == 1)
        {
            itemESP[1]->setText(1,"FAIL");
        }
            dialog->done(-2);
        cmd = -1;
        QMessageBox::critical(this,"错误","指令失败",QMessageBox::Ok);
        hist->append(buffer);
        hist->append("");
    }
}

void WgtSettings::sltCancel()
{
    if(cmd != -1)
    {
        QMessageBox::information(this,"AT指令","用户取消等待",QMessageBox::Ok);
        cmd = -1;
    }
}

void WgtSettings::sltTest()
{
}

void WgtSettings::resizeEvent(QResizeEvent *event)
{
    int width(event->size().width());
    int height(event->size().height());
    tabAT->resize(450,height>600?400:height-200);
    groupPort->move(50,height-165);
    hist->setGeometry(510,53,width-560,225);
    tose->setGeometry(510,293,width-560,100);
    btnOnePush->setGeometry(width-130,410,80,25);
    btnSend->setGeometry(width-250,410,80,25);
}
