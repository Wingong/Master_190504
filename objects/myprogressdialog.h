#ifndef MYPROGRESSDIALOG_H
#define MYPROGRESSDIALOG_H

#include <QObject>
#include <QDialog>

#include <QLabel>
#include <QProgressBar>
#include <QTimer>

class MyProgressDialog : public QDialog
{
    Q_OBJECT
public:
    MyProgressDialog(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

    QProgressBar    *bar;
    QLabel          *label;
    QLabel          *seconds;

    QTimer          *timer;

    void            setValue(int value);
};

#endif // MYPROGRESSDIALOG_H
