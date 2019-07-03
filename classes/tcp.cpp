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

void Tcp::close()
{
    for(QTcpSocket *&socket:socketList)
    {
        socket->close();
        socket->deleteLater();
    }
    socketList.clear();
    server->close();
    server->deleteLater();
    emit cleared();
    qDebug() << "closed";
}

void Tcp::discon(QTcpSocket *socket)
{
    socket->close();
    for(int i=0;i<socketList.size();i++)
    {
        if(socketList[i] == socket)
        {
            socketList.removeAt(i);
            break;
        }
    }
}

bool Tcp::listen(QString &ip, int port)
{
    if(!server->listen(QHostAddress(ip),port))
        return false;
    connect(server,&QTcpServer::newConnection,[=](){
        QTcpSocket *socket = server->nextPendingConnection();
        socketList.push_back(socket);
        connect(socket,&QTcpSocket::disconnected,[=](){
            discon(socket);
            emit disconnected(socket);
        });
        emit newConnection(socket);
    });
    connect(server,&QTcpServer::destroyed,[=](){
        qDebug() << "destroyed";
        server = new QTcpServer(this);
    });
    return true;
}
