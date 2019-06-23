#include "imagelabel.h"

ImageLabel::ImageLabel(QWidget *parent)
    : QLabel(parent)
{
}

ImageLabel::ImageLabel(const QString &text, QWidget *parent)
    : QLabel(text,parent)
{
}

//QSize ImageLabel::sizeHint() const
//{
//    return QSize(0,0);
//}

QSize ImageLabel::minimumSizeHint() const
{
    return QSize(0,0);
}
