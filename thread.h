#ifndef THREAD_H
#define THREAD_H
#pragma execution_character_set("utf-8")
#include <QObject>
#include <qthread.h>
#include <QList>
#include <QtCharts>
#include <QPen>


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
    QLabel *TempLable;

    void Thread_Fun(void);
    void ECG_chart_init();
    float filter_low_35(float inputValue);
    float filter_low_25(float inputValue);
    float filter_high_1(float inputValue);

private:
    QValueAxis *ECG_axisY;
    QValueAxis *ECG_axisX;
    QLineSeries *ECG_line;
    QChart *ECG_chart;
};


#endif // THREAD_H
