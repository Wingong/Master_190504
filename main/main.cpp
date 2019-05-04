#include "mainwindow.h"
#include <QApplication>
#include "sendthread.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    SendThread *th1 = new SendThread;
    th1->start();

    return a.exec();
}
