#ifndef WGTSETTINGS_H
#define WGTSETTINGS_H

#include <QResizeEvent>
#include <QObject>
#include <QWidget>
#include <QTabWidget>
#include <QTreeWidget>
#include <QFrame>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QVector>
#include <QByteArray>
#include <QString>
#include <QRegExp>
#include "myprogressdialog.h"
#include "mycombobox.h"
#include "serial.h"

#define CMDCNT 6

class WgtSettings : public QWidget
{
    Q_OBJECT
public:
    explicit WgtSettings(QWidget *parent = nullptr);

    const QVector<bool>
                    boolER={true,false,false,true,true,true};
    const QVector<bool>
                    boolEW={false,true,true,true,true,true};
    const QStringList
                    strItem={"AT测试","复位","波特率","模式","WiFi","IP"};
    const QStringList
                    strHint={"AT",
                             "AT+RST",
                             "AT+UART=<baudrate>,<databits>,<stopbits>,<parity>,<flow control>",
                             "AT+CWMODE=<mode>",
                             "AT+CWJAP=<ssid>,<pwd>",
                             "AT+CIFSR?"};
    const QStringList
                    strRead={"AT\r\n",
                             "",
                             "",
                             "AT+CWMODE?\r\n",
                             "AT+CWLAP\r\n",
                             "AT+CIFSR\r\n"};

    const QStringList
                    strWrite={"AT\r\n",
                             "AT+RST\r\n",
                             "AT+UART=%1,%2,%3,%4,%5\r\n",
                             "AT+CWMODE=%1\r\n",
                             "AT+CWJAP=\"%1\",\"%2\"\r\n",
                             "AT+CIPSTA=\r\n"};
    QRegExp rx = QRegExp("([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}):([0-9]{1,5}),([0-1])");
    QByteArray      buffer;

    int             cmd = -1;

    QTabWidget      *tabAT;
    QTreeWidget     *treeESPAT;
    QTreeWidget     *treeXTENDAT;
    MyProgressDialog*dialog;
    QGroupBox       *groupPort;

    QVector<QTreeWidgetItem *>
                    itemESP;
    QVector<QPushButton *>
                    btnER;
    QVector<QPushButton *>
                    btnEW;
    MyComboBox      *cbxUART;
    MyComboBox      *cbxCWMODE;
    MyComboBox      *cbxCWJAP;
    QLineEdit       *txtCIFSR;
    //QPushButton     *test;

    QLabel          *labPort;
    QLabel          *labBaud;
    QLabel          *labStop;
    QLabel          *labExam;
    MyComboBox      *cbxPort;
    MyComboBox      *cbxBaud;
    MyComboBox      *cbxStop;
    MyComboBox      *cbxExam;
    QPushButton     *btnRefresh;
    QPushButton     *btnToggle;

    //QLineEdit       *regexp;

    QTextEdit       *hist;
    QTextEdit       *tose;
    QPushButton     *btnOnePunch;
    QPushButton     *btnSend;

    Serial          *serial;
    QTcpServer      *server;
    QTcpSocket      *socket;

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void next();

public slots:
    void sltRead(void);
    void sltWrite(void);
    void sltRefresh(void);
    void sltToggle(void);
    void sltCbxTog(void);
    void sltSerialRead(void);
    void sltCancel(void);
    void sltTest(void);
};

#endif // WGTSETTINGS_H
