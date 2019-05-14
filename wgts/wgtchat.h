#ifndef WGTCHAT_H
#define WGTCHAT_H

#include <QObject>
#include <QWidget>
#include <QDateTime>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QQueue>
#include <QTimer>
#include "mycombobox.h"
#include "recvthread.h"
#include "sendthread.h"
#include "serial.h"

class WgtChat : public QWidget
{
    Q_OBJECT
public:
    explicit WgtChat(QWidget *parent = nullptr);

    QTextEdit	*hist;
    QTextEdit	*edit;
    QLabel		*labPort;
    MyComboBox	*cbxPort;
    QPushButton	*btnRefresh;
    QPushButton	*btnToggle;
    QPushButton	*btnSend;
    QPushButton	*btnClr;


    //Serial		*serRecv;
    Serial		*serSend;
    QQueue<u8>	queSend;
    QQueue<u8>	queRecv;

    RecvThread	*th1;
    SendThread	*th2;
public slots:
    void sltRefresh(void);
    void sltRecv(void);
    void sltToggle(void);
    void sltSend(void);
    void sltS(int id);
    void sltReadBuf(void);
    void resizeEvent(QResizeEvent *event);
};

#endif // WGTCHAT_H
