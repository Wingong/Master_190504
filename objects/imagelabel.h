#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QObject>
#include <QWidget>
#include <QLabel>

class ImageLabel : public QLabel
{
public:
    ImageLabel(QWidget *parent);
    ImageLabel(const QString &text, QWidget *parent);
    //QSize sizeHint() const;
    QSize minimumSizeHint() const;
};

#endif // IMAGELABEL_H
