#ifndef WGTCHAT_H
#define WGTCHAT_H

#include <QObject>
#include <QWidget>
#include <QDateTime>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QQueue>
#include <QTimer>
#include "mycombobox.h"
#include "thdchatrecv.h"
#include "thdchatsend.h"
#include "serial.h"

class WgtChat : public QWidget
{
    Q_OBJECT
public:
    explicit WgtChat(QWidget *parent = nullptr);
    u8 addr[2];

    QTextEdit   *hist;      //历史文本框
    QTextEdit   *edit;      //编辑文本框
    QLabel      *labPort;
    MyComboBox  *cbxPort;
    QPushButton *btnRefresh;
    QPushButton *btnToggle; //开/关串口
    QPushButton *btnSend;
    QPushButton *btnClr;    //清空所有接收区

    QLabel      *labAddr;
    QLineEdit   *txtAddr;


    //Serial        *serRecv;
    Serial      *serSend;   //和无人机连接的串口(Port A)
    QQueue<u8>  queSend;    //Port A发送缓冲队列
    QQueue<u8>  queRecv;    //Port A接收缓冲队列

    ThdChatRecv  *th1;       //发送线程
    ThdChatSend  *th2;       //接收线程
public slots:
    void sltRefresh(void);  //刷新Port A
    void sltRecv(void);     //Port A接收到数据
    void sltToggle(void);   //切换开/关
    void sltSend(void);     //发送按钮
    void sltS(int id);      //发送处理完成
    void sltReadBuf(void);  //Port A读取缓存
    void resizeEvent(QResizeEvent *event);
};

#endif // WGTCHAT_H
