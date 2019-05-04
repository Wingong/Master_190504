#include "mysettings.h"

MySettings::MySettings(QString setpat = "settings.ini")
    : QSettings(setpat, QSettings::IniFormat)
{
    setValue("Main/Index", "0");
    setValue("Main/Width", "530");
    setValue("Main/Height", "400");
    setValue("Serial/Baud", "115200");
    setValue("Serial/StopBit", "1");
}
