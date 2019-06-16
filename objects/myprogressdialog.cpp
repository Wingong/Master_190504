#include "myprogressdialog.h"
#include <QDebug>
#include <QMainWindow>

MyProgressDialog::MyProgressDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent,f),
      bar(new QProgressBar(this)),
      label(new QLabel("AT指令等待...",this)),
      seconds(new QLabel(this)),
      timer(new QTimer(this))
{
    this->setFixedSize(300,150);
    Qt::WindowFlags flags = this->windowFlags();
    this->setWindowFlags(flags&~Qt::WindowContextHelpButtonHint);
    bar->setGeometry(50,75,width()-75,30);
    bar->setFormat("");
    bar->setRange(0,300);
    label->setGeometry(50,40,200,25);
    seconds->setGeometry(200,80,80,50);
    label->setAlignment(Qt::AlignCenter);
    seconds->setAlignment(Qt::AlignCenter);
    timer->setInterval(50);
    //bar->setFormat("%p");
    bar->reset();
    connect(timer,&QTimer::timeout,[=]
    {
       bar->setValue(bar->value()+1);
       bar->update();
       if(bar->value()%20 == 0)
       {
           label->setText(QString("AT指令等待...%1秒").arg(bar->value()/20+1));
       }
       if(bar->value() == bar->maximum())
       {
           done(-1);
       }
    });
    timer->start();
}

void MyProgressDialog::setValue(int value)
{
    bar->setValue(value);
}
