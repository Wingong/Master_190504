#ifndef WGTVIDEO_H
#define WGTVIDEO_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QQueue>
#include <QPainter>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QLineEdit>
#include <QTextEdit>
#include <QDebug>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QByteArray>

#include "user.h"
#include "mycombobox.h"
#include "serial.h"
#include "thdimagesend.h"

class WgtVideo : public QWidget
{
    Q_OBJECT
public:
    explicit WgtVideo(QWidget *parent = nullptr);
    ~WgtVideo();

    qreal       dw,dh;
    int         tx;
    int         cnt;
    int         cntv;
    int         index,status;
    bool        auto_dir;
    int         auto_dat;
    int         recv_count;
    u8          addr[2];
    bool        opened;
    bool        automode;

    QLabel      *labPort;
    QLabel      *labPaint;
    QLabel      *labFPS;
    QLabel      *labInfo;
    QLineEdit   *txtFPS;
    QTextEdit   *txtInfo;
    QTextEdit   *txtData;
    MyComboBox  *cbxPort;
    QPushButton *btnRefresh;
    QPushButton *btnToggle; //开/关串口
    QPushButton *btnClear;

    QLabel      *labSPort;
    QLabel      *labAddr;
    QLineEdit   *txtAddr;
    MyComboBox  *cbxSPort;
    QRadioButton*rbtDirR;
    QRadioButton*rbtDirS;
    QRadioButton*rbtDatVideo;
    QRadioButton*rbtDatVoice;
    QButtonGroup*btgDir;
    QButtonGroup*btgDat;
    QCheckBox   *chbAuto;
    QPushButton *btnSToggle;

    QFrame      *fraOld;
    QFrame      *fraServer;

    QLabel      *labIP;
    QLabel      *labTcpPort;
    QLineEdit   *txtIP;
    QLineEdit   *txtTcpPort;
    QPushButton *btnListen;

    Serial      *serBt;
    Serial      *serCh;
    Serial      *serRecv;
    Serial      *serSend;
    QQueue<u8>  queRecv;
    QQueue<u8>  queSend;
    QByteArray  arr;
    QByteArray  tempArr;

    QTimer      *voiceTimer;
    QTimer      *fpsTimer;

    QTcpServer  *server;
    QTcpSocket  *client;

    QVector<QVector<bool> >
                mat;
    ThdImageSend*thread;
    QRect       rects[WIDTH][HEIGHT];
    u8          ques[WIDTH*HEIGHT/8+4];

    void        genRects(void);

signals:

private:

protected:
    void funRecv(void);
    void updateBoard();
    bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QResizeEvent *event);
public slots:
    void sltRefresh(void);
    void sltToggle(void);
    void sltClear(void);
    void sltSend(void);
    void sltReadBuf(void);
    void sltAutoRead(void);
    void sltDirTog(int index, bool b);
    void sltDatTog(int index, bool b);
    void sltAutoTog(bool b);
    void sltVoice(void);
    void sltLisTog(bool b);
    void sltConnected(void);
    void sltDisconnected(void);
    void sltTcpRecv(void);
#ifdef DEBUG
    void fps(void);
#endif
};

#endif // WGTVIDEO_H
