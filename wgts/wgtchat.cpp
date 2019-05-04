#include <QSize>
#include <QResizeEvent>
#include <QString>
#include "wgtchat.h"

WgtChat::WgtChat(QWidget *parent)
    : QWidget(parent),
      hist(new QTextEdit(this)),
      edit(new QTextEdit(this)),
      send(new QPushButton("发送",this)),
      serRecv(new Serial),
      serSend(new Serial)
{
    hist->setReadOnly(true);
    connect(send,SIGNAL(clicked(bool)),this,SLOT(sltSend()));

}

void WgtChat::sltSend()
{
    hist->append("");
    QString str;
    QDateTime time(QDateTime::currentDateTime());
    str += QString::number(time.time().hour());
    str += ':';
    str += QString::number(time.time().minute());
    str += ':';
    str += QString::number(time.time().second());
    hist->append(str);
    hist->append(edit->toPlainText());
    edit->clear();
}

void WgtChat::resizeEvent(QResizeEvent *event)
{
    QSize siz(event->size());
    hist->setGeometry(10,10,siz.width()-20,siz.height()-140);
    edit->setGeometry(10,siz.height()-125,siz.width()-20,70);
    send->move(20,siz.height()-30);
}
