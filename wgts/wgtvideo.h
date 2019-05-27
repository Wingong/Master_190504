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

#include "user.h"
#include "mycombobox.h"
#include "serial.h"
#include "testthread.h"

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
    bool        opened;

    QLabel      *labPort;
    QLabel      *labPaint;
    QLabel      *labFPS;
    QLabel      *labInfo;
    QLineEdit   *txtFPS;
    QTextEdit   *txtInfo;
    MyComboBox  *cbxPort;
    QPushButton *btnRefresh;
    QPushButton *btnToggle; //开/关串口
    QPushButton *btnClear;

    Serial      *serRecv;
    Serial      *serSend;
    QQueue<u8>  queRecv;
    QQueue<u8>  queSend;

    QTimer      *drawTimer;
    QTimer      *fpsTimer;

    QVector<QVector<bool> >
                mat;
    TestThread  *thread;
    QRect       rects[WIDTH][HEIGHT];

    void        genRects(void);

signals:
    void        readOK();

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

public slots:
    void sltRefresh(void);
    void sltToggle(void);
    void sltClear(void);
    void sltRecv(void);
    void sltSend(void);
    void sltReadBuf(void);
    void updateBoard();
#ifdef DEBUG
    void fps(void);
#endif
protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QResizeEvent *event);
};

#endif // WGTVIDEO_H
