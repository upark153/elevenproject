#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void socketReadey();
    void stateChanged(QAbstractSocket::SocketState socketState);

    void on_lineEdit_textChanged(const QString &newtext);

private:
    Ui::Widget *ui;
    QTcpSocket *m_socket;
    bool m_socketReady;
};
#endif // WIDGET_H
