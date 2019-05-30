/**
  *********************
  * Serial类头文件     *
  *********************
  */

#ifndef SERIAL_H
#define SERIAL_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QStringList>
#include "user.h"

class Serial : public QSerialPort
{
private:
    //波特率序号和数值对照表
    const int baud[8]={1200,2400,4800,9600,19200,38400,57600,115200};
public:
    //串口名称列表
    QStringList name;

    Serial();
    ~Serial();
    //刷新串口
    bool refresh();
    //设置串口波特率序号
    void setBaudRateIndex(int index)
    { setBaudRate(baud[index]); }
};

#endif // SERIAL_H
