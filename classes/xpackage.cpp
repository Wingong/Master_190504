#include "xpackage.h"

XPackage::XPackage()
{}

bool XPackage::genForerunner(QByteArray &arr, Type type, int len, int pkglen)
{
    if(pkglen > 800)
        return false;
    this->pkglen = pkglen;
    this->len = len;
    this->type = type;
    u8 flag = 1<<FLAG_SHIFT_BIT<<BEG;
    flag |= 1<<FLAG_SHIFT_BIT<<this->type;
    flag |= ((u8)(this->pkglen >> 8));
    arr = "\xFE";
    //arr.append(0xFE);
    arr.append(flag);
    arr.append((u8)(this->pkglen & 0xFF));
    arr.append((u8)(this->len >> 24));
    arr.append((u8)(this->len >> 16));
    arr.append((u8)(this->len >> 8));
    arr.append((u8)(this->len));
    return true;

}
