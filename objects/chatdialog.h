#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QByteArray>
#include <QImage>
#include <QMouseEvent>
#include <QPixmap>
#include "user.h"

class ChatDialog : public QLabel
{
    Q_OBJECT
signals:
    void dblclick(QString &path);
public:
    ChatDialog(const QString &text, QWidget *parent);
    ChatDialog(User::Dirs dir,const QString &text, QWidget *parent);
    ChatDialog(User::Dirs dir, QString &path, const char *format, QWidget *parent);
    ChatDialog(User::Dirs dir, QByteArray &image, const char *format, QWidget *parent);

    User::Dirs dir;
    QString path;
    QPixmap pm;

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    //QSize sizeHint() const;
};

#endif // ChatDialog_H
