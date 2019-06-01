#ifndef WGTSERIAL_H
#define WGTSERIAL_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <windows.h>
#include <dbt.h>
#include "user.h"
#include "mycombobox.h"
#include "serial.h"

class WgtSerial : public QWidget
{
    Q_OBJECT
public:
    explicit WgtSerial(QWidget *parent = nullptr);
    ~WgtSerial();

    QLabel      *labPort;
    QLabel      *labBaud;
    QLabel      *labStop;
    QLabel      *labExam;
    MyComboBox  *cbxPort;
    MyComboBox  *cbxBaud;
    MyComboBox  *cbxStop;
    MyComboBox  *cbxExam;

    QLabel      *labRecv;
    QLabel      *labSend;
    QRadioButton*rbtRecHex;
    QRadioButton*rbtRecTxt;
    QRadioButton*rbtSenHex;
    QRadioButton*rbtSenTxt;
    QButtonGroup*btgRecv;
    QButtonGroup*btgSend;
    QPushButton *btnRecv;
    QPushButton *btnSend;
    QPlainTextEdit*txtRecv;
    QPlainTextEdit*txtSend;

    QPushButton *btnToggle;
    QPushButton *btnRefresh;
    QPushButton *btnSenMsg;

    Serial      *xTendSerial;


public slots:
    void sltRefresh(void);
    void sltOpen(void);
    void sltClose(void);
    void sltComboClick(int cbx);
    void sltRecClr(void);
    void sltSenClr(void);
    void sltRecTog(int index,bool b);
    void sltSenTog(int index,bool b);
    void sltSenMsg(void);
    void sltReadBuf(void);
    void resizeEvent(QResizeEvent *event);
};

#endif // WGTSERIAL_H
