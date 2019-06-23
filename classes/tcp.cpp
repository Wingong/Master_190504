#include "tcp.h"
#include <QDebug>

Tcp::Tcp()
{
}

Tcp::Tcp(QWidget *parent)
    : QObject(parent),
      server(new QTcpServer(this))
{
}

bool Tcp::listen(QString &ip, int port)
{
    if(!server->listen(QHostAddress(ip),port))
        return false;
    connect(server,&QTcpServer::newConnection,[=](){
        QTcpSocket *socket = server->nextPendingConnection();
        connect(socket,&QTcpSocket::disconnected,[=](){
            socket->close();
            emit disconnected(socket);
        });
        emit newConnection(socket);
    });
    return true;
}
