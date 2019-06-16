#ifndef THDIMAGEDISP_H
#define THDIMAGEDISP_H

#include <QThread>
#include <QObject>
#include <QByteArray>
#include <QImage>
#include <QPixmap>
#include <QLabel>

class ThdImageDisp : public QThread
{
    Q_OBJECT
public:
    bool        resize = false;
    QByteArray  arr;
    QLabel      *labPaint;
    QImage      image;
    QPixmap     pixmap;
    ThdImageDisp(QObject *parent);
    void run();
};

#endif // THDIMAGEDISP_H
