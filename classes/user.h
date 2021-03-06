#ifndef USER_H
#define USER_H

#define HEX(X) ((X)<10?(X)+48:(X)+55)
#define DEBUG
#define HEIGHT 120
#define WIDTH  160
#define BTBAUD 115200
#define CHBAUD 230400

#include <QString>
#include <QVector>
#include <QStringList>

typedef unsigned char u8;

namespace user{
class User;
}

class User
{
private:
public:

    enum Cmds
    {
        BEG = 0xff,
        END = 0xfe,
        ACK = 0xfd,
        ERR = 0xfc
    };
    enum Status
    {
        IDL = -1,
        REV = 0,
        LEN = 1,
        RID = 2,
        XOR = 3,
        AOP = 5,
        EOP = 7,
        TRS = 0xf0
    };

    enum Dirs
    {
        RECV = 0,
        SEND = 1
    };

    static bool stringSort(int numIndex, QStringList &src);
};

#endif // USER_H
