#ifndef WGTSCROLL_H
#define WGTSCROLL_H

#include <QString>
#include <QVector>
#include <QLabel>
#include "chatdialog.h"

#include <QObject>
#include <QWidget>
#include "user.h"

class WgtScroll : public QWidget
{
    Q_OBJECT
public:
    explicit WgtScroll(QWidget *parent = nullptr);

    void newRecord(User::Dirs dir, const QString &str);
    void newRecord(User::Dirs dir, QString &path, const char *format);

protected:
    QVector<ChatDialog *>diags;
    void resizeEvent(QResizeEvent *event);

signals:

public slots:
};

#endif // WGTSCROLL_H
