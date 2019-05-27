/**
  *********************
  * MainWindow类头文件 *
  *********************
  */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include "wgtserial.h"
#include "wgtchat.h"
#include "wgtvideo.h"
#include "wgtsettings.h"
#include "mysettings.h"
//#include <QMenuBar>
//#include <QMenu>

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
//    QMenuBar	*menuBarMain;
//    QMenu		*menuFile;
//    QMenu		*menuEdit;

    QTabWidget	*wgtTab;		//标签窗体指针

    MySettings	*settings;		//设置类的实例指针

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
                                //重载尺寸变化事件
    void resizeEvent(QResizeEvent *event);
                                //重载本地事件虚函数，用来监听USB消息
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

    WgtSerial	*wgtSerial;		//串口窗体
    WgtChat		*wgtChat;		//通信窗体
    WgtVideo    *wgtVideo;      //视频窗体
    WgtSettings	*wgtSettings;   //设置窗体（无内容）

public slots:
    void sltTab(int index);		//切换标签：槽函数

};

#endif // MAINWINDOW_H
