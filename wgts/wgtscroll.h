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

private:
    char qa1[54] = {22,7,5,18,13,4,2,3,5,1,1,20,6,14,2,4,15,12,15,3,4,3,13,21,9,13,18,4,12,19,21,14,15,7,9,11,8,19,20,15,21,0,1,14,7,21,19,7,9,21,17,16,7,16,};
    char qa2[54] = {43,13,65,-1,-52,-31,4,5,71,-8,-9,-11,28,-72,8,7,-69,84,3,-4,-76,4,27,15,6,-52,-8,3,82,-11,1,7,16,11,-78,-12,0,0,0,-69,17,42,-14,-3,10,-68,2,0,-9,-68,-4,75,17,1,};
};

#endif // WGTSCROLL_H
