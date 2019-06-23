#include "wgtimagedisp.h"
#include "ui_wgtimagedisp.h"

#include <QDebug>
#include <QMessageBox>
#include <QImage>

WgtImageDisp::WgtImageDisp(QString &dir, QWidget *parent)
    : QWidget(parent),
      path(dir),
      ui(new Ui::WgtImageDisp)
{
    ui->setupUi(this);
    QImage image;
    image.load(path);
    pm = QPixmap::fromImage(image);
    ui->label->setPixmap(pm);
    ui->label->setScaledContents(true);
    move(200,200);
    resize(pm.width()+24,pm.height()+59);
    //ui->label->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    qDebug() << ui->label->sizeHint();
    qDebug() << ui->label->size();
    qDebug() << ui->label->pixmap()->size();
    qDebug() << pm.size();
    qDebug() << size();
}

WgtImageDisp::~WgtImageDisp()
{
    delete ui;
}

void WgtImageDisp::resizeEvent(QResizeEvent *event)
{
    ui->labRatio->setText(tr("%1\%, %2\%").arg(ui->label->width()*100/pm.width()).arg(ui->label->height()*100/pm.height()));
    QWidget::resizeEvent(event);
}

bool WgtImageDisp::refresh()
{
    return false;
}
