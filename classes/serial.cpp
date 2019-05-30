#include "serial.h"
#include <QDebug>

Serial::Serial()
    : QSerialPort()
{
    name.clear();
    //利用QSerialPortInfo类，遍历所有可用串口的名称
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        qDebug() << info.portName();
        name << info.portName();
    }
    User::stringSort(3,name);
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
    }						//遍历串口名称，并存入名称列表中
    if(name.size() == 0)
    {
        return false;
    }
    User::stringSort(3,name);
    return true;
}
