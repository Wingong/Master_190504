/**
  *********************
  * MySettings类头文件 *
  *********************
  */


#ifndef MYSETTINGS_H
#define MYSETTINGS_H

#include <QString>
#include <QSettings>

enum Indexs
{
    UpperComputer,
    SerialAssistant
};

enum BaudIndexs
{
    BI4800,BI9600,BI14400,BI19200,BI28800,BI38400,BI57600,BI115200,BI230400,BI460800
};

enum StopBits
{
    SI1, SI1_5, SI2
};

enum ExamBits
{
    EINull, EIOdd, EIEven, EIZero, EIOne
};

class MySettings : public QSettings
{
private:
    Indexs		index;
    int			port;
    BaudIndexs	baudIndex;
    StopBits	stopBit;
    ExamBits	examBit;
    QString		path;
public:
    MySettings(QString setpat);
};

#endif // MYSETTINGS_H
