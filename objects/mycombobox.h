#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include <QComboBox>
#include <QString>
#include <QStringList>

class MyComboBox : public QComboBox
{
    Q_OBJECT
private:
    QStringList comlist;
public:
    int index = -1;
    MyComboBox(QWidget *parent)
        : QComboBox(parent)
    { }
    MyComboBox(int init_ind, QWidget *parent)
        : QComboBox(parent),
          index(init_ind)
    { }
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
