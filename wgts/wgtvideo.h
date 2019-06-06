#ifndef WGTVIDEO_H
#define WGTVIDEO_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
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

    enum Dir
    {
        Recv = 0,
        Send = 1
    };

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
    Dir         dir;


    QQueue<u8>  queRecv;
    QQueue<u8>  queSend;
    QByteArray  arr;
    QByteArray  tempArr;
    QVector<QVector<bool> >
                mat;
    QRect       rects[WIDTH][HEIGHT];
    u8          ques[WIDTH*HEIGHT/8+4];

    //frames and groups
    QGroupBox   *fraPort;
    QGroupBox   *fraServer;
    QGroupBox   *fraRight;
    QButtonGroup*btgDir;
    QButtonGroup*btgDat;

    //frameOld objects
    QLabel      *labPort;
    QLabel      *labSPort;
    QLabel      *labAddr;
    QLineEdit   *txtAddr;
    QPushButton *btnRefresh;
    QPushButton *btnToggle; //开/关串口
    QPushButton *btnSToggle;
    MyComboBox  *cbxPort;
    MyComboBox  *cbxSPort;
    QRadioButton*rbtDirR;
    QRadioButton*rbtDirS;
    QRadioButton*rbtDatVideo;
    QRadioButton*rbtDatVoice;

    //frameServer objects
    QLabel      *labIP;
    QLabel      *labTcpPort;
    QLineEdit   *txtIP;
    QLineEdit   *txtTcpPort;
    QPushButton *btnListen;

    //right objects
    QLabel      *labFPS;
    QLabel      *labInfo;
    QLineEdit   *txtFPS;
    QTextEdit   *txtInfo;
    QPushButton *btnClear;
    QCheckBox   *chbAuto;

    //other objects
    QLabel      *labPaint;
    QTextEdit   *txtData;

    //non-gui objects
    QTimer      *voiceTimer;
    QTimer      *fpsTimer;

    ThdImageSend*thread;

    Serial      *serBt;
    Serial      *serCh;
    Serial      *serRecv;
    Serial      *serSend;

    QTcpServer  *server;
    QTcpSocket  *client;


signals:

private:

protected:
    void genRects(void);
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
    void sltWriteOver(qint64 bytes);
    void sltDirTog(int index, bool b);
    void sltDatTog(int index, bool b);
    void sltAutoTog(bool b);
    void sltVoice(void);
    void sltLisTog();
    void sltConnected(void);
    void sltDisconnected(void);
    void sltTcpRecv(void);
#ifdef DEBUG
    void fps(void);
#endif
};

#endif // WGTVIDEO_H
