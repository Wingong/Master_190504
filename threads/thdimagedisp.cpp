#include "thdimagedisp.h"
#include <QDebug>

ThdImageDisp::ThdImageDisp(QObject *parent)
    : QThread(parent)
{
}

void ThdImageDisp::run()
{
    if(resize == false)
        qDebug() << "IMG read from data: " << image.loadFromData(arr,"JPG");
    pixmap = QPixmap::fromImage(image);
    pixmap = pixmap.scaled(labPaint->size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    qDebug() << "IMG size: " << pixmap.size();
    labPaint->setPixmap(pixmap);
}
