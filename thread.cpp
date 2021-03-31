#include"thread.h"
#include <QDebug>
#include <QLabel>
#include <QColor>

QList<int> ECG_data;
QList<float> TEMP_data;
static float ECG_filter;
/*******************************/
static float xv1[5],yv1[5];
const float NUM1[5] = {
   0.004824343357716f,  0.01929737343086f,   0.0289460601463f,  0.01929737343086f,
   0.004824343357716f
};
const float DEN1[5] = {
                   1.0f,   -2.369513007182f,    2.313988414416f,   -1.054665405879f,
     0.1873794923682f
};
/********************************/
static float xv[5],yv[5];
const float NUM[5] = {
    0.003199660030224f,0.0003195727481286f, 0.004214703533503f,0.0003195727481286f,
    0.003199660030224f
};
const float DEN[5] = {
    1,   -3.063015682018f,    3.604390967577f,   -1.918796782283f,
        0.3886746658152f
};
/*********************************/
static float xv2[5],yv2[5];
const float NUM2[5] = {
    0.9597822300872f,   -3.839128920349f,    5.758693380523f,   -3.839128920349f,
         0.9597822300872f
};
const float DEN2[5] = {
    1,   -3.917907865392f,    5.757076379118f,   -3.760349507695f,
         0.9211819291912f
};
/*******************************/

Thread::Thread()
{
    ECG_cout = 0;
    TEMP_cout = 0;
    chart_flag = 0;
    Temp_sum = 0;
    ECG_data.clear();
    TEMP_data.clear();
}
Thread::~Thread()
{
}

void Thread::Thread_Fun()
{

//        for(int i = 0;i<ECG_data.size();i++){
//            qDebug()<<ECG_data.at(i);
//        }
//        for(int i = 0;i<TEMP_data.size();i++){
//            qDebug()<<TEMP_data.at(i);
//        }

//        ECG_data.clear();
//        TEMP_data.clear();
    char Temp_show[10];
        while(!ECG_data.isEmpty()){
            if(ECG_cout>=5000) {
                if(chart_flag != 1) chart_flag = 1;
                ECG_cout = 0;
            }
            ECG_filter = filter_low_35((float)ECG_data.at(0))/290;
            ECG_filter = filter_low_25(ECG_filter)/195;
//            ECG_filter = filter_high_1(ECG_filter);
//            qDebug()<<ECG_filter;
//            ECG_filter = (float)ECG_data.at(0);
            if(chart_flag == 0){
//                ECG_line->append(QPointF(ECG_cout,((float)ECG_data.at(0)/4096*100-30)));
                ECG_line->append(QPointF(ECG_cout,(ECG_filter/4096*100-30)));
            }
            else if(chart_flag == 1)
            {
//                ECG_line->replace((int)(ECG_cout/5),QPointF(ECG_cout,((float)ECG_data.at(0)/4096*100-30)));
                ECG_line->replace((int)(ECG_cout/5),QPointF(ECG_cout,(ECG_filter/4096*100-30)));
            }
//            qDebug()<<ECG_data.size();

            if(ECG_data.size()>100)
            {
                QThread::usleep(100);
            }
            else{
                QThread::msleep(5);
            }

            ECG_data.removeFirst();
            ECG_cout += 5;

            if(!TEMP_data.isEmpty())
            {
                if(TEMP_cout < 10)
                {
                    Temp_sum += TEMP_data.at(0);
                    TEMP_data.removeFirst();
                    TEMP_cout++;
                }
                else if(TEMP_cout >= 10)
                {
                    if(!TEMP_data.isEmpty()) TEMP_data.removeFirst();
                    sprintf(Temp_show,"%.1f",Temp_sum/10);
                    TempLable->setText(Temp_show);
                    memset(Temp_show,0,sizeof(Temp_show));
                    Temp_sum = 0;
                    TEMP_cout = 0;
                }
            }
        }
}

void Thread::ECG_chart_init()
{
    ECG_axisY = new QValueAxis();
    ECG_axisX = new QValueAxis();
    ECG_line = new QLineSeries();
    ECG_chart = new QChart();

    ECG_chart->addSeries(ECG_line);//添加曲线到chart

    ECG_axisX->setRange(0,5000);
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



    ECG_axisX->setGridLinePen(QPen(Qt::darkGreen,1,Qt::DashDotDotLine,Qt::SquareCap,Qt::RoundJoin));
    ECG_axisY->setGridLinePen(QPen(Qt::darkCyan,1,Qt::DashDotLine,Qt::SquareCap,Qt::RoundJoin));

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
    ECG_line->setUseOpenGL(true);
    ECG_line->setColor(QColor(193,96,200));

    ECG_chart->legend()->hide();
    ECG_chart->setBackgroundVisible(false);
    ECGchart->setChart(ECG_chart);
}

float Thread::filter_low_35(float inputValue) //iir lowpass
{
    xv[0] = xv[1];xv[1] = xv[2];xv[2] = xv[3];xv[3] = xv[4];xv[4] = inputValue/0.0031996f;
    yv[0] = yv[1];yv[1] = yv[2];yv[2] = yv[3];yv[3] = yv[4];
    yv[4] = NUM[0]*xv[4]+NUM[1]*xv[3]+NUM[2]*xv[2]+NUM[3]*xv[1]+NUM[4]*xv[0]-DEN[1]*yv[3]-DEN[2]*yv[2]-DEN[3]*yv[1]-DEN[4]*yv[0];
    return yv[4];
}

float Thread::filter_low_25(float inputValue)
{
    for(int i=0;i<4;i++)
    {
        xv1[i] = xv1[i+1];
    }
    xv1[4] = inputValue/0.00482f;
    for(int j=0;j<4;j++)
    {
        yv1[j] = yv1[j+1];
    }
    yv1[4] = NUM1[0]*xv1[4]+NUM1[1]*xv1[3]+NUM1[2]*xv1[2]+NUM1[3]*xv1[1]+NUM1[4]*xv1[0]-DEN1[1]*yv1[3]-DEN1[2]*yv1[2]-DEN1[3]*yv1[1]-DEN1[4]*yv1[0];
    return yv1[4];
}

float Thread::filter_high_1(float inputValue)
{
    for(int i=0;i<4;i++)
    {
        xv2[i] = xv2[i+1];
    }
    xv2[4] = inputValue;
    for(int j=0;j<4;j++)
    {
        yv2[j] = yv2[j+1];
    }
    yv2[4] = NUM2[0]*xv2[4]+NUM2[1]*xv2[3]+NUM2[2]*xv2[2]+NUM2[3]*xv2[1]+NUM2[4]*xv2[0]-DEN2[1]*yv2[3]-DEN2[2]*yv2[2]-DEN2[3]*yv2[1]-DEN2[4]*yv2[0];
    return yv2[4];
}
