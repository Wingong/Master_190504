#ifndef SERIAL_H
#define SERIAL_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QStringList>

class Serial : public QSerialPort
{
private:
    const int baud[8]={1200,2400,4800,9600,19200,38400,57600,115200};
public:
    QStringList name;

    Serial();
    ~Serial();
    bool refresh();
    void setBaudRateIndex(int index)
    { setBaudRate(baud[index]); }
};

#endif // SERIAL_H
