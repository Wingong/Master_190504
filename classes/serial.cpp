#include "serial.h"
#include <QDebug>

Serial::Serial()
    : QSerialPort()
{
    //利用QSerialPortInfo类，遍历所有可用串口的名称
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
    name.clear();			//清空串口名列表
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        name << info.portName();
    }						//遍历串口名称，并存入名称列表中
    if(name.size() == 0)
    {
        return false;
    }
    return true;
}
