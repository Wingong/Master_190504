#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include <QComboBox>
#include <QString>
#include <QStringList>
#include <QMouseEvent>

class MyComboBox : public QComboBox
{
    Q_OBJECT
signals:
    void clicked(int index);
private:
    QStringList comlist;
protected:
    virtual void mousePressEvent(QMouseEvent *e)
    {
        if(e->button() == Qt::LeftButton)
        {
            emit clicked(index);  //触发clicked信号
        }
        QComboBox::mousePressEvent(e);
    }
public:
    int index = -1;         //多选框序号
    MyComboBox(QWidget *parent)
        : QComboBox(parent)
    { }
    MyComboBox(int init_ind, QWidget *parent)
        : QComboBox(parent),
          index(init_ind)
    { }
    //重载，流式输入
    MyComboBox & operator << (const QString &text)
    { this->addItem(text); comlist << text; return *this; }
    MyComboBox & operator << (const QStringList &textlist)
    { this->addItems(textlist); comlist << textlist; return *this; }
    QStringList & list(void)
    { return comlist; }
    void clear(void)
    { QComboBox::clear(); comlist.clear();}
};

#endif // MYCOMBOBOX_H
