#ifndef WGTVIDEO_H
#define WGTVIDEO_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QQueue>
#include <QPainter>
#include <QTimer>
#include <QVector>
#include <QLineEdit>
#include <QTextEdit>
#include <QDebug>
#include <QRadioButton>
#include <QButtonGroup>
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
    int         index,status;
    int         recv_count;
    u8          addr[2];
    bool        opened;

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
    QPushButton *btnSToggle;


    Serial      *serBt;
    Serial      *serCh;
    Serial      *serSend;
    QQueue<u8>  queRecv;
    QQueue<u8>  queSend;
    QByteArray  arr;

    QTimer      *drawTimer;
    QTimer      *fpsTimer;

    QVector<QVector<bool> >
                mat;
    ThdImageSend*thread;
    QRect       rects[WIDTH][HEIGHT];
    u8          ques[WIDTH*HEIGHT/8+4];

    void        genRects(void);

signals:

private:
    inline bool store(u8 ch)
    {
        if(index < WIDTH*HEIGHT)
        {
            for(int i=0;i<8;i++)
            {
                mat[index%WIDTH][index/WIDTH] = ch & 0x80;
                ch <<= 1;
                index ++;
            }
            return true;
        }
        else
        {
            status = 0;
            txtInfo->append("Too much data!\n");
            index = 0;
            status = 0;
            return false;
        }
    }
    void msleep(int msec);

public slots:
    void sltRefresh(void);
    void sltToggle(void);
    void sltClear(void);
    void sltRecv(void);
    void sltSend(void);
    void sltReadBuf(void);
    void sltDirTog(int index, bool b);
    void sltDatTog(int index, bool b);
    void updateBoard();
#ifdef DEBUG
    void fps(void);
#endif
protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QResizeEvent *event);
};

#endif // WGTVIDEO_H
