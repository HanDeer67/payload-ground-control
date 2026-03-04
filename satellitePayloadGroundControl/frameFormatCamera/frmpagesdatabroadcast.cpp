#include "frmpagesdatabroadcast.h"
#include "ui_frmpagesdatabroadcast.h"
#include "frameassemble.h"
#include "app.h"
#include <QFileDialog>

frmPagesDataBroadcast::frmPagesDataBroadcast(QWidget *parent) :
    QWidget(parent),
    ui(new Ui_frmpagesdatabroadcast)
{
    ui->setupUi(this);
    this->initForms();
}

frmPagesDataBroadcast::~frmPagesDataBroadcast()
{
    delete ui;
}

void frmPagesDataBroadcast::initForms()
{
    sendTimeTimer = new QTimer(this);
    sendTimeTimer->setTimerType(Qt::TimerType::PreciseTimer);
    sendTimeTimer->setInterval(1000);
    connect(sendTimeTimer,&QTimer::timeout,this,&frmPagesDataBroadcast::sendTimerTimeout);

    traceDataSendTimer = new QTimer(this);
    traceDataSendTimer->setTimerType(Qt::TimerType::PreciseTimer);
    traceDataSendTimer->setInterval(1000);
    connect(traceDataSendTimer,&QTimer::timeout,this,&frmPagesDataBroadcast::sendTraceDataTimerTimeout);


    //初始化两个时间
    QDateTime defaultTime(QDate(2009, 1, 1), QTime(0, 0, 0));
    if(App::baseTime == 0)
    {
//        App::baseTime = QDateTime::currentDateTime().toTime_t();
        App::baseTime = defaultTime.toTime_t();
    }

    if(App::startTime == 0)
    {
//        App::startTime = QDateTime::currentDateTime().toTime_t();
        App::startTime = defaultTime.toTime_t();
    }

    ui->dt_base_time->setDateTime(QDateTime::fromTime_t(App::baseTime));
    ui->dt_start_time->setDateTime(QDateTime::fromTime_t(App::startTime));

    ui->le_curve_path->setReadOnly(true);
    ui->le_trace_path->setReadOnly(true);

}

void frmPagesDataBroadcast::sendTimerTimeout()
{
    quint32 timeSpanSecond = timeDiffValue+startTimeCount;
    //根据答复 微妙值暂设为0 暂时不处理，填充00
    quint16 timeSpanUs = 0;
    //发送时间码
    //FrameAssemble::Instance()->RS422TimeCodeDataSend(timeSpanSecond,timeSpanUs);
    QString msg = "发送的秒为"+QString::number(timeSpanSecond)+"微妙为:"+QString::number(timeSpanUs);
    showSendTimeMsg(msg);
    startTimeCount++;
}

void frmPagesDataBroadcast::sendTraceDataTimerTimeout()
{
    //读取文件一行然后
    if(!traceFileHander.fileName().isEmpty() && traceFileHander.isOpen())
    {
        //文件还有内容继续发送
        if(!traceFileHander.atEnd())
        {
            QString content = traceFileHander.readLine();
            QStringList dataList = content.split("\t");
            if(dataList.length()>=8)
            {
                //转换数据并组装cmd进行发送

                //当前轨道模式：
                //0x01：注入轨道
                //0x02：GNSS定位轨道
                //0x04：无轨道

                quint8 orbitMode = 0;

                int index = ui->cb_trace_mode->currentIndex();
                orbitMode = index+1;
                if(orbitMode == 3)orbitMode = 4;


                //开始处理文件

                float x = dataList.at(0).toFloat();
                float y = dataList.at(1).toFloat();
                float z = dataList.at(2).toFloat();
                float Vx = dataList.at(3).toFloat();
                float Vy = dataList.at(4).toFloat();
                float Vz = dataList.at(5).toFloat();
                quint32 Second = dataList.at(6).toUInt();
                quint16 ms = dataList.at(7).toUShort();
                //发送卫星位置数据
                QString msg = QString("发送数据 X:%1 Y:%2 Z:%3 Vx:%4 Vy:%5 Vz:%6 模式:%7")
                        .arg(x)
                        .arg(y)
                        .arg(z)
                        .arg(Vx)
                        .arg(Vy)
                        .arg(Vz)
                        .arg(ui->cb_trace_mode->currentText());
                showSendTraceMsg(msg);
                FrameAssemble::Instance()->RS422SatelliteOrbitDataSend(orbitMode,Second,ms,x,y,z,Vx,Vy,Vz);
            }
        }
        //到达行尾结束
        else
        {
            stopSendTraceData();
            showSendTraceMsg("轨道数据广播完毕!");
        }
    }
}

void frmPagesDataBroadcast::showSendTimeMsg(QString msg)
{
    static int count = 0;
    count++;
    if(count>5000)
    {
         ui->te_time_send->clear();
         count = 0;
    }
    QString text = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]:")+msg;
    ui->te_time_send->append(text);
}

void frmPagesDataBroadcast::showSendTraceMsg(QString msg)
{
    //te_gps_data_show
    static int count = 0;
    count++;
    if(count>5000)
    {
         ui->te_gps_data_show->clear();
         count = 0;
    }
    QString text = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]:")+msg;
    ui->te_gps_data_show->append(text);
}

void frmPagesDataBroadcast::on_pb_send_location_clicked()
{
    //发送位置数据 敏感数据
    quint32 second = 0;
    quint16 ms = 0;
    float X = 0;
    float Y = 0;
    float Z = 0;
    FrameAssemble::Instance()->RS422SensitiveDataSend(second,ms,X,Y,Z);
}

void frmPagesDataBroadcast::on_pb_send_time_clicked()
{

    if(ui->pb_send_time->text()=="开始广播")
    {
        startTimeCount = 0;
        uint v1 = ui->dt_base_time->dateTime().toTime_t();
        uint v2 = ui->dt_start_time->dateTime().toTime_t();
//        timeDiffValue = qAbs(v1-v2);
        if(v1>v2)
        {
            timeDiffValue = v1-v2;
        }
        else
        {
            timeDiffValue = v2-v1;
        }

        qDebug()<<"timeDiffValue"<<timeDiffValue;
        //先发送一条数据 然后开启1s定时器
        sendTimerTimeout();
        if(sendTimeTimer)
        {
            sendTimeTimer->stop();
            sendTimeTimer->start();
        }

        ui->pb_send_time->setText("停止广播");
    }
    else
    {
        ui->pb_send_time->setText("开始广播");
        if(sendTimeTimer)
        {
            sendTimeTimer->stop();
        }
    }
}

//void frmPagesDataBroadcast::on_pb_send_satellite_location_clicked()
//{

//    if(ui->pb_send_satellite_location->text()=="开始广播")
//    {
//        startSendTraceData();
//    }
//    else
//    {
//        stopSendTraceData();
//        showSendTraceMsg("轨道数据停止广播!");
//    }
//}

void frmPagesDataBroadcast::on_pb_send_satellite_attitude_clicked()
{
    quint32 Second = 0;
    quint16 us = 0;
    float q1 = 0;
    float q2 = 1;
    float q3 = 2;
    float q4 = 3;
    //发送卫星姿态数据
    FrameAssemble::Instance()->RS422SatelliteGestureDataSend(Second,us,q1,q2,q3,q4);
}

void frmPagesDataBroadcast::on_pb_begin_inject_clicked()
{
    //读取文件

    QByteArray cmd;

    QFile file(curevFileName);
    if(file.exists())
    {
        //开始读取文件中的第一行
        if(file.open(QIODevice::Text | QIODevice::ReadOnly))
        {
            //读取第一行的文本描述
            QString title = file.readLine();
            qDebug()<<title;

            //开始处理文件
//            QString strTime = QString("%1%2%3%4%5%6")
//                    .arg(list.at(5).toInt(),4,10,QLatin1Char('0'))
//                    .arg(list.at(6).toInt(),2,10,QLatin1Char('0'))
//                    .arg(list.at(7).toInt(),2,10,QLatin1Char('0'))
//                    .arg(list.at(8).toInt(),2,10,QLatin1Char('0'))
//                    .arg(list.at(9).toInt(),2,10,QLatin1Char('0'))
//                    .arg(list.at(10).toInt(),2,10,QLatin1Char('0'));

//            QDateTime dt = QDateTime::fromString(strTime,"yyyyMMddhhmmss");

            //时间差
//            int t0 = dt.toTime_t()-ui->dt_base_time->dateTime().toTime_t();

            //需要高低字节转换
            //最后需要计算crc
            quint32 t = 70270825;
            //低字节序转高字节序
            t = qbswap<quint32>(t);
            cmd.append((char*)&t,sizeof (t));



            while(!file.atEnd())
            {

                QString content = file.readLine();
                QStringList list = content.split("\t");
                //qDebug()<<list<<list.length();
                if(list.length()<11)continue;
                for (int i =0; i < 5;i++)
                {
                    double X = list.at(i).toDouble();
                    int Y = 0;
                    if(i == 2 || i == 3)
                    {
                        Y =(int)((X+128)*65536/256);
                    }
                    else
                    {
                        Y=(int)((X+180)*65536/360);
                    }
                    cmd.append((Y&0xFF00)>>8);
                    cmd.append((Y&0xFF));
                }
            }
        }
    }
    QByteArray realCmd;
    quint32 cmdLenth = cmd.length();
    cmdLenth = qbswap<quint32>(cmdLenth);
    realCmd.append((char*)&cmdLenth,sizeof (cmdLenth));
    realCmd.append(cmd);
    //开始曲线注入
    FrameAssemble::Instance()->RS422CurveDataSend(realCmd);
}

void frmPagesDataBroadcast::on_pb_begin_cmd_inject_clicked()
{
    //测试
    QString str = "E2 25 0D 2C C0 00 07 F3 00 4D 00 66 AA AA AA AA 80 00 80 00 00 00 00 00 80 00 80 00 00 00 00 00 00 4C 03 D1 80 00 80 00 00 00 00 00 80 00 80 00 00 00 00 00 00 4D 00 02 80 00 80 00 00 00 00 00 80 00 80 00 00 00 00 00 00 4D 00 1B 80 00 80 00 00 00 00 00 80 00 80 00 00 00 00 00 00 4D 00 34 80 00 80 00 00 00 00 00 80 00 80 00 00 00 00 00 00 4D 00 4D 80 00 80 00 00 00 00 00 80 00 80 00 00 00 00 00 00 4D 00 66 80 00 80 00 00 00 00 00 80 00 80 00 00 00 00 00 00 4D 00 7F AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA 83 4F 83 F3 00 4C 03 09 83 4F 83 F3 00 4C 03 09 83 50 83 F3 00 4C 03 09 83 4F 83 F3 00 4C 03 09 83 50 83 F4 00 4C 03 09 83 50 83 F5 00 4C 03 09 83 50 83 F3 00 4C 03 09 83 50 83 F4 00 4C 03 09 83 4F 83 F4 00 4C 03 09 83 4F 83 F4 00 4C 03 09 83 50 83 F4 00 4C 03 09 83 50 83 F5 00 4C 03 09 83 50 83 F4 00 4C 03 09 83 4F 83 F5 00 4C 03 09 83 50 83 F5 00 4C 03 09 83 50 83 F4 00 4C 03 09 83 51 83 F3 00 4C 03 09 83 50 83 F5 00 4C 03 09 83 50 83 F3 00 4C 03 09 83 50 83 F5 00 4C 03 09 83 50 83 F4 00 4C 03 09 83 50 83 F3 00 4C 03 09 83 4F 83 F2 00 4C 03 09 83 4F 83 F3 00 4C 03 09 83 50 83 F2 00 4C 03 09 83 50 83 F1 00 4C 03 09 83 4F 83 F2 00 4C 03 22 83 4F 83 F2 00 4C 03 22 83 50 83 F3 00 4C 03 22 83 4F 83 F2 00 4C 03 22 83 4F 83 F2 00 4C 03 22 83 4F 83 F3 00 4C 03 22 83 4F 83 F2 00 4C 03 22 83 4E 83 F5 00 4C 03 22 83 4F 83 F5 00 4C 03 22 83 4F 83 F3 00 4C 03 22 83 4F 83 F3 00 4C 03 22 83 4F 83 F4 00 4C 03 22 83 4E 83 F4 00 4C 03 22 83 4F 83 F3 00 4C 03 22 83 4F 83 F1 00 4C 03 22 83 50 83 F3 00 4C 03 22 83 4F 83 F3 00 4C 03 22 83 4F 83 F3 00 4C 03 22 83 4F 83 F4 00 4C 03 22 83 4F 83 F3 00 4C 03 22 83 4F 83 F2 00 4C 03 22 83 4F 83 F3 00 4C 03 22 83 4F 83 F3 00 4C 03 22 83 4E 83 F3 00 4C 03 22 83 4E 83 F4 00 4C 03 22 83 4F 83 F4 00 4C 03 3B 83 4F 83 F4 00 4C 03 3B 83 4F 83 F3 00 4C 03 3B 83 50 83 F4 00 4C 03 3B 83 50 83 F5 00 4C 03 3B 83 50 83 F5 00 4C 03 3B 83 51 83 F4 00 4C 03 3B 83 50 83 F5 00 4C 03 3B 83 50 83 F4 00 4C 03 3B 83 50 83 F6 00 4C 03 3B 83 51 83 F5 00 4C 03 3B 83 51 83 F4 00 4C 03 3B 83 4F 83 F3 00 4C 03 3B 83 50 83 F5 00 4C 03 3B 83 50 83 F4 00 4C 03 3B 83 50 83 F4 00 4C 03 3B 83 50 83 F4 00 4C 03 3B 83 50 83 F3 00 4C 03 3B 83 4F 83 F3 00 4C 03 3B 83 4F 83 F3 00 4C 03 3B 83 4F 83 F3 00 4C 03 3B 83 50 83 F1 00 4C 03 3B 83 4E 83 F3 00 4C 03 3B 83 50 83 F3 00 4C 03 3B 83 4F 83 F3 00 4C 03 3B 83 4E 83 F2 00 4C 03 54 83 4E 83 F3 00 4C 03 54 83 4E 83 F3 00 4C 03 54 83 4F 83 F4 00 4C 03 54 83 4F 83 F3 00 4C 03 54 83 4F 83 F2 00 4C 03 54 83 50 83 F3 00 4C 03 54 83 4F 83 F4 00 4C 03 54 83 4F 83 F3 00 4C 03 54 83 50 83 F2 00 4C 03 54 83 4F 83 F3 00 4C 03 54 83 4F 83 F3 00 4C 03 54 83 50 83 F5 00 4C 03 54 83 4F 83 F5 00 4C 03 54 83 4F 83 F4 00 4C 03 54 83 4F 83 F4 00 4C 03 54 83 50 83 F4 00 4C 03 54 83 4F 83 F4 00 4C 03 54 83 4F 83 F4 00 4C 03 54 83 4F 83 F4 00 4C 03 54 83 50 83 F3 00 4C 03 54 83 4F 83 F5 00 4C 03 54 83 50 83 F4 00 4C 03 54 83 50 83 F4 00 4C 03 54 83 50 83 F3 00 4C 03 54 83 50 83 F5 00 4C 03 6D 83 50 83 F5 00 4C 03 6D 83 50 83 F4 00 4C 03 6D 83 50 83 F4 00 4C 03 6D 83 50 83 F3 00 4C 03 6D 83 50 83 F5 00 4C 03 6D 83 50 83 F3 00 4C 03 6D 83 50 83 F4 00 4C 03 6D 83 4F 83 F3 00 4C 03 6D 83 50 83 F4 00 4C 03 6D 83 50 83 F5 00 4C 03 6D 83 4F 83 F5 00 4C 03 6D 83 4F 83 F5 00 4C 03 6D 83 50 83 F3 00 4C 03 6D 83 50 83 F3 00 4C 03 6D 83 4F 83 F2 00 4C 03 6D 83 50 83 F2 00 4C 03 6D 83 50 83 F1 00 4C 03 6D 83 4F 83 F2 00 4C 03 6D 83 4F 83 F2 00 4C 03 6D 83 4F 83 F2 00 4C 03 6D 83 4F 83 F1 00 4C 03 6D 83 4F 83 F1 00 4C 03 6D 83 4F 83 F2 00 4C 03 6D 83 50 83 F3 00 4C 03 6D 83 4F 83 F3 00 4C 03 6D 83 50 83 F3 00 4C 03 86 83 4F 83 F2 00 4C 03 86 83 50 83 F3 00 4C 03 86 83 4F 83 F3 00 4C 03 86 83 50 83 F3 00 4C 03 86 83 4F 83 F3 00 4C 03 86 83 4F 83 F3 00 4C 03 86 83 4F 83 F4 00 4C 03 86 83 4F 83 F4 00 4C 03 86 83 4F 83 F2 00 4C 03 86 83 4F 83 F4 00 4C 03 86 83 4E 83 F4 00 4C 03 86 83 4F 83 F5 00 4C 03 86 83 4F 83 F4 00 4C 03 86 83 4F 83 F3 00 4C 03 86 83 4F 83 F3 00 4C 03 86 83 4F 83 F3 00 4C 03 86 83 50 83 F4 00 4C 03 86 83 50 83 F3 00 4C 03 86 83 4F 83 F2 00 4C 03 86 83 4F 83 F4 00 4C 03 86 83 50 83 F5 00 4C 03 86 83 50 83 F4 00 4C 03 86 83 50 83 F3 00 4C 03 86 83 4F 83 F3 00 4C 03 86 83 4F 83 F5 00 4C 03 9F 83 50 83 F6 00 4C 03 9F 83 50 83 F3 00 4C 03 9F 83 50 83 F4 00 4C 03 9F 83 50 83 F2 00 4C 03 9F 83 50 83 F4 00 4C 03 9F 83 50 83 F4 00 4C 03 9F 83 50 83 F3 00 4C 03 9F 83 4F 83 F4 00 4C 03 9F 83 50 83 F3 00 4C 03 9F 83 4F 83 F5 00 4C 03 9F 83 4E 83 F4 00 4C 03 9F 83 50 83 F1 00 4C 03 9F 83 4F 83 F2 00 4C 03 9F 83 50 83 F3 00 4C 03 9F 83 4F 83 F4 00 4C 03 9F 83 50 83 F2 00 4C 03 9F 83 4E 83 F4 00 4C 03 9F 83 4F 83 F2 00 4C 03 9F 83 4F 83 F5 00 4C 03 9F 83 50 83 F3 00 4C 03 9F 83 50 83 F4 00 4C 03 9F 83 4E 83 F5 00 4C 03 9F 83 50 83 F5 00 4C 03 9F 83 50 83 F6 00 4C 03 9F AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA FB FB";
    QByteArray data;
    data = QByteArray::fromHex(str.toLocal8Bit());
    FrameAssemble::Instance()->LVDSCardDataRecved(0,data);
}

void frmPagesDataBroadcast::on_pb_send_time_clc_clicked()
{

    uint v1 = ui->dt_base_time->dateTime().toTime_t();
    uint v2 = ui->dt_start_time->dateTime().toTime_t();

    if(v1>v2)
    {
        timeDiffValue = v1-v2;
    }
    else
    {
        timeDiffValue = v2-v1;
    }

    ui->lb_time_diff->setText(QString::number(timeDiffValue));

    App::baseTime = v1;
    App::startTime = v2;
    App::writeConfig();
}


void frmPagesDataBroadcast::on_pb_choose_curve_clicked()
{
    curevFileName = QFileDialog::getOpenFileName(nullptr,"打开文件","","txt (*.txt);;* (*.*)");
    //curevFileName = QFileDialog::getExistingDirectory(this, tr("选择文件保存路径"), "./", QFileDialog::ShowDirsOnly);
    if(curevFileName.isEmpty())return;
    ui->le_curve_path->setText(curevFileName);
}

void frmPagesDataBroadcast::on_pb_choose_trace_clicked()
{
    QString filter =
        "数据文件 (*.txt *.csv);;"
        "文本文件 (*.txt);;"
        "CSV 文件 (*.csv);;"
        "所有文件 (*.*)";  // 多个过滤器用 ;; 分隔
    traceFileName = QFileDialog::getOpenFileName(this,"打开文件","",filter);
    //curevFileName = QFileDialog::getExistingDirectory(this, tr("选择文件保存路径"), "./", QFileDialog::ShowDirsOnly);
    if(traceFileName.isEmpty())return;
    ui->le_trace_path->setText(traceFileName);
}

void frmPagesDataBroadcast::on_pb_choose_satellite_clicked()
{
    QString filter =
        "数据文件 (*.txt *.csv);;"
        "文本文件 (*.txt);;"
        "CSV 文件 (*.csv);;"
        "所有文件 (*.*)";  // 多个过滤器用 ;; 分隔
    satelliteAttiFileName = QFileDialog::getOpenFileName(this,"打开文件","",filter);
    //curevFileName = QFileDialog::getExistingDirectory(this, tr("选择文件保存路径"), "./", QFileDialog::ShowDirsOnly);
    if(satelliteAttiFileName.isEmpty())return;
    ui->lineEdit_satellite->setText(satelliteAttiFileName);
}

void frmPagesDataBroadcast::on_pb_choose_sunVector_clicked()
{
    QString filter =
        "数据文件 (*.txt *.csv);;"
        "文本文件 (*.txt);;"
        "CSV 文件 (*.csv);;"
        "所有文件 (*.*)";  // 多个过滤器用 ;; 分隔
    sunVectorFileName = QFileDialog::getOpenFileName(this,"打开文件","",filter);
    //curevFileName = QFileDialog::getExistingDirectory(this, tr("选择文件保存路径"), "./", QFileDialog::ShowDirsOnly);
    if(sunVectorFileName.isEmpty())return;
    ui->lineEdit_sun_vector->setText(sunVectorFileName);
}

void frmPagesDataBroadcast::on_pb_clear_te_time_clicked()
{
    ui->te_time_send->clear();
}

void frmPagesDataBroadcast::on_pb_clear_te_gps_clicked()
{
    ui->te_gps_data_show->clear();
}

void frmPagesDataBroadcast::on_pb_clear_sun_vector_clicked()
{
    ui->sun_vector_data_show->clear();
}

void frmPagesDataBroadcast::on_pb_clear_satellite_attitude_clicked()
{
    ui->satellite_attitude_data_show->clear();
}

void frmPagesDataBroadcast::startSendTraceData()
{
    //判断文件是否存在
    traceFileHander.setFileName(traceFileName);
    //文件存在且大小不为0
    if(traceFileHander.size() == 0)
    {
        QMessageBox::information(this,"通知","选择的文件为空或者文件不存在!");
        return;
    }

    //开启定时器

    if(traceFileHander.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        //每次打开文件先去读一行的头部信息
        QString title  = traceFileHander.readLine();
    }
    else
    {
        QMessageBox::information(this,"通知","文件打开失败!");
        return;
    }

    if(traceDataSendTimer)traceDataSendTimer->start();

//    ui->pb_send_satellite_location->setText("停止广播");
    showSendTraceMsg("轨道数据开始广播!");
}

void frmPagesDataBroadcast::stopSendTraceData()
{
    if(traceDataSendTimer)traceDataSendTimer->stop();
//    ui->pb_send_satellite_location->setText("开始广播");
    if(traceFileHander.isOpen())traceFileHander.close();
}
