#include"thread.h"
#include <QDebug>
#include <QLabel>
#include <QColor>


QList<int> ECG_data;
QList<float> TEMP_data;
uint8_t ECG_Thread_isRun = 0;
int ECG_BPM_TO_client = 0,ECG_BPM_SEND_FLAG = 0,isOutFlag = 0;
static float ECG_filter;
static bool fileopen = false;
static QString outdirpath = "";

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

static float ECG_MAX_VALUE = 0;
static int ECG_MAX_TIME = 0,ECG_GETR_NUM = 0,ECG_GETR_COUT = 0;
static uint8_t Value_Window = 40;
static QList<QPointF> ECG_MAX_vec;
static QList<QPointF> ECG_MAX_GETR_COL;
static QList<QPointF> ECG_MAX_GETR;
static float ECG_MAX_SUM = 0,ECG_MAX_AVE = 0;
static int ECG_BPM = 0, ECG_LAST_MAX_TIME = 0;

Thread::Thread()
{
    ECG_cout = 0;
    TEMP_cout = 0;
    chart_flag = 0;
    Temp_sum = 0;
    ECG_BPM_TO_client = 0;
    ECG_data.clear();
    TEMP_data.clear();
}
Thread::~Thread()
{
}

void Thread::Thread_Fun()
{

    char Temp_show[10];

    while(!ECG_data.isEmpty()){
        if(ECG_cout>=5000) {
            if(chart_flag != 1) chart_flag = 1;
            ECG_cout = 0;
        }
            ECG_filter = filter_low_35((float)ECG_data.at(0))/290;
//        ECG_filter = filter_low_25((float)ECG_data.at(0))/195;
            ECG_filter = filter_low_25(ECG_filter)/195;
//        ECG_filter = filter_high_0_67(ECG_filter);
//            qDebug()<<ECG_filter;
//            ECG_filter = (float)ECG_data.at(0);
        find_maxValue(ECG_filter);

        /**********************************************/
        if(isOutFlag == 1)
        {
            if(!fileopen)
            {
                outdirpath = outPath_lineEdit->text();
                QDateTime current_datetime = QDateTime::currentDateTime();
                outdirpath += ("/ECG"+current_datetime.toString("yyyy-MM-dd-hh-mm-ss")+".txt");
                fileout = new FILE;
                fopen_s(&fileout,outdirpath.toStdString().data(),"wb+");
                fileopen = true;
            }
        }
        if(isOutFlag == 1&&fileopen == true)
        {
            fprintf(fileout,"%.1f\n",ECG_filter);
        }
        else if(isOutFlag == 0&&fileopen == true)
        {
            fileopen = false;
            fclose(fileout);
            outdirpath = "";
        }
        /*******************************************/


        if(chart_flag == 0){
//                ECG_line->append(QPointF(ECG_cout,((float)ECG_data.at(0)/4096*100-30)));
            ECG_line->append(QPointF(ECG_cout,(ECG_filter/4096*100-30)));
            if(!ECG_MAX_vec.isEmpty())
            {
                if((ECG_cout-200)==int(ECG_MAX_vec.at(0).x()))
                {
                    ECG_MAX_line->append(QPointF(ECG_MAX_vec.at(0).x(),ECG_MAX_vec.at(0).y()/4096*20+80));
                    if(int(ECG_MAX_vec.at(0).x())<=3500)
                    {
                        Get_R(int(ECG_MAX_vec.at(0).x()),float(ECG_MAX_vec.at(0).y()));
                    }
                    ECGcacheLabel->setNum(ECG_data.size());
                }
                else
                {
                    ECG_MAX_line->append(QPointF(ECG_cout-200,80));
                }
                if(ECG_cout>4000)
                {
                    Get_R(ECG_cout,0);
                }
                if(ECG_MAX_vec.size()>=2){
                    ECG_MAX_vec.removeFirst();
                }
            }
        }
        else if(chart_flag == 1)
        {
//                ECG_line->replace((int)(ECG_cout/5),QPointF(ECG_cout,((float)ECG_data.at(0)/4096*100-30)));
            ECG_line->replace(int(ECG_cout/5),QPointF(ECG_cout,(ECG_filter/4096*100-30)));
            if(!ECG_MAX_vec.isEmpty())
            {
                if((ECG_cout-200)==int(ECG_MAX_vec.at(0).x()))
                {
                    ECG_MAX_line->replace(int(ECG_cout/5),QPointF(ECG_MAX_vec.at(0).x(),ECG_MAX_vec.at(0).y()/4096*20+80));
                    if(int(ECG_MAX_vec.at(0).x())<=4000)
                    {
                        Get_R(int(ECG_MAX_vec.at(0).x()),float(ECG_MAX_vec.at(0).y()));
                    }
                    ECGcacheLabel->setNum(ECG_data.size());
//                    qDebug()<<ECG_data.size();
                }
                else
                {
                    ECG_MAX_line->replace(int(ECG_cout/5),QPointF(ECG_cout-200,80));
                }
                if(ECG_cout>4500)
                {
                    Get_R(ECG_cout,0);
                }
                if(ECG_MAX_vec.size()>=2){
                    ECG_MAX_vec.removeFirst();
                }
            }
        }

        if(ECG_data.size()>500)
        {
            QThread::usleep(1500);
        }
        else if(ECG_data.size()>300&&ECG_data.size()<500)
        {
            QThread::usleep(2500);
        }
        else{
            QThread::usleep(5000);
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
                if(Temp_sum/10>=35&&Temp_sum/10<=50){
                    sprintf(Temp_show,"%.1f",Temp_sum/10);
                    TempLabel->setText(Temp_show);
                }
                memset(Temp_show,0,sizeof(Temp_show));
                Temp_sum = 0;
                TEMP_cout = 0;
            }
        }
    }
    ECG_Thread_isRun = 0;
}

void Thread::ECG_chart_init()
{
    ECG_axisY = new QValueAxis();
    ECG_axisX = new QValueAxis();
    ECG_line = new QLineSeries();
    /*******/
    ECG_MAX_line = new QLineSeries();
    /*******/

    ECG_chart = new QChart();

    ECG_chart->addSeries(ECG_line);//添加曲线到chart
    /*****/
    ECG_chart->addSeries(ECG_MAX_line);
    /*******/

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

    /*********/
    ECG_MAX_line->attachAxis(ECG_axisX);
    ECG_MAX_line->attachAxis(ECG_axisY);
    ECG_MAX_line->setUseOpenGL(true);
    ECG_MAX_line->setColor(QColor(193,96,200));
    /*********/

    ECG_chart->legend()->hide();
    ECG_chart->setBackgroundVisible(false);
    ECGchart->setChart(ECG_chart);
}
/*************find_maxValue******************/
void Thread::find_maxValue(float intputValue)
{
    float ECG_tempValue;
    ECG_tempValue = intputValue;
    if(--Value_Window > 0)
    {
        if(ECG_MAX_VALUE<ECG_tempValue)
        {
            ECG_MAX_VALUE = ECG_tempValue;
            ECG_MAX_TIME = ECG_cout;
        }
    }
    else
    {
        Value_Window = 40;
        ECG_MAX_vec.append(QPointF(ECG_MAX_TIME,ECG_MAX_VALUE));
        ECG_MAX_VALUE = 0;
//        qDebug()<<ECG_MAX_vec.at(0);
//        ECG_MAX_vec.removeFirst();
    }
}
/**************get_bpm*********/
void Thread::Get_R(int inputTime,float intputValue)
{
    if(inputTime<=4000)
    {
        ECG_MAX_GETR_COL.append(QPointF(inputTime,intputValue));
        ECG_MAX_SUM +=  intputValue;
        ECG_MAX_AVE = ECG_MAX_SUM/(ECG_MAX_GETR_COL.size());
        ECG_GETR_NUM = ECG_MAX_GETR_COL.size();
    }
    else if(inputTime>4000)
    {
        if(!ECG_MAX_GETR_COL.isEmpty()&&ECG_GETR_COUT<ECG_GETR_NUM)
        {
            if(ECG_MAX_GETR_COL.at(ECG_GETR_COUT).y()>(ECG_MAX_AVE+300)&&(ECG_MAX_GETR_COL.at(ECG_GETR_COUT).x()-ECG_LAST_MAX_TIME)>100)
            {
                ECG_MAX_GETR.append(ECG_MAX_GETR_COL.at(ECG_GETR_COUT));
                ECG_LAST_MAX_TIME = ECG_MAX_GETR_COL.at(ECG_GETR_COUT).x();
            }
            ECG_GETR_COUT++;
        }
        else
        {
            if(ECG_MAX_GETR.size()>=2)
            {
                ECG_BPM = int(ECG_MAX_GETR.last().x() - ECG_MAX_GETR.first().x())/(ECG_MAX_GETR.size()-1);
                ECG_BPM = int(1000.0/float(ECG_BPM)*60);
                ECGBPMLabel->setNum(ECG_BPM);
//                qDebug()<<ECG_MAX_GETR;
                if(ECG_BPM_SEND_FLAG==0)
                {
                    ECG_BPM_TO_client = ECG_BPM;
                    ECG_BPM_SEND_FLAG = 1;
                }
                ECG_BPM = 0;
//                qDebug()<<ECG_MAX_AVE;
                ECG_MAX_GETR_COL.clear();
                ECG_MAX_SUM = 0;
                ECG_MAX_AVE = 0;
                ECG_GETR_COUT = 0;
                ECG_LAST_MAX_TIME = 0;
                ECG_MAX_GETR.clear();
            }
        }
    }
}

/******************filter code***************/
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

float Thread::filter_high_0_67(float inputValue)
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

