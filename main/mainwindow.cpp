#include "mainwindow.h"

#include <QDebug>
#include <QPalette>
#include <QResizeEvent>
#include <QSerialPort>
#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent)     //初始化成员变量
    : QMainWindow(parent),
      wgtTab(new QTabWidget(this)),
      settings(new MySettings("settings.ini")),
      wgtSerial(new WgtSerial(this)),
      wgtChat(new WgtChat(this)),
      wgtVideo(new WgtVideo(this)),
      wgtSettings(new WgtSettings(this))
//      menuBarMain(this->menuBar()),
//      menuFile(new QMenu("文件(&F)", menuBarMain)),
//      menuEdit(new QMenu("编辑(&E)", menuBarMain)),
{
//    menuBarMain->addMenu(menuFile);
//    menuBarMain->addMenu(menuEdit);
//
//    menuFile->addAction("打开(&O)",this,SLOT(sltOpen()),QKeySequence("Ctrl+O"));

    this->setWindowTitle("无人机应急通信基站 v0.3.0");    //设置窗体标题
    this->setMinimumHeight(280);                //设置窗体最小宽、高
    this->setMinimumWidth(380);
    this->setGeometry(100,100,600,400);         //设置窗体尺寸和位置

    this->wgtTab->addTab(wgtSerial,"串口助手");  //添加标签窗体
    this->wgtTab->addTab(wgtChat,"传输");
    this->wgtTab->addTab(wgtVideo,"视频");
    this->wgtTab->addTab(wgtSettings,"设置");
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
                                                //连接窗体切换和窗体切换槽函数
    connect(wgtTab,SIGNAL(currentChanged(int)),this,SLOT(sltTab(int)));
}

MainWindow::~MainWindow()
{
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
            if(wgtTab->currentIndex() ==  0)
                wgtSerial->sltRefresh();
            else if(wgtTab->currentIndex() == 1)
                wgtChat->sltRefresh();
            else if(wgtTab->currentIndex() == 2)
                wgtVideo->sltRefresh();
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

void MainWindow::sltTab(int index)
{
    switch(index)
    {
    case 0:
        wgtSerial->setEnabled(true);
        wgtChat->setEnabled(false);
        wgtVideo->setEnabled(false);
        wgtSettings->setEnabled(false);
        qDebug() << '0';
        break;
    case 1:
        wgtSerial->setEnabled(false);
        wgtChat->setEnabled(true);
        wgtVideo->setEnabled(false);
        wgtSettings->setEnabled(false);
        qDebug() << '1';
        break;
    case 2:
        wgtSerial->setEnabled(false);
        wgtChat->setEnabled(false);
        wgtVideo->setEnabled(true);
        wgtSettings->setEnabled(false);
        qDebug() << '2';
        break;
    case 3:
        wgtSerial->setEnabled(false);
        wgtChat->setEnabled(false);
        wgtVideo->setEnabled(false);
        wgtSettings->setEnabled(true);
        qDebug() << '3';
        break;
    default:
        qDebug() << 'F';
        ;
    }
}
