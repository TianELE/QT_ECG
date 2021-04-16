#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QLabel>
#include <QDateTime>
#include <QTimer>
#include <QString>
#include <QMovie>
#include <QDebug>
#include<iostream>
using namespace std;

extern  QList<int> ECG_data;
extern  QList<float> TEMP_data;
extern uint8_t ECG_Thread_isRun;
extern int ECG_BPM_TO_client,ECG_BPM_SEND_FLAG,isOutFlag;
static QString ECG_BPM_OUT;

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
    Thread_class->TempLabel = ui->label_temp;
    Thread_class->ECGBPMLabel = ui->label_bpm;
    Thread_class->ECGcacheLabel = ui->label_ECG_cache;
    Thread_class->outPath_lineEdit = ui->lineEdit_pathOUT;
    Thread_class->ECG_chart_init();

//    QTimer *ECG_time = new QTimer(this);
//    ECG_time->start(20);
//    connect(ECG_time, &QTimer::timeout,this,&MainWindow::ToThread);

    _init();

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
    if(Thread_class!=nullptr)
    {
        if(Thread_1->isRunning())
        {
            qDebug("thread stop now");
            Thread_1->quit();
            Thread_1->wait();
        }
    }
}

void MainWindow::_init()
{
    ui->label_client_ip->setText("无设备连接:-(");
    ui->label_client_ip_2->setText("无设备连接:-(");

    ui->lineEdit_pathOUT->setEnabled(false);
    ui->Button_setOUT->setEnabled(false);
    ui->pushButton_startOUT->setEnabled(false);
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
        ui->pushButton_connect_tcp_2->setText("断开");
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
        ui->label_client_ip_2->setText(client_ip);
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
    if(ECG_Thread_isRun == 0)
    {
        while(!ECG_data.isEmpty())
        {
            ui->label_ECG_cache->setNum(ECG_data.size());
        }
        ui->label_ECG_cache->setNum(0);
    }
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
        ui->pushButton_connect_tcp_2->setText("连接");
        ui->label_client_ip->setText("无设备连接:-(");
        ui->label_client_ip_2->setText("无设备连接:-(");
        tcpServer->close();
        tcpSocket->close();

        ui->lineEdit_IP->setEnabled(true);
        ui->lineEdit_wifi_id->setEnabled(true);
        ui->lineEdit_wifi_pw->setEnabled(true);
        ui->spinBox_Port->setEnabled(true);
        ui->pushButton_push->setEnabled(true);

    }
}
void MainWindow::on_pushButton_connect_tcp_2_clicked()
{
    if(ui->pushButton_connect_tcp_2->text() == tr("连接"))
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
        ui->pushButton_connect_tcp_2->setText("连接");
        ui->label_client_ip->setText("无设备连接:-(");
        ui->label_client_ip_2->setText("无设备连接:-(");
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
//        if(Thread_1->isRunning()&&ECG_Thread_isRun == 0)
//        {
//            qDebug("running");
//            Thread_1->quit();
//            Thread_1->wait();
//        }
        if(ECG_data.size()<=150||ECG_data.size()>=300)
        {
//            ECG_Thread_isRun = 1;
//            Thread_1->start();
            qDebug("starting");
            emit ToThread();
        }
//        emit ToThread();

    }
    if(ECG_BPM_TO_client!=0&&ECG_BPM_SEND_FLAG == 1)
    {
       ECG_BPM_SEND_FLAG = 0;
       ECG_BPM_OUT = QString("%1").arg(ECG_BPM_TO_client)+"\n";
       tcpSocket->write(ECG_BPM_OUT.toLatin1(),ECG_BPM_OUT.toLatin1().size());
       qDebug()<<ECG_BPM_OUT.toLatin1();
       ECG_BPM_TO_client = 0;
       ECG_BPM_OUT.clear();
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
    ui->label_local_ip_2->setText(ip_port);
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


void MainWindow::on_checkBox_isOUT_stateChanged(int arg1)
{
    if(arg1 == Qt::Checked)
    {
        ui->lineEdit_pathOUT->setEnabled(true);
        ui->Button_setOUT->setEnabled(true);
        ui->pushButton_startOUT->setEnabled(true);
    }
    else if(arg1 == Qt::Unchecked)
    {
        ui->lineEdit_pathOUT->setEnabled(false);
        ui->Button_setOUT->setEnabled(false);
        ui->pushButton_startOUT->setEnabled(false);
    }
}

void MainWindow::on_Button_setOUT_clicked()
{
    QString outpath = QFileDialog::getExistingDirectory(this,"选择输出文件目录","./",QFileDialog::ShowDirsOnly);
    ui->lineEdit_pathOUT->setText(outpath);
}

void MainWindow::on_pushButton_startOUT_clicked()
{
    if(ui->pushButton_startOUT->text() == tr("开始"))
    {
        ui->checkBox_isOUT->setEnabled(false);
        ui->pushButton_startOUT->setText("停止");
        ui->pushButton_connect_tcp->setEnabled(false);
        ui->pushButton_connect_tcp_2->setEnabled(false);
        ui->lineEdit_pathOUT->setEnabled(false);
        ui->Button_setOUT->setEnabled(false);
        isOutFlag = 1;
    }
    else if(ui->pushButton_startOUT->text() == tr("停止"))
    {
        ui->checkBox_isOUT->setEnabled(true);
        ui->pushButton_startOUT->setText("开始");
        ui->pushButton_connect_tcp->setEnabled(true);
        ui->pushButton_connect_tcp_2->setEnabled(true);
        ui->lineEdit_pathOUT->setEnabled(true);
        ui->Button_setOUT->setEnabled(true);
        isOutFlag = 0;
    }
}

void MainWindow::on_pushButton_openWave_clicked()
{
    QProcess openEXE(this);
    QString command = "./ECGSHOW/ECGWaveShow.exe";
    openEXE.startDetached(command,QStringList());
}

void MainWindow::on_pushButton_openFile_clicked()
{
    QString dirpath = ui->lineEdit_pathOUT->text();
    QDesktopServices::openUrl(QUrl("file:///"+dirpath));
}
