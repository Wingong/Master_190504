#ifndef WGTIMAGEDISP_H
#define WGTIMAGEDISP_H

#include <QWidget>
#include <QResizeEvent>
#include <QString>
#include <QPixmap>
#include "imagelabel.h"

namespace Ui {
class WgtImageDisp;
}

class WgtImageDisp : public QWidget
{
    Q_OBJECT

public:
    explicit WgtImageDisp(QString &dir, QWidget *parent = 0);
    bool refresh(void);
    ~WgtImageDisp();

protected:
    void resizeEvent(QResizeEvent *event);

    QString path;
    QPixmap pm;

private:
    Ui::WgtImageDisp *ui;
};

#endif // WGTIMAGEDISP_H
