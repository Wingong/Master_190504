#include "mainwindow.h"

#include <QDebug>
#include <QPalette>
#include <QResizeEvent>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)     //初始化成员变量
    : QMainWindow(parent),
      wgtTab(new QTabWidget(this)),
      settings(new QSettings("settings.ini",QSettings::IniFormat)),
      wgtSerial(new WgtSerial(this)),
      wgtChat(new WgtChat(this)),
      wgtComm(new WgtComm(this)),
      wgtNet(new WgtNet(this)),
      wgtSettings(new WgtSettings(this)),
      wgtVideo(new WgtVideo(this))
{
    this->setWindowTitle("无人机应急通信基站 v0.6.1");    //设置窗体标题
    this->setMinimumHeight(280);                //设置窗体最小宽、高
    this->setMinimumWidth(380);
    //this->setWindowIcon(QIcon(":/icons/icon.ico"));

    wgtTab->addTab(wgtSerial,"串口助手");  //添加标签窗体
    wgtTab->addTab(wgtChat,"传输");
    wgtTab->addTab(wgtComm,"视频");
    wgtTab->addTab(wgtNet,"网络助手");
    wgtTab->addTab(wgtSettings,"设置");
    wgtTab->addTab(wgtVideo,"视频");
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
    pal = wgtComm->palette();
    pal.setColor(QPalette::Background,0xf0f0f0);
    wgtComm->setAutoFillBackground(true);
    wgtComm->setPalette(pal);
    wgtComm->show();
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
    wgtSerial->cbxBaud->setCurrentText(settings->value("Serial/Baud",QVariant("115200")).toString());
    wgtComm->loadSettings(*settings);
    wgtSettings->cbxPort->setCurrentText(settings->value("Settings/Port",QVariant("COM1")).toString());
    wgtSettings->cbxBaud->setCurrentText(settings->value("Settings/Baud",QVariant("115200")).toString());
    wgtSettings->cbxStop->setCurrentIndex(settings->value("Settings/Stop",QVariant(0)).toInt());
    wgtSettings->cbxExam->setCurrentIndex(settings->value("Settings/Parity",QVariant(0)).toInt());
    wgtSettings->cbxUART->setCurrentText(settings->value("Settings/Baud",QVariant("115200")).toString());
}

MainWindow::~MainWindow()
{
    disconnect(wgtTab,SIGNAL(currentChanged(int)),this,SLOT(sltTab()));
    QSettings tempSet("settings.ini",QSettings::IniFormat);
    tempSet.setValue("Main/Index",QVariant(wgtTab->currentIndex()));
    tempSet.setValue("Main/Geometry",QVariant(geometry()));
    tempSet.setValue("Serial/Baud",QVariant(wgtSerial->cbxBaud->currentText()));
    wgtComm->writeSettings(tempSet);
    tempSet.setValue("Settings/Port",QVariant(wgtSettings->cbxPort->currentText()));
    tempSet.setValue("Settings/Baud",QVariant(wgtSettings->cbxBaud->currentText()));
    tempSet.setValue("Settings/Stop",QVariant(wgtSettings->cbxStop->currentIndex()));
    tempSet.setValue("Settings/Parity",QVariant(wgtSettings->cbxExam->currentIndex()));
    qDebug() << "Des";

    delete  wgtSerial;
    //delete  wgtChat;
    //delete  wgtComm;
    //delete  wgtSettings;
    delete  wgtTab;
    wgtSerial   = nullptr;
    //wgtChat     = nullptr;
    //wgtComm    = nullptr;
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
            wgtSerial->sltRefresh();
            wgtChat->sltRefresh();
            wgtComm->refresh(WgtComm::X | WgtComm::L);
            wgtSettings->sltRefresh();
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
}
