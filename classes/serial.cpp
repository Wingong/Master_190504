#include "serial.h"
#include <QDebug>

Serial::Serial()
    : QSerialPort()
{
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        qDebug() << info.portName();
        name << info.portName();
    }
}

Serial::~Serial()
{
    qDebug() << "DESTRUCT\n";
}

bool Serial::refresh()
{
    name.clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        name << info.portName();
    }
    if(name.size() == 0)
    {
        return false;
    }
    return true;
}
