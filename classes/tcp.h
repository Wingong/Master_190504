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
signals:
    void newConnection(QTcpSocket *socket);
    void disconnected(QTcpSocket *socket);
};

#endif // TCP_H
