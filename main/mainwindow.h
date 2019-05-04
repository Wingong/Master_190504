#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include "wgtserial.h"
#include "wgtchat.h"
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

    QTabWidget	*wgtTab;

    MySettings	*settings;

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void resizeEvent(QResizeEvent *event);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

    WgtSerial	*wgtSerial;
    WgtChat		*wgtChat;
    WgtSettings	*wgtSettings;

public slots:
    void sltTab(int index);

};

#endif // MAINWINDOW_H
