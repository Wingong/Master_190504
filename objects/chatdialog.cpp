#include "chatdialog.h"
#include <QSizePolicy>
#include <QDebug>
#include "wgtcomm.h"

ChatDialog::ChatDialog(User::Dirs dir, const QString &text, QWidget *parent)
    : QLabel(text,parent),
      dir(dir)
{
    QSizePolicy sp(QSizePolicy::Expanding,QSizePolicy::Expanding);
    sp.setHorizontalStretch(0);
    sp.setVerticalStretch(0);
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
    setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    setWordWrap(true);
    setMargin(10);
    if(dir == User::RECV)
        setStyleSheet("QLabel{border-radius:5px;border:1px solid #d0d0d0;background-color:#f0f0f0}");
    else
        setStyleSheet("QLabel{color:white;border-radius:5px;border:1px solid #d0d0d0;background-color:#55bbff}");
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    adjustSize();
}

ChatDialog::ChatDialog(User::Dirs dir, QString &path, const char *format, QWidget *parent)
    : QLabel(parent),
      dir(dir),
      path(path)
{
    setAlignment(Qt::AlignCenter);
    QImage image;
    image.load(path,format);
    QPixmap pm = QPixmap::fromImage(image);
    if(pm.width()>pm.height())
        pm = pm.scaled(200,200,Qt::KeepAspectRatioByExpanding);
    else
        pm = pm.scaled(200,200,Qt::KeepAspectRatio);
    setPixmap(pm);
    qDebug() << pm.size();
    if(dir == User::RECV)
        setStyleSheet("QLabel{border-radius:4px;border:1px solid #d0d0d0;background-color:#f0f0f0}");
    else
        setStyleSheet("QLabel{color:white;border-radius:4px;border:1px solid #d0d0d0;background-color:#55bbff}");
    resize(pm.width()+8,pm.height()+8);
    qDebug() << size();
    connect(this,SIGNAL(dblclick(QString&)),qobject_cast<WgtComm*>(parent->parent()->parent()->parent()),SLOT(on_ImaClicked(QString&)));
}

ChatDialog::ChatDialog(User::Dirs dir, QByteArray &image, const char *format, QWidget *parent)
    : QLabel(parent),
      dir(dir)
{
    //setAlignment(Qt::AlignCenter);
    //QImage image;
    //image.load(path,format);
    //QPixmap pm = QPixmap::fromImage(image);
    //if(pm.width()>pm.height())
    //    pm = pm.scaled(200,200,Qt::KeepAspectRatioByExpanding);
    //else
    //    pm = pm.scaled(200,200,Qt::KeepAspectRatio);
    //setPixmap(pm);
    //qDebug() << pm.size();
    //if(dir == User::RECV)
    //    setStyleSheet("QLabel{border-radius:4px;border:1px solid #d0d0d0;background-color:#f0f0f0}");
    //else
    //    setStyleSheet("QLabel{color:white;border-radius:4px;border:1px solid #d0d0d0;background-color:#55bbff}");
    //resize(pm.width()+8,pm.height()+8);
    //qDebug() << size();
    //connect(this,SIGNAL(dblclick(QString&)),qobject_cast<WgtComm*>(parent->parent()->parent()->parent()),SLOT(on_ImaClicked(QString&)));
}

void ChatDialog::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(pixmap()!=nullptr)
        emit dblclick(path);
    QLabel::mouseDoubleClickEvent(event);
}

//QSize ChatDialog::sizeHint() const
//{
//    return QSize(50,50);
//}
