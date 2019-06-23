#ifndef WGTCOMM_H
#define WGTCOMM_H

#include <QWidget>

#include <QTcpServer>
#include <QTcpSocket>
#include <QRegExp>

#include <QSettings>

#include "wgtimagedisp.h"
#include "mycombobox.h"
#include "serial.h"
#include "xpackage.h"
#include "tcp.h"

#include "thdimagedisp.h"

namespace Ui {
class WgtComm;
}

class WgtComm : public QWidget
{
    Q_OBJECT

public:
    enum Port
    {
        X = 1,
        T = 2,
        L = 4
    };

    explicit WgtComm(QWidget *parent = 0);
    ~WgtComm();
    bool loadSettings(QSettings &settings);
    bool writeSettings(QSettings &settings);
    void refresh(int port);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void on_CbxClicked(int index);
    void on_ImaClicked(QString &path);
    void on_UToggle(void);
    void on_TToggle(void);
    void on_LToggle(void);
    void on_Connected(void);
    void on_Disconnected(void);

private:
    QRegExp         rxIP = QRegExp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
    QRegExp         rxTerminalHeader = QRegExp("OK(.)\\r\\n");
    QByteArray      jpgend = "\xFF\xD9";

    QByteArray      tcpBuffer;

    QTcpServer      *server;
    QTcpSocket      *socket = nullptr;
    Serial          *serU;
    Serial          *serL;
    XPackage        package;
    Tcp             *tcp;

    ThdImageDisp    *thImageDisp;
    XPackage::Type  mode = XPackage::NON;
    Ui::WgtComm *ui;
};

#endif // WGTCOMM_H
