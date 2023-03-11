#include "widget.h"
#include "ui_widget.h"
#include <stdio.h>
#include <iostream>




Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget),
      m_socket(new QTcpSocket(this)), //소켓 생성
      m_socketReady(false) //소켓 대기 false
{
    ui->setupUi(this);
    ui->comboBox->setCurrentIndex(0);

    m_socket -> connectToHost("localhost", 5050);

    connect(m_socket,QTcpSocket::readyRead,this,&Widget::readData);
    connect(m_socket,&QTcpSocket::connected,this,&Widget::socketReadey);
    connect(m_socket,&QTcpSocket::stateChanged,this,&Widget::stateChanged);

    //콤보박스의 인덱스가 바뀔때 tableChange가 실행된다.
    connect(ui->comboBox,QComboBox::currentIndexChanged,this,&Widget::tableChange);
    ui->all_people_graph->addWidget(chartView); //ui에 차트넣음

}

Widget::~Widget()
{
    m_socket -> close();
    delete ui;
}

//소켓 대기
void Widget::socketReadey()
{
    m_socketReady = true;
}
//소켓 상태
void Widget::stateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "소켓상태확인->" <<socketState;
    //QAbstractSocket::ConnectingState->소켓이 연결중인 상태다.
    //QAbstractSocket::UnconnectedState->소켓이 연결되지 않은 상태
    //QAbstractSocket::ConnectedState->연결된상태
}

//송신
void Widget::tableChange()//콤보박스의 인덱스가 바뀔때 tableChange가 실행된다.
{
    if (ui->comboBox->currentIndex()!=0)
    {
        window->close();
    }

    dbl_chart_items.clear();
    if(m_socketReady)
    {
        QString str = ui->comboBox->currentText();
        QDataStream out(m_socket);
        out << str;
    }
}

//수신
void Widget::readData()
{
    QList<QList<QString>> doubl_data_list; //이중리스트 선언
    QString code;
    int h_len;   //행갯수
    int c_len;   //칼럼갯수

    QDataStream in(m_socket);
    in.startTransaction(); // 전체 데이터를 가질 때까지 기다렸다가 한번에 읽고 모든 것을 읽을 수 있게 해준다.

    QString recvString;
    in >> recvString;

    if(!in.commitTransaction())
    {
        return; // wait for more data
    }

    QStringList data_list = recvString.split(";");
    code = data_list[0];
    h_len = data_list[1].toInt();
    c_len = data_list[2].toInt();
    data_list.removeAt(0);
    data_list.removeAt(0);
    data_list.removeAt(0);
    int list_count =0;

    // 인구 점수
    if(code == "광산구 전체 인구"){
        for (int i = 0; i < h_len; i++) {
                QList<QString> inner_list;
                for (int j = 0; j < c_len; j++) {
                    inner_list.append(data_list[list_count]);
                    list_count++;
                }
                doubl_data_list.append(inner_list);
            }

        qDebug() << code;
        qDebug() << doubl_data_list; //이중리스트

        ui->tableView->clearSpans(); //유아지우는거
        model->clear();//데이터지우는거

        for(int i = 0; i < h_len; i++){ //받은 데이터 이중리스트 비교해서 items 리스트에 값 넣는 반복문
            items.append(new QStandardItem(doubl_data_list[i][0])); //ex)계
            items.append(new QStandardItem(doubl_data_list[i][1])); //ex)399669
            items.append(new QStandardItem(doubl_data_list[i][4])); //ex)170895
            if(i > 0){
                double score = (doubl_data_list[i][1].toDouble() / doubl_data_list[0][1].toDouble() * 100) / (doubl_data_list[i][4].toDouble() / doubl_data_list[0][4].toDouble() * 100);
                items.append(new QStandardItem(QString::number(score).left(3))); //ex)점수 결과

                //차트에 넣을 아이템 생성(이중 리스트로)
                inner_chart_items.append(doubl_data_list[i][0]); //계 다음부터
                inner_chart_items.append(QString::number(score).left(3));
                dbl_chart_items.append(inner_chart_items);
                inner_chart_items.removeAt(0);
                inner_chart_items.removeAt(0);

            }
            //테이블 모델행에 아이템 넣기
            model->appendRow(items);
            items.clear();
        }
        model->setHeaderData(0, Qt::Horizontal, QObject::tr("동"));
        model->setHeaderData(1, Qt::Horizontal, QObject::tr("인구 합계"));
        model->setHeaderData(2, Qt::Horizontal, QObject::tr("세대수"));
        model->setHeaderData(3, Qt::Horizontal, QObject::tr("인구 점수"));

        ui->tableView->setModel(model);
        ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //헤더 크기 조절



    //광산구 전체 인구 -> 차트 셋팅 시작//
        chart->removeAllSeries();
        chart->setTitle("");
        axisY->clear();


        qDebug()<<"dbl_chart_items           "<<dbl_chart_items;
        qDebug()<<"더블차트아이템len"<<dbl_chart_items.size();

        QBarSet *set0 = new QBarSet("인구 점수");
        QHorizontalBarSeries *series = new QHorizontalBarSeries();
        chart->addSeries(series); //두번적으면 "Can not add series. Series already on the chart."

        //차트 카테고리(y축)과 데이터 지정하기
        QStringList categories;
        for (int i=dbl_chart_items.length()-1; i>=0; i--){
            categories << dbl_chart_items[i][0];

           QString setdata = dbl_chart_items[i][1];
           *set0 << setdata.toFloat();

        }
           series->append(set0);

        //데이터레이블, 범례 켜기/위치 지정, 차트제목 설정
        series->setLabelsVisible(true); //데이터레이블 켜기
        series->setLabelsPosition(QHorizontalBarSeries::LabelsOutsideEnd);
        chart->legend()->setVisible(true); //범례켜기
        chart->legend()->setAlignment(Qt::AlignBottom); //범례위치
        chart->setTitle("광산구 동별 인구 점수"); //차트제목

        //차트애니메이션 설정
        chart->setAnimationOptions(QChart::SeriesAnimations); //차트애니메이션
        chartView->setRenderHint(QPainter::Antialiasing);
        //차트테마설정
        chartView->chart()->setTheme(QChart::ChartThemeBlueCerulean);
        // 차트 폰트 설정
        QFont titleFont("맑은 고딕", 18, QFont::Bold);
        chart->setTitleFont(titleFont);

        QFont asixXFont("맑은 고딕", 14);
        chart->legend()->setFont(asixXFont);
        axisX->setLabelsFont(asixXFont);

        QFont asixYFont("맑은 고딕", 11, QFont::Bold);
        axisY->setLabelsFont(asixYFont);

        set0->setLabelFont(asixYFont);

        // 차트 데이터 레이블 색 설정
//        QColor barColor1(189,234,224); // 민트색으로 지정
//        set0->setColor(barColor1);
        set0->setLabelColor(Qt::gray);

        //y에 동이름넣기
        axisY->append(categories);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        //x에 어쩌고저쩌고
        chart->addAxis(axisX, Qt::AlignBottom);
        axisX->setRange(0, 1.1);
        series->attachAxis(axisX);
        axisX->setLabelFormat("%.1f");
//        axisX->applyNiceNumbers(); //범위를 자연스럽게 조정하는 매서드

        chartView->resize(300,200);
//        chartView->update();
        //차트 새로운 창에서 뜨게하는거
        window->setCentralWidget(chartView); //chartView 가운데로
        window->resize(800, 800);
        window->move(1000,100);
        window->show();

//        dbl_chart_items.clear();

    }

    // 혼합 시설 점수
    else if(code == "혼합 시설"){
        for (int i = 0; i < h_len; i++) {
                QList<QString> inner_list;
                for (int j = 0; j < c_len; j++) {
                    inner_list.append(data_list[list_count]);
                    list_count++;
                }
                doubl_data_list.append(inner_list);
            }

        qDebug() << code;
        qDebug() << doubl_data_list;

        ui->tableView->clearSpans();
        model->clear();

        for(int i = 0; i < h_len; i++){
            items.append(new QStandardItem(doubl_data_list[i][0]));
            items.append(new QStandardItem(doubl_data_list[i][1]));
            items.append(new QStandardItem(doubl_data_list[i][2]));
            items.append(new QStandardItem(doubl_data_list[i][3]));
            items.append(new QStandardItem(doubl_data_list[i][4]));
            if(i > 0){
                double score = ((doubl_data_list[i][1].toDouble() + doubl_data_list[i][2].toDouble() + doubl_data_list[i][3].toDouble() + doubl_data_list[i][4].toDouble())
                                / 1992.0 * 100.0)
                                / (1992.0 / 13771.0 * 100.0);
                items.append(new QStandardItem(QString::number(score).left(3)));

                //차트에 넣을 아이템 생성(이중 리스트로)
                inner_chart_items.append(doubl_data_list[i][0]); //계 다음부터
                inner_chart_items.append(QString::number(score).left(3));
                dbl_chart_items.append(inner_chart_items);
                inner_chart_items.removeAt(0);
                inner_chart_items.removeAt(0);
            }
            model->appendRow(items);
            items.clear();
        }

        model->setHeaderData(0, Qt::Horizontal, QObject::tr("동"));
        model->setHeaderData(1, Qt::Horizontal, QObject::tr("복지 시설"));
        model->setHeaderData(2, Qt::Horizontal, QObject::tr("의료 시설"));
        model->setHeaderData(3, Qt::Horizontal, QObject::tr("교육 시설"));
        model->setHeaderData(4, Qt::Horizontal, QObject::tr("주거 시설"));
        model->setHeaderData(5, Qt::Horizontal, QObject::tr("혼합 시설 점수"));

        ui->tableView->setModel(model);
        ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        //광산구 혼합시설 -> 차트 셋팅 시작//
            chart->removeAllSeries();
            chart->setTitle("");
            axisY->clear();

            qDebug()<<"dbl_chart_items           "<<dbl_chart_items;
            qDebug()<<"더블차트아이템len"<<dbl_chart_items.size();

            QBarSet *set0 = new QBarSet("시설 점수");
            QHorizontalBarSeries *series = new QHorizontalBarSeries();
            chart->addSeries(series); //두번적으면 "Can not add series. Series already on the chart."

            //차트 카테고리(y축)과 데이터 지정하기
            QStringList categories;
            for (int i=dbl_chart_items.length()-1; i>=0; i--){
                categories << dbl_chart_items[i][0];

               QString setdata = dbl_chart_items[i][1];
               *set0 << setdata.toFloat();

            }
               series->append(set0);

            //데이터레이블, 범례 켜기/위치 지정, 차트제목 설정
            series->setLabelsVisible(true); //데이터레이블 켜기
            series->setLabelsPosition(QHorizontalBarSeries::LabelsOutsideEnd);
            chart->legend()->setVisible(true); //범례켜기
            chart->legend()->setAlignment(Qt::AlignBottom); //범례위치
            chart->setTitle("광산구 동별 혼합시설 점수"); //차트제목

            //차트애니메이션 설정
            chart->setAnimationOptions(QChart::SeriesAnimations); //차트애니메이션
            chartView->setRenderHint(QPainter::Antialiasing);
            //차트테마설정
            chartView->chart()->setTheme(QChart::ChartThemeBlueCerulean);
            // 차트 폰트 설정
            QFont titleFont("맑은 고딕", 18, QFont::Bold);
            chart->setTitleFont(titleFont);

            QFont asixXFont("맑은 고딕", 14);
            chart->legend()->setFont(asixXFont);
            axisX->setLabelsFont(asixXFont);

            QFont asixYFont("맑은 고딕", 11, QFont::Bold);
            axisY->setLabelsFont(asixYFont);

            set0->setLabelFont(asixYFont);

            // 차트 데이터 레이블 색 설정
    //        QColor barColor1(189,234,224); // 민트색으로 지정
    //        set0->setColor(barColor1);
            set0->setLabelColor(Qt::gray);

            //y에 동이름넣기
            axisY->append(categories);
            chart->addAxis(axisY, Qt::AlignLeft);
            series->attachAxis(axisY);

            //x에 어쩌고저쩌고
            chart->addAxis(axisX, Qt::AlignBottom);
            axisX->setRange(0, 0.7);
            series->attachAxis(axisX);
            axisX->setLabelFormat("%.1f");
    //        axisX->applyNiceNumbers(); //범위를 자연스럽게 조정하는 매서드

            chartView->resize(300,200);
    //        chartView->update();
            //차트 새로운 창에서 뜨게하는거
            window->setCentralWidget(chartView); //chartView 가운데로
            window->resize(800, 800);
            window->move(1000,100);
            window->show();

    //        dbl_chart_items.clear();
    }

    else if(code == "테이블 선택"){

    }

    // 그 외 시설 점수
    else{
        for (int i = 0; i < h_len; i++) {
                QList<QString> inner_list;
                for (int j = 0; j < c_len; j++) {
                    inner_list.append(data_list[list_count]);
                    list_count++;
                }
                doubl_data_list.append(inner_list);
            }

        qDebug() << code;
        qDebug() << doubl_data_list;

        ui->tableView->clearSpans();
        model->clear();

        for(int i = 0; i < h_len; i++){
            items.append(new QStandardItem(doubl_data_list[i][0]));
            items.append(new QStandardItem(doubl_data_list[i][1]));
            if(i > 0){
                double score = ((doubl_data_list[i][1].toDouble() / doubl_data_list[0][1].toDouble() * 100) / (doubl_data_list[0][1].toDouble() / 13771 * 100));
                items.append(new QStandardItem(QString::number(score).left(3)));

                //차트에 넣을 아이템 생성(이중 리스트로)
                inner_chart_items.append(doubl_data_list[i][0]); //계 다음부터
                inner_chart_items.append(QString::number(score).left(3));
                dbl_chart_items.append(inner_chart_items);
                inner_chart_items.removeAt(0);
                inner_chart_items.removeAt(0);
            }
            model->appendRow(items);
            items.clear();
        }

        QBarSet *set0 = new QBarSet("시설 점수");
    //광산구 혼합시설 -> 차트 셋팅 시작//
        chart->removeAllSeries();
        chart->setTitle("");
        axisY->clear();

        qDebug()<<"dbl_chart_items           "<<dbl_chart_items;
        qDebug()<<"더블차트아이템len"<<dbl_chart_items.size();

//            QBarSet *set0 = new QBarSet("시설 점수");
        QHorizontalBarSeries *series = new QHorizontalBarSeries();
        chart->addSeries(series); //두번적으면 "Can not add series. Series already on the chart."

        //차트 카테고리(y축)과 데이터 지정하기
        QStringList categories;
        for (int i=dbl_chart_items.length()-1; i>=0; i--){
            categories << dbl_chart_items[i][0];

           QString setdata = dbl_chart_items[i][1];
           *set0 << setdata.toFloat();

        }
           series->append(set0);

        //데이터레이블, 범례 켜기/위치 지정, 차트제목 설정
        series->setLabelsVisible(true); //데이터레이블 켜기
        series->setLabelsPosition(QHorizontalBarSeries::LabelsOutsideEnd);
        chart->legend()->setVisible(true); //범례켜기
        chart->legend()->setAlignment(Qt::AlignBottom); //범례위치

        //차트애니메이션 설정
        chart->setAnimationOptions(QChart::SeriesAnimations); //차트애니메이션
        chartView->setRenderHint(QPainter::Antialiasing);
        //차트테마설정
        chartView->chart()->setTheme(QChart::ChartThemeBlueCerulean);
        // 차트 폰트 설정
        QFont titleFont("맑은 고딕", 18, QFont::Bold);
        chart->setTitleFont(titleFont);

        QFont asixXFont("맑은 고딕", 14);
        chart->legend()->setFont(asixXFont);
        axisX->setLabelsFont(asixXFont);

        QFont asixYFont("맑은 고딕", 11, QFont::Bold);
        axisY->setLabelsFont(asixYFont);

        set0->setLabelFont(asixYFont);

        // 차트 데이터 레이블 색 설정
//        QColor barColor1(189,234,224); // 민트색으로 지정
//        set0->setColor(barColor1);
        set0->setLabelColor(Qt::gray);

        //y에 동이름넣기
        axisY->append(categories);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        //x에 어쩌고저쩌고
        chart->addAxis(axisX, Qt::AlignBottom);
        axisX->setRange(0, 1.4);
        series->attachAxis(axisX);
        axisX->setLabelFormat("%.1f");
//        axisX->applyNiceNumbers(); //범위를 자연스럽게 조정하는 매서드

        chartView->resize(300,200);
//        chartView->update();
        //차트 새로운 창에서 뜨게하는거
        window->setCentralWidget(chartView); //chartView 가운데로
        window->resize(800, 800);
        window->move(1000,100);
        window->show();

//        dbl_chart_items.clear();

        if(code == "부동산업체/산업단지"){
            model->setHeaderData(0, Qt::Horizontal, QObject::tr("동"));
            model->setHeaderData(1, Qt::Horizontal, QObject::tr("부동산업체/산업단지 합계"));
            model->setHeaderData(2, Qt::Horizontal, QObject::tr("부동산업체/산업단지 점수"));

            chart->setTitle("광산구 동별 부동산업체/산업단지 점수"); //차트제목
            axisX->setRange(0, 1.4);
        }
        else if(code == "편의 시설"){
            model->setHeaderData(0, Qt::Horizontal, QObject::tr("동"));
            model->setHeaderData(1, Qt::Horizontal, QObject::tr("편의시설 합계"));
            model->setHeaderData(2, Qt::Horizontal, QObject::tr("편의시설 점수"));

            chart->setTitle("광산구 동별 편의시설 점수"); //차트제목
            axisX->setRange(0, 0.6);
        }
        else if(code == "음식 시설"){
            model->setHeaderData(0, Qt::Horizontal, QObject::tr("동"));
            model->setHeaderData(1, Qt::Horizontal, QObject::tr("음식시설 합계"));
            model->setHeaderData(2, Qt::Horizontal, QObject::tr("음식시설 점수"));

            chart->setTitle("광산구 동별 음식시설 점수"); //차트제목
            axisX->setRange(0, 0.3);
        }        
        ui->tableView->setModel(model);
        ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    }
}
