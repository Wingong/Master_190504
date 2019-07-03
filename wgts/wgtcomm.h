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
#include "thdimagesend.h"

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
    enum Mode
    {
        NON = 0,
        IMG,
        VOI,
        DAT
    };

    explicit WgtComm(QWidget *parent = 0);
    ~WgtComm();
    bool loadSettings(QSettings &settings);
    bool writeSettings(QSettings &settings);
    void refresh(int port);

protected:
    void resizeEvent(QResizeEvent *event);
    void sendIMG(QByteArray &arr);

private slots:
    void on_CbxClicked(int index);
    void on_ImaClicked(QString &path);
    void on_UToggle(void);
    void on_TToggle(void);
    void on_LToggle(void);
    void on_Connected(void);
    void on_Disconnected(void);
    void on_TcpRefreshed(void);
    void on_TcpReadyRead(void);

private:
    QRegExp         rxIP = QRegExp("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
    QRegExp         rxTerminalHeader = QRegExp("OK(.)\\r\\n");
    QByteArray      jpgend = "\xFF\xD9";

    QByteArray      tcpBuffer;
    QByteArray      uartBuffer;
    QByteArray      voiceBuffer;

    QTcpSocket      *currentSocket = nullptr;
    Serial          *serU;
    Serial          *serL;
    XPackage        package;
    Tcp             *tcp;

    ThdImageDisp    *thImageDisp;
    ThdImageSend    *thImageSend;
    XPackage::Type  mode = XPackage::NON;
    Mode            uartMode = NON;
    Ui::WgtComm *ui;

    int             imgSize;
};

#endif // WGTCOMM_H
