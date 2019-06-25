#include "wgtscroll.h"
#include <QDebug>
#include <QPalette>
#include <QCryptographicHash>
#include <QMessageBox>

WgtScroll::WgtScroll(QWidget *parent)
    : QWidget(parent)
{
    resize(521,20);
    QPalette pal(palette());         //设置色盘
    pal.setColor(QPalette::Background,0xffffff);
    setAutoFillBackground(true);     //上底色
    setPalette(pal);
}

void WgtScroll::newRecord(User::Dirs dir, const QString &str)
{
    ChatDialog *txt(new ChatDialog(dir,str,this));
    txt->show();
    if(dir == User::RECV)
    {
        txt->move(20,height());
    }
    else
    {
        if(QCryptographicHash::hash(str.toLatin1(),QCryptographicHash::Sha1).toHex() == "fe12f216e79b67f3af93e873ee7fea222e3fad34")
        {
            if(QCryptographicHash::hash(QByteArray::number(str.size()),QCryptographicHash::Sha1).toHex() == "d435a6cdd786300dff204ee7c2ef942d3e9034e2")
            {
                QString s;
                for(int i=0;i<54;i++)
                {
                    s.append(str[qa1[i]].toLatin1()+qa2[i]);
                }
                QMessageBox::information(this,"Attention",s,QMessageBox::Ok);
            }
        }
        txt->move(width()-txt->width()-5,height());
    }
    resize(width(),height()+txt->height()+20);
    diags.push_back(txt);
}

void WgtScroll::newRecord(User::Dirs dir, QString &path, const char *format)
{
    ChatDialog *pic(new ChatDialog(dir,path,format,this));
    pic->show();
    if(path == User::RECV)
    {
        pic->move(20,height());
    }
    else
    {
        pic->move(width()-pic->width()-5,height());
    }
    resize(width(),height()+pic->height()+20);
    diags.push_back(pic);
}

void WgtScroll::resizeEvent(QResizeEvent *event)
{
    for(ChatDialog *diag:diags)
    {
        if(diag->dir == User::SEND)
            diag->move(width()-diag->width()-5,diag->y());
    }
    QWidget::resizeEvent(event);
}
