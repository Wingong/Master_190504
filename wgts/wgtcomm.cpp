#include "wgtcomm.h"
#include "ui_wgtcomm.h"

#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QCryptographicHash>

WgtComm::WgtComm(QWidget *parent) :
    QWidget(parent),
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

    connect(ui->txtTosend,&QPlainTextEdit::textChanged,this,[=](){
        if(!ui->btnSend->isEnabled() && ui->txtTosend->toPlainText().size())
        {
            ui->btnSend->setEnabled(true);
        }
        else if(ui->btnSend->isEnabled() && (ui->txtTosend->toPlainText().size() == 0))
        {
            ui->btnSend->setEnabled(false);
        }

    });
    connect(ui->cbxUPort,&MyComboBox::clicked,this,&WgtComm::on_CbxClicked);
    connect(ui->cbxLPort,&MyComboBox::clicked,this,&WgtComm::on_CbxClicked);
    connect(ui->cbxTIP,&MyComboBox::clicked,this,&WgtComm::on_CbxClicked);
    //刷新当前连接
    connect(ui->cbxTCon,&MyComboBox::currentTextChanged,this,[=](const QString &text){
        QRegExp rx = QRegExp("([0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}):([0-9]{1,5})");
        if(rx.indexIn(text) == -1)
            return;
        for(QTcpSocket *&socket:tcp->socketList)
        {
            if(socket->peerAddress().toString() == rx.cap(1) && socket->peerPort() == rx.cap(2).toInt())
            {
                currentSocket = socket;
                qDebug() << currentSocket->peerPort();
                return;
            }
        }
        currentSocket = nullptr;
    });
    //文本发送
    connect(ui->btnSend,&QPushButton::clicked,[=](){
        QString str = ui->txtTosend->toPlainText();
        QByteArray arr = str.toLatin1();
        ui->wgtScroll->newRecord(User::SEND,str);
        QScrollBar *bar = ui->scrollArea->verticalScrollBar();
        bar->setValue(bar->maximum());
        QByteArray size;
        size.append((u8)(arr.size()>>24));
        size.append((u8)(arr.size()>>16));
        size.append((u8)(arr.size()>>8));
        size.append((u8)(arr.size()));
        arr.push_front(size);
        arr.push_front("\xFE\x44");
        arr.append("\xFE\xED");
        serU->write(arr);

    });
    //图像发送
    connect(ui->btnFSend,&QPushButton::clicked,[=]()
    {
        QString path(QFileDialog::getOpenFileName(this,tr("打开文件"),tr("."),tr("JPEG Image (*.jpg *.jpeg);;All Files (*.*)")));
        if(path == tr(""))
            return;
        ui->wgtScroll->newRecord(User::SEND,path,"JPG");
        QScrollBar *bar = ui->scrollArea->verticalScrollBar();
        bar->setValue(bar->maximum());
        QFile file(path);
        if(!file.open(QIODevice::ReadOnly))
        {
            qDebug() << "file open failed!";
            return;
        }
        QByteArray arr = file.readAll();
        sendIMG(arr);
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

    connect(ui->btnTDiscon,&QPushButton::clicked,this,[=](){
        tcp->discon(currentSocket);
    });
    connect(thImageSend,&ThdImageSend::readOK,this,[=](){
        if(serU->isOpen())
        {
            QByteArray tosend(thImageSend->tosend);
            serU->write(tosend);
        }
    },Qt::QueuedConnection);
    connect(serU,&Serial::bytesWritten,this,[=](qint64 bytes){
        ui->txtInfo->appendPlainText(tr("%1 bytes written successly\n").arg(bytes));
    });
    connect(serU,&Serial::readyRead,[=](){
        QByteArray readArr(serU->readAll());
        //图像 - 首帧
        if(uartMode == NON && readArr.left(2) == "\xFEI")
        {
            QByteArray size = readArr.mid(2,4);
            imgSize = ((u8)size[0]<<24) | ((u8)size[1]<<16) | ((u8)size[2]<<8) | ((u8)size[3]);
            imgSize += 40;
            ui->txtInfo->appendPlainText(tr("IMG Recv %1\%").arg(QString::number((uartBuffer.size()*100/imgSize))));
            if(readArr.right(2) == "\xFE\xED")
            {
                QByteArray contArr = readArr.mid(38,readArr.size()-40);
                QByteArray hash = readArr.mid(6,32);
                if(QCryptographicHash::hash(contArr,QCryptographicHash::Md5).toHex() == hash)
                {
                    for(int i=0;i<contArr.size();i++)
                    {
                        if(contArr[i] == '\\')
                        {
                            contArr.remove(i,1);
                            switch(contArr[i]){
                            case '1':
                                contArr[i] = 0x11;
                                break;
                            case '3':
                                contArr[i] = 0x13;
                                break;
                            default:
                                break;
                            }
                        }
                    }
                    QDate date = QDate::currentDate();
                    QTime time = QTime::currentTime();
                    QString path(tr("%1-%2-%3-%4-%5-%6.jpg").arg(date.year()).arg(date.month()).arg(date.day()).arg(time.hour()).arg(time.minute()).arg(time.second()));
                    ui->txtInfo->appendPlainText(tr("Saved as %1\n").arg(path));
                    QFile file(path);
                    if(!file.open(QFile::ReadWrite))
                    {
                        ui->txtInfo->appendPlainText(file.errorString());
                        return;
                    }
                    file.write(contArr);
                    file.close();
                    ui->wgtScroll->newRecord(User::RECV,path,"JPG");
                    QScrollBar *bar = ui->scrollArea->verticalScrollBar();
                    bar->setValue(bar->maximum());
                }
            }
            else
            {
                uartMode = IMG;
                uartBuffer = readArr;
            }
        }
        //文本 - 首帧
        else if(uartMode == NON && readArr.left(2) == "\xFE\x44")
        {
            QByteArray size = readArr.mid(2,4);
            imgSize = ((u8)size[0]<<24) | ((u8)size[1]<<16) | ((u8)size[2]<<8) | ((u8)size[3]);
            if(readArr.right(2) == "\xFE\xED")
            {
                ui->txtInfo->appendPlainText(tr("%1 bytes data received\n").arg(readArr.size()));
                QString contStr = readArr.mid(6,readArr.size()-8);
                ui->wgtScroll->newRecord(User::RECV,contStr);
                QScrollBar *bar = ui->scrollArea->verticalScrollBar();
                bar->setValue(bar->maximum());
            }
            else
            {
                uartMode = DAT;
                uartBuffer = readArr;
            }
        }
        //语音
        else if(readArr.left(2) == "\xFEV" && readArr.right(2) == "\xFE\xED")
        {
            QByteArray contArr = readArr.mid(2,readArr.size()-4);
            for(int i=0;i<contArr.size();i++)
            {
                if(contArr[i] == '\\')
                {
                    contArr.remove(i,1);
                    switch(contArr[i]){
                    case '1':
                        contArr[i] = 0x11;
                        break;
                    case '3':
                        contArr[i] = 0x13;
                        break;
                    default:
                        break;
                    }
                }
            }
            voiceBuffer.append(contArr);
            qDebug() << contArr.size();
            qDebug() << voiceBuffer.size();
        }
        //图像 - 普通帧
        else if(uartMode == IMG)
        {
            if(readArr.right(2) == "\xFE\xED")
            {
                uartBuffer.append(readArr);
                QTextCursor tc = ui->txtInfo->textCursor();
                tc.select(QTextCursor::BlockUnderCursor);
                tc.removeSelectedText();
                ui->txtInfo->appendPlainText(tr("IMG Recv %1\%").arg(QString::number((uartBuffer.size()*100/imgSize))));
                QByteArray contArr = uartBuffer.mid(38,uartBuffer.size()-40);
                QByteArray hash = uartBuffer.mid(6,32);
                if(QCryptographicHash::hash(contArr,QCryptographicHash::Md5).toHex() == hash)
                {
                    for(int i=0;i<contArr.size();i++)
                    {
                        if(contArr[i] == '\\')
                        {
                            contArr.remove(i,1);
                            switch(contArr[i]){
                            case '1':
                                contArr[i] = 0x11;
                                break;
                            case '3':
                                contArr[i] = 0x13;
                                break;
                            default:
                                break;
                            }
                        }
                    }
                    QDate date = QDate::currentDate();
                    QTime time = QTime::currentTime();
                    QString path(tr("%1-%2-%3-%4-%5-%6.jpg").arg(date.year()).arg(date.month()).arg(date.day()).arg(time.hour()).arg(time.minute()).arg(time.second()));
                    ui->txtInfo->appendPlainText(tr("Saved as %1\n").arg(path));
                    QFile file(path);
                    if(!file.open(QFile::ReadWrite))
                    {
                        ui->txtInfo->appendPlainText(file.errorString());
                        return;
                    }
                    file.write(contArr);
                    file.close();
                    ui->wgtScroll->newRecord(User::RECV,path,"JPG");
                    QScrollBar *bar = ui->scrollArea->verticalScrollBar();
                    bar->setValue(bar->maximum());
                    uartMode = NON;
                }
            }
            else
            {
                uartBuffer.append(readArr);
                QTextCursor tc = ui->txtInfo->textCursor();
                tc.select(QTextCursor::BlockUnderCursor);
                tc.removeSelectedText();
                ui->txtInfo->appendPlainText(tr("IMG Recv %1\%").arg(QString::number((uartBuffer.size()*100/imgSize))));
            }
        }
        //文本 - 普通帧
        else if(uartMode == DAT)
        {
            if(readArr.right(2) == "\xFE\xED")
            {
                uartBuffer.append(readArr);
                ui->txtInfo->appendPlainText(tr("%1 bytes data received\n").arg(uartBuffer.size()));
                QString contStr = uartBuffer.mid(6,uartBuffer.size()-8);
                ui->wgtScroll->newRecord(User::RECV,contStr);
                QScrollBar *bar = ui->scrollArea->verticalScrollBar();
                bar->setValue(bar->maximum());
                uartMode = NON;
            }
            else
            {
                uartBuffer.append(readArr);
            }
        }
    });
    connect(tcp,&Tcp::newConnection,[=](QTcpSocket *socket){
        qDebug() << socket->peerAddress().toString();
        ui->txtInfo->appendPlainText("[New Connection]");
        ui->txtInfo->appendPlainText(tr("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
        ui->txtInfo->appendPlainText("");
        connect(socket,&QTcpSocket::readyRead,[=](){
            QByteArray tempArr(socket->readAll());
            ui->txtInfo->appendPlainText(tr("%1\t%2").arg(tempArr.size()).arg(QString(tempArr.right(2).toHex())));
            switch(mode){
            case XPackage::NON:
                if(rxTerminalHeader.indexIn(tempArr) != -1)
                {
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
                    ui->txtInfo->appendPlainText("New IMG\n");
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
                    QScrollBar *bar = ui->scrollArea->verticalScrollBar();
                    bar->setValue(bar->maximum());
                    sendIMG(tcpBuffer);
                    //thImageSend->arr = tcpBuffer;
                    //thImageSend->start();
                    tcpBuffer.clear();
                    mode = XPackage::NON;
                }
                break;
            case XPackage::VOI:
                //tcpBuffer.append(tempArr);
                if(tempArr == "END\r\n")
                {
                    mode = XPackage::NON;
                }
                else if(tempArr.size() % 30 == 0)
                {
                    tempArr.push_front("\xFEV");
                    tempArr.push_back("\xFE\xED");
                    for(int i=0;i<tempArr.size();i++)
                    {
                        if(tempArr[i] == '\x11')
                        {
                            tempArr[i] = '1';
                            tempArr.insert(i,'\\');
                            i++;
                        }
                        if(tempArr[i] == '\x13')
                        {
                            tempArr[i] = '3';
                            tempArr.insert(i,'\\');
                            i++;
                        }
                        if(tempArr[i] == '\\')
                        {
                            tempArr.insert(i,'\\');
                            i++;
                        }
                    }
                    serU->write(tempArr);
                }
                break;
            default:
                break;
            }
        });
    });
    connect(tcp,&Tcp::disconnected,[=](QTcpSocket *socket){
        ui->txtInfo->appendPlainText("[Disconnected]");
        ui->txtInfo->appendPlainText(tr("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
        ui->txtInfo->appendPlainText("");
    });
    connect(tcp,&Tcp::newConnection,this,&WgtComm::on_TcpRefreshed);
    connect(tcp,&Tcp::disconnected,this,&WgtComm::on_TcpRefreshed);
    connect(tcp,&Tcp::cleared,this,&WgtComm::on_TcpRefreshed);
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
    }
    if((port & L) && !serU->isOpen())
    {
        serL->refresh();
        QString currentText(ui->cbxLPort->currentText());
        ui->cbxLPort->clear();
        (*ui->cbxLPort) << serL->name;
        ui->cbxLPort->setCurrentText(currentText);
    }
    if((port & T) && !tcp->server->isListening())
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

void WgtComm::sendIMG(QByteArray &arr)
{
    for(int i=0;i<arr.size();i++)
    {
        if(arr[i] == '\x11')
        {
            arr[i] = '1';
            arr.insert(i,'\\');
            i++;
        }
        if(arr[i] == '\x13')
        {
            arr[i] = '3';
            arr.insert(i,'\\');
            i++;
        }
        if(arr[i] == '\\')
        {
            arr.insert(i,'\\');
            i++;
        }
    }
    QByteArray hash = QCryptographicHash::hash(arr,QCryptographicHash::Md5).toHex();
    QByteArray size;
    size.append((u8)(arr.size()>>24));
    size.append((u8)(arr.size()>>16));
    size.append((u8)(arr.size()>>8));
    size.append((u8)(arr.size()));
    arr.push_front(hash);
    arr.push_front(size);
    arr.push_front("\xFEI");
    arr.append("\xFE\xED");
    serU->write(arr);
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
        serU->setFlowControl(QSerialPort::SoftwareControl);
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
    if(tcp->server != nullptr)
        qDebug() << "null";
    if(tcp->server != nullptr && tcp->server->isListening())
    {
        qDebug() << "stop listening";
        tcp->close();
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
        if(!tcp->listen(str,ui->txtTPort->text().toInt()))
        {
            QMessageBox::critical(this,"错误",tcp->server->errorString(),QMessageBox::Ok);
            return;
        }
        ui->btnTToggle->setText("停止监听");
        ui->btnTRefresh->setEnabled(false);
        ui->cbxTIP->setEnabled(false);
        ui->txtTPort->setEnabled(false);
    }
}

void WgtComm::on_Connected()
{}

void WgtComm::on_Disconnected()
{}

void WgtComm::on_TcpReadyRead()
{}

void WgtComm::on_TcpRefreshed()
{
    ui->cbxTCon->clear();
    for(QTcpSocket *&socket:tcp->socketList)
    {
        ui->cbxTCon->addItem(tr("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
    }
    ui->txtTCount->setText(QString::number(ui->cbxTCon->count()));
}
