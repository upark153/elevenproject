#ifndef WIDGET_H
#define WIDGET_H

#include <QMainWindow>
#include <QWidget>
#include <QTcpSocket>

#include <QTableWidget>
#include <QDebug>
#include <QDataStream>
#include <QList>
#include <QString>
#include <QStandardItem>

//차트 인클루드
#include <QChart>
#include <QChartView>
#include <QBarSet>
#include <QHorizontalBarSeries>
#include <QStringList>
#include <QBarCategoryAxis>
#include <QValueAxis>

#include <QtCore/QPointF>
#include <QAreaSeries>
#include <QAbstractBarSeries>


#include <QPieSeries>
#include <QPieSlice>

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
    void readData();
    void tableChange(); //콤보박스의 인덱스가 바뀔때 tableChange가 실행된다.

//    void setchart(); //차트셋팅

private:
    Ui::Widget *ui;
    QTcpSocket *m_socket;
    bool m_socketReady;

    QStandardItemModel *model = new QStandardItemModel();
    QList<QStandardItem*> items;
    QList<QList<QString>>dbl_chart_items;
    QList<QString> inner_chart_items;

    //차트관련
    QChart *chart = new QChart();
    QChartView *chartView = new QChartView(chart);
//    QBarCategoryAxis *axisX = new QBarCategoryAxis(); //x축(가로) 기준으로 바넣기
//    QValueAxis *axisY = new QValueAxis(); //y축(세로)
    QValueAxis *axisX = new QValueAxis();//x축(가로)
    QBarCategoryAxis *axisY = new QBarCategoryAxis(); //y축 기준으로 바넣기

    QMainWindow *window = new QMainWindow();

    QBarSet *set0;
};
#endif // WIDGET_H
