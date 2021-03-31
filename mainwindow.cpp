#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QLabel>
#include <QDateTime>
#include <QTimer>
#include <QString>
#include <QMovie>
#include <QDebug>

extern  QList<int> ECG_data;
extern  QList<float> TEMP_data;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    tcpServer = new QTcpServer();//创建tcp服务端
//    tcpSocket = new QTcpSocket();

    Thread_1 = new QThread;
    Thread_class = new Thread;

    Thread_class->ECGchart = ui->ECG_LineChart;
    Thread_class->TempLable = ui->label_temp;
    Thread_class->ECG_chart_init();

//    QTimer *ECG_time = new QTimer(this);
//    ECG_time->start(20);
//    connect(ECG_time, &QTimer::timeout,this,&MainWindow::ToThread);

    ui->label_client_ip->setText("无设备连接:-(");

    currentTimeLabel = new QLabel; // 创建QLabel控件
    QTimer *time_timer = new QTimer(this);
    time_timer->start(1000); //每隔1000ms发送timeout的信号
    ui->statusbar->addWidget(currentTimeLabel);
    connect(time_timer, &QTimer::timeout,this,&MainWindow::time_update);

    set_IP_PORT();

//    emit ToThread();

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
        connect(this,&MainWindow::ToThread,Thread_class,&Thread::Thread_Fun);
        Thread_class->moveToThread(Thread_1);
        Thread_1->start();
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
    QStringList origin_datalist;
    buffer = tcpSocket->readAll();
    if(!buffer.isEmpty())
    {
//        qDebug()<<tr(buffer);
        origin_datalist = tr(buffer).split(":");
        while(!origin_datalist.isEmpty())
        {
            if(origin_datalist.at(0)=="AD")
            {
                ECG_data.append(origin_datalist.at(1).toInt());
                origin_datalist.removeFirst();
                origin_datalist.removeFirst();
            }
            else if(origin_datalist.at(0)=="TEMP")
            {
                TEMP_data.append(origin_datalist.at(1).toFloat());
                origin_datalist.removeFirst();
                origin_datalist.removeFirst();
            }
            else
            {
                origin_datalist.removeFirst();
            }
//            for(int i = 0;i<ECG_data.size();i++){
//                qDebug()<<ECG_data.at(i);
//            }
//            for(int i = 0;i<TEMP_data.size();i++){
//                qDebug()<<TEMP_data.at(i);
//            }
        }
        if(ECG_data.size()>=100)
        {
            emit ToThread();
        }
//        emit ToThread();

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

//void MainWindow::TimeoutECG()
//{
//    if(!ECG_data.isEmpty()){
//        ECG_cout += 8;
//        if(ECG_cout>5000) ECG_cout = 0;
//        ECG_line->append(QPointF(ECG_cout%5000,(ECG_data.at(0)/4092*100)));
//        ECG_data.removeFirst();
//    }
//    if(!TEMP_data.isEmpty())
//    {
//        qDebug()<<TEMP_data.at(0);
//        TEMP_data.removeFirst();
//    }
//}
