#include "wgtnet.h"

WgtNet::WgtNet(QWidget *parent)
    : QWidget(parent),
      frame(new QFrame(this))
{
    frame->setGeometry(10,10,100,100);
    frame->setFrameShape(QFrame::StyledPanel);
}
