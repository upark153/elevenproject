#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QDataStream>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("Server | Data receiver");

    m_socket = nullptr;

    m_server.listen(QHostAddress::LocalHost, 5050);

    connect(&m_server,&QTcpServer::newConnection,this,&Widget::gotConnection);


}

Widget::~Widget()
{
    if (m_socket)
    {
        qDebug() << "Closing socket : " << (m_socket==nullptr);
        m_socket -> close();
        m_socket -> deleteLater();
    }
    m_server.close();
    delete ui;
}

void Widget::gotConnection()
{
    qDebug() << "Server got new connection";
    m_socket = m_server.nextPendingConnection();
    connect(m_socket,QTcpSocket::readyRead,this,&Widget::readData);
}

void Widget::readData()
{
    QDataStream in(m_socket);

    in.startTransaction(); // 전체 데이터를 가질 때까지 기다렸다가 한번에 읽고 모든 것을 읽을 수 있게 해준다.

    QString recvString;
    in >> recvString;

    if(!in.commitTransaction())
    {
        return; // wait for more data
    }

    ui->textEdit->append(recvString);
}

