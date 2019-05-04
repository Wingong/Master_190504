#include "mainwindow.h"

#include <QDebug>
#include <QPalette>
#include <QResizeEvent>
#include <QSerialPort>
#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      wgtTab(new QTabWidget(this)),
      wgtSerial(new WgtSerial(this)),
      wgtChat(new WgtChat(this)),
      wgtSettings(new WgtSettings(this)),
//      menuBarMain(this->menuBar()),
//      menuFile(new QMenu("文件(&F)", menuBarMain)),
//      menuEdit(new QMenu("编辑(&E)", menuBarMain)),
      settings(new MySettings("settings.ini"))
{
//    menuBarMain->addMenu(menuFile);
//    menuBarMain->addMenu(menuEdit);
//
//    menuFile->addAction("打开(&O)",this,SLOT(sltOpen()),QKeySequence("Ctrl+O"));

    this->setWindowTitle("Upper Alpha 190414");
    this->setMinimumHeight(280);
    this->setMinimumWidth(380);
    this->setGeometry(100,100,600,400);

    this->wgtTab->addTab(wgtSerial,"串口助手");
    this->wgtTab->addTab(wgtChat,"聊天");
    this->wgtTab->addTab(wgtSettings,"设置");
    QPalette pal(wgtSerial->palette());
    pal.setColor(QPalette::Background,0xf0f0f0);
    wgtSerial->setAutoFillBackground(true);
    wgtSerial->setPalette(pal);
    wgtSerial->show();
    pal = wgtChat->palette();
    pal.setColor(QPalette::Background,0xf0f0f0);
    wgtChat->setAutoFillBackground(true);
    wgtChat->setPalette(pal);
    wgtChat->show();

    connect(wgtTab,SIGNAL(currentChanged(int)),this,SLOT(sltTab(int)));
}

MainWindow::~MainWindow()
{
    delete	wgtSerial;
    delete	wgtTab;
    wgtSerial	= nullptr;
    wgtTab		= nullptr;
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
        wgtSettings->setEnabled(false);
        qDebug() << '0';
        break;
    case 1:
        wgtSerial->setEnabled(false);
        wgtChat->setEnabled(true);
        wgtSettings->setEnabled(false);
        qDebug() << '1';
        break;
    case 2:
        wgtSerial->setEnabled(false);
        wgtChat->setEnabled(false);
        wgtSettings->setEnabled(true);
        qDebug() << '2';
        break;
    default:
        qDebug() << 'F';
        ;
    }
}
