#include "mainwindow.h"

#include <QDebug>
#include <QPalette>
#include <QResizeEvent>
#include <QSerialPort>
#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent)     //初始化成员变量
    : QMainWindow(parent),
      wgtTab(new QTabWidget(this)),
      settings(new QSettings("settings.ini",QSettings::IniFormat)),
      wgtSerial(new WgtSerial(this)),
      wgtChat(new WgtChat(this)),
      wgtVideo(new WgtVideo(this)),
      wgtNet(new WgtNet(this)),
      wgtSettings(new WgtSettings(this))
//      menuBarMain(this->menuBar()),
//      menuFile(new QMenu("文件(&F)", menuBarMain)),
//      menuEdit(new QMenu("编辑(&E)", menuBarMain)),
{
//    menuBarMain->addMenu(menuFile);
//    menuBarMain->addMenu(menuEdit);
//
//    menuFile->addAction("打开(&O)",this,SLOT(sltOpen()),QKeySequence("Ctrl+O"));

    this->setWindowTitle("无人机应急通信基站 v0.4.5");    //设置窗体标题
    this->setMinimumHeight(280);                //设置窗体最小宽、高
    this->setMinimumWidth(380);

    wgtTab->addTab(wgtSerial,"串口助手");  //添加标签窗体
    wgtTab->addTab(wgtChat,"传输");
    wgtTab->addTab(wgtVideo,"视频");
    wgtTab->addTab(wgtNet,"网络助手");
    wgtTab->addTab(wgtSettings,"设置");
    QPalette pal(wgtSerial->palette());         //设置色盘
    pal.setColor(QPalette::Background,0xf0f0f0);
    wgtSerial->setAutoFillBackground(true);     //上底色
    wgtSerial->setPalette(pal);
    wgtSerial->show();
    pal = wgtChat->palette();
    pal.setColor(QPalette::Background,0xf0f0f0);
    wgtChat->setAutoFillBackground(true);
    wgtChat->setPalette(pal);
    wgtChat->show();
    pal = wgtVideo->palette();
    pal.setColor(QPalette::Background,0xf0f0f0);
    wgtVideo->setAutoFillBackground(true);
    wgtVideo->setPalette(pal);
    wgtVideo->show();
    pal = wgtNet->palette();
    pal.setColor(QPalette::Background,0xf0f0f0);
    wgtNet->setAutoFillBackground(true);
    wgtNet->setPalette(pal);
    wgtNet->show();
    pal = wgtSettings->palette();
    pal.setColor(QPalette::Background,0xf0f0f0);
    wgtSettings->setAutoFillBackground(true);
    wgtSettings->setPalette(pal);
    wgtSettings->show();
                                                //连接窗体切换和窗体切换槽函数
    connect(wgtTab,SIGNAL(currentChanged(int)),this,SLOT(sltTab()));

    wgtTab->setCurrentIndex(settings->value("Main/Index",QVariant(2)).toInt());
    setGeometry(settings->value("Main/Geometry",QVariant(QRect(100,100,800,600)).toRect()).toRect());
    wgtSerial->cbxBaud->setCurrentText(QString::number(settings->value("Serial/Baud",QVariant(wgtSerial->xTendSerial->baudRate())).toInt()));
    wgtVideo->cbxPort->setCurrentText(settings->value("Video/BtPort",QVariant(wgtVideo->cbxPort->currentText())).toString());
    wgtVideo->cbxSPort->setCurrentText(settings->value("Video/ChPort",QVariant(wgtVideo->cbxSPort->currentText())).toString());
    wgtVideo->txtIP->setText(settings->value("Video/IP",QVariant(wgtVideo->txtIP->text())).toString());
    wgtVideo->txtTcpPort->setText(settings->value("Video/TCPPort",QVariant(wgtVideo->txtTcpPort->text().toInt())).toString());
    wgtSettings->cbxPort->setCurrentText(settings->value("Settings/Port",QVariant("COM1")).toString());
    wgtSettings->cbxBaud->setCurrentText(settings->value("Settings/Baud",QVariant("115200")).toString());
    wgtSettings->cbxStop->setCurrentIndex(settings->value("Settings/Stop",QVariant(0)).toInt());
    wgtSettings->cbxExam->setCurrentIndex(settings->value("Settings/Parity",QVariant(0)).toInt());
}

MainWindow::~MainWindow()
{
    disconnect(wgtTab,SIGNAL(currentChanged(int)),this,SLOT(sltTab()));
    QSettings tempSet("settings.ini",QSettings::IniFormat);
    tempSet.setValue("Main/Index",QVariant(wgtTab->currentIndex()));
    tempSet.setValue("Main/Geometry",QVariant(geometry()));
    tempSet.setValue("Serial/Baud",QVariant(wgtSerial->xTendSerial->baudRate()));
    tempSet.setValue("Video/BtPort",QVariant(wgtVideo->cbxPort->currentText()));
    tempSet.setValue("Video/ChPort",QVariant(wgtVideo->cbxSPort->currentText()));
    tempSet.setValue("Video/IP",QVariant(wgtVideo->txtIP->text()));
    tempSet.setValue("Video/TCPPort",QVariant(wgtVideo->txtTcpPort->text().toInt()));
    tempSet.setValue("Settings/Port",QVariant(wgtSettings->cbxPort->currentText()));
    tempSet.setValue("Settings/Baud",QVariant(wgtSettings->cbxBaud->currentText()));
    tempSet.setValue("Settings/Stop",QVariant(wgtSettings->cbxStop->currentIndex()));
    tempSet.setValue("Settings/Parity",QVariant(wgtSettings->cbxExam->currentIndex()));
    qDebug() << "Des";

    delete  wgtSerial;
    //delete  wgtChat;
    //delete  wgtVideo;
    //delete  wgtSettings;
    delete  wgtTab;
    wgtSerial   = nullptr;
    //wgtChat     = nullptr;
    //wgtVideo    = nullptr;
    //wgtSettings = nullptr;
    wgtTab      = nullptr;
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    if (eventType == "windows_generic_MSG")
    {
        bool bResult = false;
        MSG* msg = reinterpret_cast<MSG*>(message);
        //PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
        if(msg->message == WM_DEVICECHANGE && msg->wParam >= DBT_DEVICEARRIVAL)
        {
            //if(wgtTab->currentIndex() ==  0)
                wgtSerial->sltRefresh();
            //else if(wgtTab->currentIndex() == 1)
                wgtChat->sltRefresh();
            //else if(wgtTab->currentIndex() == 2)
                wgtVideo->sltRefresh();
                wgtSerial->sltRefresh();
        }
        return bResult;
    }
    else
    {
        return QWidget::nativeEvent(eventType, message, result);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    wgtTab->setGeometry(1,5,event->size().width()-1,event->size().height()-5);
}

void MainWindow::sltTab(void)
{
    wgtTab->currentWidget()->setEnabled(true);
    for(int i=0;i<5;i++)
    {
        if(i != wgtTab->currentIndex())
        {
            wgtTab->widget(i)->setEnabled(false);
        }
    }
    //switch(index)
    //{
    //case 0:
    //    wgtSerial->setEnabled(true);
    //    wgtChat->setEnabled(false);
    //    wgtVideo->setEnabled(false);
    //    wgtSettings->setEnabled(false);
    //    qDebug() << '0';
    //    break;
    //case 1:
    //    wgtSerial->setEnabled(false);
    //    wgtChat->setEnabled(true);
    //    wgtVideo->setEnabled(false);
    //    wgtSettings->setEnabled(false);
    //    qDebug() << '1';
    //    break;
    //case 2:
    //    wgtSerial->setEnabled(false);
    //    wgtChat->setEnabled(false);
    //    wgtVideo->setEnabled(true);
    //    wgtSettings->setEnabled(false);
    //    qDebug() << '2';
    //    break;
    //case 3:
    //    wgtSerial->setEnabled(false);
    //    wgtChat->setEnabled(false);
    //    wgtVideo->setEnabled(false);
    //    wgtSettings->setEnabled(true);
    //    qDebug() << '3';
    //    break;
    //default:
    //    qDebug() << 'F';
    //    ;
    //}
}
