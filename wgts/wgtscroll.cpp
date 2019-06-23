#include "wgtscroll.h"
#include <QDebug>
#include <QPalette>

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
