#ifndef TCP_H
#define TCP_H

#include <QObject>
#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QString>
#include <QByteArray>

class Tcp : public QObject
{
    Q_OBJECT
public:
    QTcpServer *server;
    Tcp();
    Tcp(QWidget *parent);
    bool listen(QString &ip, int port);
    void close();
    void discon(QTcpSocket *socket);
    QVector<QTcpSocket *>socketList;
signals:
    void newConnection(QTcpSocket *socket);
    void disconnected(QTcpSocket *socket);
    void cleared();
};

#endif // TCP_H
