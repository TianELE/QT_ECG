#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma execution_character_set("utf-8")
#include <QMainWindow>
#include <QLabel>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMessageBox>
#include <QtCharts>
#include <QPen>
#include <QList>
#include "thread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadStyleSheet(const QString &styleSheetFile);
    void time_update(); //时间更新槽函数，状态栏显示时间
    void tcp_init();
    void set_IP_PORT();
//    void ECG_chart_init();

private slots:
    void on_pushButton_clicked();

    void on_actionMacOS_triggered();

    void on_actionManjaro_triggered();

    void on_actionAMOLED_triggered();

    void on_actionMaterialDark_triggered();

    void on_actionNormal_triggered();

    void on_tabWidget_tabBarClicked(int index);

    void SlotNewConnection();

    void on_pushButton_connect_tcp_clicked();

    void ServerDisConnection();

    void Read_data();

    void on_lineEdit_IP_editingFinished();

    void on_spinBox_Port_valueChanged(int arg1);


signals:
    void ToThread();

private:
    Ui::MainWindow *ui;
    QLabel *currentTimeLabel;
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
//    QValueAxis *ECG_axisY;
//    QValueAxis *ECG_axisX;
//    QLineSeries *ECG_line;
//    QChart *ECG_chart;
    QThread *Thread_1;
    Thread *Thread_class;

};
#endif // MAINWINDOW_H
