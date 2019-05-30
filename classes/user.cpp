#include "user.h"
#include <QDebug>

bool User::stringSort(int numIndex,QStringList &src)
{
    QStringList sort(src);
    src.clear();
    QVector<int> index;
    for(QString str:sort)
    {
        int num=str.mid(numIndex).toInt(nullptr,10);
        int i=0;
        for(;i<index.size()&&num>index[i];i++);
        index.insert(i,num);
        src.insert(i,str);
    }
    return true;
}
