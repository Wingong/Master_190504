#include "wgtcomm.h"
#include "ui_wgtcomm.h"

#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QNetworkInterface>
#include <QHostAddress>

WgtComm::WgtComm(QWidget *parent) :
    QWidget(parent),
    server(new QTcpServer(this)),
    serU(new Serial),
    serL(new Serial),
    tcp(new Tcp(this)),

    thImageDisp(new ThdImageDisp(this)),
    thImageSend(new ThdImageSend(this)),

    ui(new Ui::WgtComm)
{
    ui->setupUi(this);
    ui->scrollArea->setWidget(ui->wgtScroll);

    (*ui->cbxUBaud) << "1200" << "2400" << "4800" << "9600" << "19200"
                    << "38400" << "57600" << "115200" << "230400" << "460800";
    (*ui->cbxLBaud) << "1200" << "2400" << "4800" << "9600" << "19200"
                    << "38400" << "57600" << "115200" << "230400" << "460800";
    refresh(X | T | L);

    connect(ui->cbxUPort,&MyComboBox::clicked,this,&WgtComm::on_CbxClicked);
    connect(ui->cbxLPort,&MyComboBox::clicked,this,&WgtComm::on_CbxClicked);
    connect(ui->cbxTIP,&MyComboBox::clicked,this,&WgtComm::on_CbxClicked);
    connect(ui->btnSend,&QPushButton::clicked,[=](){
        QString str = ui->txtTosend->toPlainText();
        QByteArray fore;
        ui->wgtScroll->newRecord(User::SEND,str);
        package.genForerunner(fore,XPackage::IMG,0x0,800);
        qDebug() << fore.toHex();

    });
    connect(ui->btnFSend,&QPushButton::clicked,[=]()
    {
        QString path(QFileDialog::getOpenFileName(this,tr("打开文件"),tr("."),tr("JPEG Image (*.jpg *.jpeg);;All Files (*.*)")));
        if(path == tr(""))
            return;
        ui->wgtScroll->newRecord(User::SEND,path,"JPG");
    });
    connect(ui->btnHClear,&QPushButton::clicked,[=](){
        ui->scrollArea->takeWidget();
        ui->wgtScroll = new WgtScroll(this);
        ui->wgtScroll->setGeometry(10,10,521,20);
        ui->scrollArea->setWidget(ui->wgtScroll);
        ui->txtInfo->clear();
    });
    connect(ui->btnSClear,&QPushButton::clicked,[=](){
        ui->txtTosend->clear();
    });
    connect(ui->btnURefresh,&QPushButton::clicked,[=](){
        refresh(X);
    });
    connect(ui->btnTRefresh,&QPushButton::clicked,[=](){
        refresh(T);
    });
    connect(ui->btnLRefresh,&QPushButton::clicked,[=](){
        refresh(L);
    });
    connect(ui->btnUToggle,&QPushButton::clicked,this,&WgtComm::on_UToggle);
    connect(ui->btnTToggle,&QPushButton::clicked,this,&WgtComm::on_TToggle);
    connect(ui->btnLToggle,&QPushButton::clicked,this,&WgtComm::on_LToggle);

    connect(thImageSend,&ThdImageSend::readOK,this,[=](){
        if(serU->isOpen())
        {
            QByteArray tosend(thImageSend->tosend);
            serU->write(tosend);
        }
    },Qt::QueuedConnection);
    connect(serU,&Serial::readyRead,[=](){
        QByteArray readArr(serU->readAll());
        if(thImageSend->send_busy && readArr == "\x11")
        {
            thImageSend->send_busy = false;
        }
    });
}

WgtComm::~WgtComm()
{
    delete ui;
}

void WgtComm::on_CbxClicked(int index){
    switch(index)
    {
    case 1:
        qDebug() << "cbxUPort";
        break;
    case 2:
        qDebug() << "cbxLPort";
        break;
    case 3:
        qDebug() << "cbxTIP";
        break;
    default:
        break;
    }
    qDebug() << "Highlighted";
}

bool WgtComm::loadSettings(QSettings &settings)
{
    settings.beginGroup(tr("Communication"));
    ui->cbxUPort->setCurrentText(settings.value("XPort",QVariant("COM1")).toString());
    ui->cbxUBaud->setCurrentText(settings.value("XBaudRate",QVariant(115200)).toString());
    ui->cbxTIP->setCurrentText(settings.value("IP",QVariant(tr("127.0.0.1"))).toString());
    ui->txtTPort->setText(settings.value("TPort",QVariant(tr("80"))).toString());
    ui->cbxLPort->setCurrentText(settings.value("LPort",QVariant("COM1")).toString());
    ui->cbxLBaud->setCurrentText(settings.value("LBaudRate",QVariant(115200)).toString());
    settings.endGroup();
    return true;
}

bool WgtComm::writeSettings(QSettings &settings)
{
    settings.beginGroup(tr("Communication"));
    settings.setValue("XPort",QVariant(ui->cbxUPort->currentText()));
    settings.setValue("XBaudRate",QVariant(ui->cbxUBaud->currentText()));
    settings.setValue("IP",QVariant(ui->cbxTIP->currentText()));
    settings.setValue("TPort",QVariant(ui->txtTPort->text()));
    settings.setValue("LPort",QVariant(ui->cbxLPort->currentText()));
    settings.setValue("LBaudRate",QVariant(ui->cbxLBaud->currentText()));
    settings.endGroup();
    return true;
}

void WgtComm::refresh(int port)
{
    if((port & X) && !serU->isOpen())
    {
        serU->refresh();
        QString currentText(ui->cbxUPort->currentText());
        ui->cbxUPort->clear();
        (*ui->cbxUPort) << serU->name;
        ui->cbxUPort->setCurrentText(currentText);
        qDebug() << "X";
    }
    if((port & L) && !serU->isOpen())
    {
        serL->refresh();
        QString currentText(ui->cbxLPort->currentText());
        ui->cbxLPort->clear();
        (*ui->cbxLPort) << serL->name;
        ui->cbxLPort->setCurrentText(currentText);
        qDebug() << "L";
    }
    if((port & T) && !server->isListening())
    {
        QList<QHostAddress> list(QNetworkInterface::allAddresses());
        ui->cbxTIP->clear();
        QString currentText(ui->cbxTIP->currentText());
        for(auto &i:list)
        {
            if(rxIP.indexIn(i.toString()) != -1)
            {
                (*ui->cbxTIP) << i.toString();
            }
        }
        ui->cbxTIP->setCurrentText(currentText);
        qDebug() << "T";
    }
}

void WgtComm::on_ImaClicked(QString &path)
{
    WgtImageDisp *image(new WgtImageDisp(path));
    image->refresh();
    image->show();
}

void WgtComm::resizeEvent(QResizeEvent *event)
{
    ui->wgtScroll->resize(ui->scrollArea->width()-30,ui->wgtScroll->height());
    QWidget::resizeEvent(event);
}

void WgtComm::on_UToggle()
{
    if(serU->isOpen())
    {
        ui->cbxUPort->setEnabled(true);
        ui->btnURefresh->setEnabled(true);
        ui->btnUToggle->setText("打开");
        serU->close();
    }
    else
    {
        serU->setPortName(ui->cbxUPort->currentText());
        if(!serU->open(QIODevice::ReadWrite))
        {
            QMessageBox::critical(this,"错误","打开失败！",QMessageBox::Ok);
            return;
        }
        //thread->start();
        serU->setBaudRate(ui->cbxUBaud->currentText().toInt());
        serU->setDataBits(QSerialPort::Data8);
        serU->setFlowControl(QSerialPort::NoFlowControl);
        serU->setParity(QSerialPort::NoParity);
        serU->setStopBits(QSerialPort::OneStop);
        ui->cbxUPort->setEnabled(false);
        ui->btnURefresh->setEnabled(false);
        ui->btnUToggle->setText("关闭");
    }
}

void WgtComm::on_LToggle()
{
    if(serL->isOpen())
    {
        ui->cbxLPort->setEnabled(true);
        ui->btnLRefresh->setEnabled(true);
        ui->btnLToggle->setText("打开");
        serL->close();
    }
    else
    {
        serL->setPortName(ui->cbxLPort->currentText());
        if(!serL->open(QIODevice::ReadWrite))
        {
            QMessageBox::critical(this,"错误","打开失败！",QMessageBox::Ok);
            return;
        }
        //thread->start();
        serL->setBaudRate(ui->cbxLBaud->currentText().toInt());
        serL->setDataBits(QSerialPort::Data8);
        serL->setFlowControl(QSerialPort::NoFlowControl);
        serL->setParity(QSerialPort::NoParity);
        serL->setStopBits(QSerialPort::OneStop);
        ui->cbxLPort->setEnabled(false);
        ui->btnLRefresh->setEnabled(false);
        ui->btnLToggle->setText("关闭");
    }
}

void WgtComm::on_TToggle()
{
    if(tcp->server->isListening())
    {
//        if(socket != nullptr)
//        {
//            disconnect(socket,&QTcpSocket::readyRead,[=](){
//
//    client = server->nextPendingConnection();
//    connect(client,SIGNAL(readyRead()),this,SLOT(sltTcpRecv()));
//    connect(client,SIGNAL(disconnected()),this,SLOT(sltDisconnected()));
//    txtInfo->append("[New connection]");
//    txtInfo->append(QString("Time: ").append(QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss")));
//    txtInfo->append(QString("IP: %1").arg(client->peerAddress().toString()));
//    txtInfo->append(QString("Port: %1\n").arg(client->peerPort()));
//            });
//            disconnect(socket,&QTcpSocket::disconnected,[=](){
//                ui->txtInfo->appendPlainText("Client offline.\n");
//                socket->close();
//                delete socket;
//                socket = nullptr;
//            });
//            socket->close();
//            delete socket;
//            socket = nullptr;
//        }
        qDebug() << "stop listening";
        tcp->server->close();
        qDebug() << server->isListening();
        ui->btnTToggle->setText("监听");
        ui->btnTRefresh->setEnabled(true);
        ui->cbxTIP->setEnabled(true);
        ui->txtTPort->setEnabled(true);
        tcpBuffer.clear();
        mode = XPackage::NON;
    }
    else
    {
        QString str = ui->cbxTIP->currentText();
        delete tcp;
        tcp = new Tcp(this);
        if(!tcp->listen(str,ui->txtTPort->text().toInt()))
        {
            QMessageBox::critical(this,"错误",server->errorString(),QMessageBox::Ok);
            return;
        }
        connect(tcp,&Tcp::newConnection,[=](QTcpSocket *socket){
            ui->txtInfo->appendPlainText("[New Connection]");
            ui->txtInfo->appendPlainText(tr("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
            ui->txtInfo->appendPlainText("");
            connect(socket,&QTcpSocket::readyRead,[=](){
                qDebug() << QTime::currentTime();
                QByteArray tempArr(socket->readAll());
                switch(mode){
                case XPackage::NON:
                    if(rxTerminalHeader.indexIn(tempArr) != -1)
                    {
                        qDebug() << rxTerminalHeader.cap(1).toInt();
                        switch(rxTerminalHeader.cap(1).toInt())
                        {
                        case 1:
                            mode = XPackage::IMG;
                            break;
                        case 2:
                            mode = XPackage::VOI;
                            break;
                        default:
                            break;
                        }
                        qDebug() << mode;
                        if(mode != XPackage::NON)
                        {
                            tcpBuffer.clear();
                            socket->write("ACK\r\n");
                        }
                    }
                    break;
                case XPackage::IMG:
                    tcpBuffer.append(tempArr);
                    if(tempArr.right(2) == jpgend)
                    {
                        ui->txtInfo->appendPlainText(tr("Size: %1").arg(tcpBuffer.size()));
                        QDate date = QDate::currentDate();
                        QTime time = QTime::currentTime();
                        QString path(tr("%1-%2-%3-%4-%5-%6.jpg").arg(date.year()).arg(date.month()).arg(date.day()).arg(time.hour()).arg(time.minute()).arg(time.second()));
                        ui->txtInfo->appendPlainText(tr("Saved as %1").arg(path));
                        QFile file(path);
                        if(!file.open(QFile::ReadWrite))
                        {
                            ui->txtInfo->appendPlainText(file.errorString());
                            return;
                        }
                        file.write(tcpBuffer);
                        file.close();
                        ui->wgtScroll->newRecord(User::SEND,path,"JPG");
                        thImageSend->arr = tcpBuffer;
                        thImageSend->start();
                        tcpBuffer.clear();
                        mode = XPackage::NON;
                    }
                    break;
                default:
                    break;
                }
                qDebug() << "End: " << QTime::currentTime();
            });
        });
        connect(tcp,&Tcp::disconnected,[=](QTcpSocket *socket){
            ui->txtInfo->appendPlainText("[Disconnected]");
            ui->txtInfo->appendPlainText(tr("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
            ui->txtInfo->appendPlainText("");
        });
        ui->btnTToggle->setText("停止监听");
        ui->btnTRefresh->setEnabled(false);
        ui->cbxTIP->setEnabled(false);
        ui->txtTPort->setEnabled(false);
        //connect(serSend,SIGNAL(bytesWritten(qint64)),this,SLOT(sltWriteOver(qint64)));
    }
}

void WgtComm::on_Connected()
{}

void WgtComm::on_Disconnected()
{}

void WgtComm::on_TcpReadyRead()
{}
