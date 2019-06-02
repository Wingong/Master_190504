#ifndef WGTNET_H
#define WGTNET_H

#include <QObject>
#include <QWidget>
#include <QFrame>

class WgtNet : public QWidget
{
    Q_OBJECT
public:
    explicit WgtNet(QWidget *parent = nullptr);

    QFrame      *frame;

signals:

public slots:
};

#endif // WGTNET_H
