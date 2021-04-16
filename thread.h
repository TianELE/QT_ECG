#ifndef THREAD_H
#define THREAD_H
#pragma execution_character_set("utf-8")
#include <QObject>
#include <qthread.h>
#include <QList>
#include <QtCharts>
#include <QPen>
#include <stdio.h>


class Thread :public QObject
{
    Q_OBJECT
public:
    Thread();
    ~Thread();

    int ECG_cout;
    int TEMP_cout;
    uint8_t chart_flag;
    float Temp_sum;

    QChartView *ECGchart;
    QLabel *TempLabel;
    QLabel *ECGBPMLabel;
    QLabel *ECGcacheLabel;
    QLineEdit *outPath_lineEdit;

    void Thread_Fun(void);
    void ECG_chart_init();
    float filter_low_35(float inputValue);
    float filter_low_25(float inputValue);
    float filter_high_0_67(float inputValue);
    void find_maxValue(float intputValue);
    void Get_R(int inputTime,float intputValue);

private:
    QValueAxis *ECG_axisY;
    QValueAxis *ECG_axisX;
    QLineSeries *ECG_line;
    QLineSeries *ECG_MAX_line;

    FILE *fileout;

    QChart *ECG_chart;
};


#endif // THREAD_H
