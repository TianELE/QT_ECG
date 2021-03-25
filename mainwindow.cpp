#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QLabel>
#include <QDateTime>
#include <QTimer>
#include <QString>
#include <QMovie>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    tcpServer = new QTcpServer();//创建tcp服务端
//    tcpSocket = new QTcpSocket();

    ECG_chart_init();
    ui->label_client_ip->setText("无设备连接:-(");

    currentTimeLabel = new QLabel; // 创建QLabel控件
    QTimer *time_timer = new QTimer(this);
    time_timer->start(1000); //每隔1000ms发送timeout的信号
    ui->statusbar->addWidget(currentTimeLabel);
    connect(time_timer, &QTimer::timeout,this,&MainWindow::time_update);

    set_IP_PORT();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadStyleSheet(const QString &styleSheetFile)
{
    QFile file(styleSheetFile);
    file.open(QFile::ReadOnly);
    if (file.isOpen())
    {
        QString styleSheet = this->styleSheet();
        //styleSheet += QLatin1String(file.readAll());//读取样式表文件
        styleSheet = QLatin1String(file.readAll());//读取样式表文件
        this->setStyleSheet(styleSheet);//把文件内容传参
        file.close();
    }
    else
    {
    }

}

void MainWindow::time_update()
{
    //[1] 获取时间
    QDateTime current_time = QDateTime::currentDateTime();
    QString timestr = current_time.toString(QStringLiteral( "yyyy-MM-dd hh:mm:ss")); //设置显示的格式
    currentTimeLabel->setText(timestr); //设置label的文本内容为时间

}


void MainWindow::on_actionMacOS_triggered()
{
    this->loadStyleSheet(":/qss/MacOS.qss");
}

void MainWindow::on_actionManjaro_triggered()
{
    this->loadStyleSheet(":/qss/ManjaroMix.qss");
}

void MainWindow::on_actionAMOLED_triggered()
{
    this->loadStyleSheet(":/qss/AMOLED.qss");
}

void MainWindow::on_actionMaterialDark_triggered()
{
    this->loadStyleSheet(":/qss/MaterialDark.qss");
}

void MainWindow::on_actionNormal_triggered()
{
    this->loadStyleSheet(":/qss/init.qss");
}

void MainWindow::on_pushButton_clicked()
{

}


void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    static QMovie *gif = new QMovie(":/icon/img/Raingif.gif");
    ui->label_gif->setMovie(gif);
    if(index == 1)
    {
        gif->start();
    }
    else{
        gif->stop();
    }
}

void MainWindow::tcp_init()
{
    tcpServer = new QTcpServer();//创建tcp服务端
    tcpSocket = new QTcpSocket();
    unsigned short port = ui->spinBox_Port->text().toUShort();
    if(!tcpServer->listen(QHostAddress::Any,port))
    {
        QMessageBox::information(this,"提示","创建监听错误");
    }
    else
    {
        connect(tcpServer,SIGNAL(newConnection()),this,SLOT(SlotNewConnection()));
        ui->pushButton_connect_tcp->setText("断开");
    }
}

void MainWindow::SlotNewConnection()
{
    tcpSocket = tcpServer->nextPendingConnection();
    if(tcpSocket == NULL)
    {
        QMessageBox::information(this,"提示","客户端连接错误");
    }
    else {
        QString client_ip = tcpSocket->peerAddress().toString();
        QString client_port = QString::number(tcpSocket->peerPort());
        client_ip = client_ip+":"+client_port+" 已连接:-)";
        client_ip = client_ip.remove(0,7);
        ui->label_client_ip->setText(client_ip);
        qDebug()<<client_ip;
//        QMessageBox::information(this,"提示","客户端连接成功");
        connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(ServerDisConnection()));
        connect(tcpSocket,SIGNAL(readyRead()),this,SLOT(Read_data()));
    }
}

void MainWindow::ServerDisConnection()
{
    QMessageBox::information(this,"提示","客户端断开连接");
    tcpSocket->deleteLater();
    tcpServer->deleteLater();
    return;
}

void MainWindow::on_pushButton_connect_tcp_clicked()
{
    if(ui->pushButton_connect_tcp->text() == tr("连接"))
    {
        tcp_init();
        ui->lineEdit_IP->setEnabled(false);
        ui->lineEdit_wifi_id->setEnabled(false);
        ui->lineEdit_wifi_pw->setEnabled(false);
        ui->spinBox_Port->setEnabled(false);
        ui->pushButton_push->setEnabled(false);
    }
    else
    {
        ui->pushButton_connect_tcp->setText("连接");
        ui->label_client_ip->setText("无设备连接:-(");
        tcpServer->close();
        tcpSocket->close();

        ui->lineEdit_IP->setEnabled(true);
        ui->lineEdit_wifi_id->setEnabled(true);
        ui->lineEdit_wifi_pw->setEnabled(true);
        ui->spinBox_Port->setEnabled(true);
        ui->pushButton_push->setEnabled(true);
    }
}

void MainWindow::Read_data()
{
    QByteArray buffer;
    buffer = tcpSocket->readAll();
    if(!buffer.isEmpty())
    {
        qDebug()<<tr(buffer);
    }
}

void MainWindow::on_lineEdit_IP_editingFinished()
{
    set_IP_PORT();
}

void MainWindow::on_spinBox_Port_valueChanged(int arg1)
{
    set_IP_PORT();
}

void MainWindow::set_IP_PORT()
{
    QString ip_port = ui->lineEdit_IP->text()+":"+ui->spinBox_Port->text();
    ui->label_local_ip->setText(ip_port);
}

void MainWindow::ECG_chart_init()
{
    ECG_axisY = new QValueAxis();
    ECG_axisX = new QValueAxis();
    ECG_line = new QLineSeries();
    ECG_chart = new QChart();

    ECG_chart->addSeries(ECG_line);//添加曲线到chart

    ECG_axisX->setRange(-1500,3500);
    ECG_axisY->setRange(0,100);
    ECG_axisX->setTickCount(11);//主网格
    ECG_axisX->setMinorTickCount(4);//次网格
    ECG_axisY->setTickCount(6);
    ECG_axisY->setMinorTickCount(4);

    ECG_axisX->setTitleText("时间/ms");
    ECG_axisX->setLabelFormat("%d");
    ECG_axisX->setLabelsAngle(-45);
    ECG_axisX->setLabelsColor(Qt::darkCyan);
    QFont axisXfont = ECG_axisX->titleFont();
    axisXfont.setPointSize(12);
//    axisXfont.setBold(true);
    ECG_axisX->setTitleFont(axisXfont);

    ECG_axisY->setTitleText("幅值/%");
    ECG_axisY->setLabelFormat("%d");
    ECG_axisY->setLabelsAngle(-45);
    ECG_axisY->setLabelsColor(Qt::darkCyan);
    QFont axisYfont = ECG_axisY->titleFont();
    axisYfont.setPointSize(12);
    axisYfont.setBold(true);
    ECG_axisY->setTitleFont(axisYfont);



    ECG_axisX->setGridLinePen(QPen(Qt::darkGreen,2,Qt::DashDotDotLine,Qt::SquareCap,Qt::RoundJoin));
    ECG_axisY->setGridLinePen(QPen(Qt::darkCyan,2,Qt::DashDotLine,Qt::SquareCap,Qt::RoundJoin));

    ECG_axisX->setMinorGridLinePen(QPen(Qt::darkGreen,1,Qt::DotLine,Qt::SquareCap,Qt::RoundJoin));
    ECG_axisY->setMinorGridLinePen(QPen(Qt::darkCyan,1,Qt::DotLine,Qt::SquareCap,Qt::RoundJoin));


    ECG_axisX->setLinePen(QPen(Qt::darkCyan,3,Qt::SolidLine,Qt::SquareCap,Qt::RoundJoin));
    ECG_axisY->setLinePen(QPen(Qt::darkCyan,3,Qt::SolidLine,Qt::SquareCap,Qt::RoundJoin));

    ECG_axisX->setGridLineVisible(true);
    ECG_axisY->setGridLineVisible(true);//显示线框

    ECG_axisX->setLabelsVisible(true);
    ECG_axisY->setLabelsVisible(true);//显示刻度数值

    ECG_chart->addAxis(ECG_axisX,Qt::AlignBottom);
    ECG_chart->addAxis(ECG_axisY,Qt::AlignLeft);

    ECG_line->attachAxis(ECG_axisX);
    ECG_line->attachAxis(ECG_axisY);
    ECG_line->setColor(QColor(Qt::black));
    ECG_line->setUseOpenGL(true);

    ECG_chart->legend()->hide();
    ECG_chart->setBackgroundVisible(false);
    ui->ECG_LineChart->setChart(ECG_chart);

}
