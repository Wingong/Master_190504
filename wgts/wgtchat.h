#ifndef WGTCHAT_H
#define WGTCHAT_H

#include <QObject>
#include <QWidget>
#include <QDateTime>
#include <QTextEdit>
#include <QPushButton>
#include "serial.h"

class WgtChat : public QWidget
{
    Q_OBJECT
public:
    explicit WgtChat(QWidget *parent = nullptr);

    QTextEdit	*hist;
    QTextEdit	*edit;
    QPushButton	*send;

    Serial		*serRecv;
    Serial		*serSend;

public slots:
    void sltSend(void);
    void resizeEvent(QResizeEvent *event);
};

#endif // WGTCHAT_H
