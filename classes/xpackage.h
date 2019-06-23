#ifndef FRAME_H
#define FRAME_H

#include <QByteArray>
#include <QString>
#include "user.h"

class XPackage
{
public:
#define FLAG_SHIFT_BIT 6
#define TYPE_SHIFT_BIT 4
#define PKGLEN_MASK 0x0F
    enum Flag
    {
        BEG = 1,
        END = 2
    };
    enum Type
    {
        NON = 0,
        IMG = 1,
        VOI = 2,
        DAT = 4
    };
    XPackage();
    bool genForerunner(QByteArray &arr, Type type, int len, int pkglen);
    QByteArray destiny(void){return QByteArray("\xFE\x42");}
    int nextFrame(QByteArray &dest, QByteArray &src);
protected:
    Type    type;
    int     len;
    int     pkglen;
    int     pkgser;
    QByteArray forerunner;
};

#endif // FRAME_H
