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

    dataTransfer  = new DataTransfer(this);
    connect(ui->pushButton_resetBroadCount,&QPushButton::clicked,this,&frmPagesDataBroadcast::resetBroadCount);

    // 定轨结果 2bit
    ui->comboBox_orbitingResult->addItem("定轨无效", 0);
    ui->comboBox_orbitingResult->addItem("定轨有效", 2);
    ui->comboBox_orbitingResult->addItem("定轨外推有效", 3);
    ui->comboBox_orbitingResult->setCurrentIndex(0);
    // 定位结果 1bit
    ui->comboBox_locatingResult->addItem("定位无效",0);
    ui->comboBox_locatingResult->addItem("定位有效",1);
    ui->comboBox_locatingResult->setCurrentIndex(0);
    // 解算模式 2bit
    ui->comboBox_solvingMode->addItem("初始态", 0);
    ui->comboBox_solvingMode->addItem("单GPS", 1);
    ui->comboBox_solvingMode->addItem("单BDS", 2);
    ui->comboBox_solvingMode->addItem("GPS+BDS", 3);
    ui->comboBox_solvingMode->setCurrentIndex(3);
    // 定轨模式 2bit
    ui->comboBox_orbitingMode->addItem("初始态", 0);
    ui->comboBox_orbitingMode->addItem("单GPS", 1);
    ui->comboBox_orbitingMode->addItem("单BDS", 2);
    ui->comboBox_orbitingMode->addItem("GPS+BDS", 3);
    ui->comboBox_orbitingMode->setCurrentIndex(3);
    // PPS状态 1bit
    ui->comboBox_PPSStatus->addItem("PPS无效",0);
    ui->comboBox_PPSStatus->addItem("PPS有效",1);
    ui->comboBox_PPSStatus->setCurrentIndex(0);

    // 四元数有效标志 1bit
    ui->comboBox_quaternion->addItem("无效", 0);
    ui->comboBox_quaternion->addItem("有效", 1);
    ui->comboBox_quaternion->setCurrentIndex(0);
    // 姿态角速度有效标志 1bit
    ui->comboBox_attiAnguVelo->addItem("无效",0);
    ui->comboBox_attiAnguVelo->addItem("有效",1);
    ui->comboBox_attiAnguVelo->setCurrentIndex(0);
    // 太阳矢量有效标志 1bit
    ui->comboBox_sunVector->addItem("无效",0);
    ui->comboBox_sunVector->addItem("有效",1);
    ui->comboBox_sunVector->setCurrentIndex(0);
    // 星历卫星1有效标志 1bit
    ui->comboBox_ephemeris1->addItem("无效",0);
    ui->comboBox_ephemeris1->addItem("有效",1);
    ui->comboBox_ephemeris1->setCurrentIndex(0);
    // 星历卫星2有效标志 1bit
    ui->comboBox_ephemeris2->addItem("无效",0);
    ui->comboBox_ephemeris2->addItem("有效",1);
    ui->comboBox_ephemeris2->setCurrentIndex(0);
    // 保留 3bit
    ui->comboBox_retain->addItem("保留",0);
    ui->comboBox_retain->setCurrentIndex(0);

}

frmPagesDataBroadcast::~frmPagesDataBroadcast()
{
    delete ui;
}

void frmPagesDataBroadcast::initForms()
{
    sendTimeTimer = new QTimer(this);
    sendTimeTimer->setTimerType(Qt::TimerType::PreciseTimer);
    int timeDelayTime = ui->lineEdit_timeDelayTime->text().toInt();
    sendTimeTimer->setInterval(timeDelayTime);
    connect(sendTimeTimer,&QTimer::timeout,this,&frmPagesDataBroadcast::sendTimerTimeout);

    traceDataSendTimer = new QTimer(this);
    traceDataSendTimer->setTimerType(Qt::TimerType::PreciseTimer);
    int timeDelayTrace = ui->lineEdit_timeDelayTrace->text().toInt();
    traceDataSendTimer->setInterval(timeDelayTrace);
    connect(traceDataSendTimer,&QTimer::timeout,this,&frmPagesDataBroadcast::sendTimerTimeout_trace);

    sunSendTimer = new QTimer(this);
    sunSendTimer->setTimerType(Qt::TimerType::PreciseTimer);
    int timeDelaySun = ui->lineEdit_timeDelaySun->text().toInt();
    sunSendTimer->setInterval(timeDelaySun);
    connect(sunSendTimer,&QTimer::timeout,this,&frmPagesDataBroadcast::sendTimerTimeout_sun);

    satelliteSendTimer = new QTimer(this);
    satelliteSendTimer->setTimerType(Qt::TimerType::PreciseTimer);
    int timeDelayAttitude = ui->lineEdit_timeDelayAttitude->text().toInt();
    satelliteSendTimer->setInterval(timeDelayAttitude); // 2026.4.2修改 姿控组播需要4Hz
    connect(satelliteSendTimer,&QTimer::timeout,this,&frmPagesDataBroadcast::sendTimerTimeout_attitude);

}

// 从头开始读函数
void frmPagesDataBroadcast::resetCsvPosition()
{
    if(timeFileHander.isOpen()){
        timeFileHander.seek(0);
        timeFileHander.readLine(); // 跳表头
    }
    if(traceFileHander.isOpen()){
        traceFileHander.seek(0);
        traceFileHander.readLine(); // 跳表头
    }
    if(sunFileHander.isOpen()){
        sunFileHander.seek(0);
        sunFileHander.readLine(); // 跳表头
    }
    if(satelliteFileHander.isOpen()){
        satelliteFileHander.seek(0);
        satelliteFileHander.readLine(); // 跳表头
    }
}

void frmPagesDataBroadcast::sendTimerTimeout()
{
    // A：手动输入模式
    QString tempTimeEmit = ui->te_time_send->toPlainText();
    QByteArray tempTimeEmitArray  = dataTransfer->string2Bytearray(tempTimeEmit);
    bool useCsv = ui->checkBox_timeCsvBroad->isChecked();
    bool isEnabled = ui->checkBox_addBoardTime->isChecked();

    if(!useCsv && isEnabled && !tempTimeEmitArray.trimmed().isEmpty()){
        emit timeBroadcastSignal(tempTimeEmitArray);
        return;
    }

    // B：CSV模式
    if(useCsv && isEnabled){
        // 1. 检查文件是否已经打开，如果没有则尝试打开
        if(!timeFileHander.isOpen()){
            if (timeFileHander.fileName().isEmpty() || !timeFileHander.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "错误", "无法打开CSV文件，请检查路径！");
                // 停止定时器防止连续弹窗
                if(sendTimeTimer->isActive()) sendTimeTimer->stop();
                return;
            }
            // 如果 CSV 有表头，可以在这里跳过第一行
             timeFileHander.readLine(); // 跳过表头
        }
    }

    // 2. 读取下一行
    if (!timeFileHander.atEnd()) {
        QString line = timeFileHander.readLine().trimmed();
        if(!line.isEmpty()){
            // 假设 CSV 是以逗号或 Tab 分割，取你需要的那一列
            QStringList dataList = line.split(",");
            // 从dataList中取值进行解析并生成帧数据
            // 在这里调用一个定义的函数，这个函数可以实现输入dataList，输出一个QByteArray格式的帧数据，然后将这个帧数据加载到信号中发送出去
            QByteArray broadFrameArray = outputBroadFrameTime(dataList);
            emit timeBroadcastSignal(broadFrameArray);
        }
    } else {
        // 3. 文件读完了
        qDebug() << "文件读取完毕";
        timeFileHander.close(); // 关闭文件
        if(sendTimeTimer->isActive()) sendTimeTimer->stop(); // 停止发送
        QMessageBox::information(this, "通知", "文件数据发送完毕！");
    }
}

void frmPagesDataBroadcast::sendTimerTimeout_attitude(){

    // A：手动输入模式
    QString tempAttitudeEmit = ui->satellite_attitude_data_show->toPlainText();
    QByteArray tempAttitudeEmitArray  = dataTransfer->string2Bytearray(tempAttitudeEmit);
    bool useCsv = ui->checkBox_addBroad_satellite->isChecked();
    bool isEnabled = ui->checkBox_attitudeCsvBroad->isChecked();

    if(!useCsv && isEnabled && !tempAttitudeEmitArray.trimmed().isEmpty()){
        emit attitudeBroadcastSignal(tempAttitudeEmitArray);
        return;
    }

    // B：CSV模式
    if(useCsv && isEnabled){
        // 1. 检查文件是否已经打开，如果没有则尝试打开
        if(!satelliteFileHander.isOpen()){
            if (satelliteFileHander.fileName().isEmpty() || !satelliteFileHander.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "错误", "无法打开CSV文件，请检查路径！");
                // 停止定时器防止连续弹窗
                if(satelliteSendTimer->isActive()) satelliteSendTimer->stop();
                return;
            }
            // 如果 CSV 有表头，可以在这里跳过第一行
            satelliteFileHander.readLine(); // 跳过表头
        }
    }

    // 2. 读取下一行
    if (!satelliteFileHander.atEnd()) {
        QString line = satelliteFileHander.readLine().trimmed();
        if(!line.isEmpty()){
            // 假设 CSV 是以逗号或 Tab 分割，取你需要的那一列
            QStringList dataList = line.split(",");
            // 从dataList中取值进行解析并生成帧数据
            // 在这里调用一个定义的函数，这个函数可以实现输入dataList，输出一个QByteArray格式的帧数据，然后将这个帧数据加载到信号中发送出去
            QByteArray broadFrameArray = outputBroadFrameAttitude(dataList);
            emit attitudeBroadcastSignal(broadFrameArray);
        }
    } else {
        // 3. 文件读完了
        qDebug() << "文件读取完毕";
        satelliteFileHander.close(); // 关闭文件
        if(satelliteSendTimer->isActive()) satelliteSendTimer->stop(); // 停止发送
        QMessageBox::information(this, "通知", "文件数据发送完毕！");
    }

}

void frmPagesDataBroadcast::sendTimerTimeout_trace()
{

    QString tempTraceBroadcast = ui->te_gps_data_show->toPlainText();

    if(!ui->checkBox_traceCsvBroad->isChecked() && ui->checkBox_addBoardTrace->isChecked()&& !tempTraceBroadcast.trimmed().isEmpty()){
        QByteArray tempTraceBroadcastArray  = dataTransfer->string2Bytearray(tempTraceBroadcast);
        emit traceBroadcastSignal(tempTraceBroadcastArray);
        return;
    }

    if(ui->checkBox_addBoardTrace->isChecked()){
        // 更换为从csv读入，检查当前是否有文件载入，如果没有，return
        qDebug()<<"业务组播切换为从csv文件中读取数据";
        if(traceFileHander.open(QIODevice::Text | QIODevice::ReadOnly))
        {
            //每次打开文件先去读一行的头部信息
            QString title  = traceFileHander.readLine();
            qDebug()<<title;
        }
        else
        {
            QMessageBox::information(this,"通知","文件打开失败!");
            return;
        }
    }

}

void frmPagesDataBroadcast::sendTimerTimeout_sun()
{

    QString tempSunBroadcast = ui->sun_vector_data_show->toPlainText();
    if(!ui->checkBox_sunCsvBroad->isChecked()&& ui->checkBox_addBroadSun->isChecked()&& !tempSunBroadcast.trimmed().isEmpty()){
        QByteArray tempSunBroadcastArray  = dataTransfer->string2Bytearray(tempSunBroadcast);
        emit traceBroadcastSignal(tempSunBroadcastArray);
        return;
    }
    if(ui->checkBox_addBroadSun->isChecked()){
        // 更换为从csv读入，检查当前是否有文件载入，如果没有，return
        qDebug()<<"太阳广播切换为从csv文件中读取数据";\
        if(sunFileHander.open(QIODevice::Text | QIODevice::ReadOnly))
        {
            //每次打开文件先去读一行的头部信息
            QString title  = sunFileHander.readLine();
            qDebug()<<title;
        }
        else
        {
            QMessageBox::information(this,"通知","文件打开失败!");
            return;
        }
    }

}

//void frmPagesDataBroadcast::sendTraceDataTimerTimeout()
//{
//    //读取文件一行然后
//    if(!traceFileHander.fileName().isEmpty() && traceFileHander.isOpen())
//    {
//        //文件还有内容继续发送
//        if(!traceFileHander.atEnd())
//        {
//            QString content = traceFileHander.readLine();
//            QStringList dataList = content.split("\t");
//            if(dataList.length()>=8)
//            {
//                //转换数据并组装cmd进行发送

//                //当前轨道模式：
//                //0x01：注入轨道
//                //0x02：GNSS定位轨道
//                //0x04：无轨道

//                quint8 orbitMode = 0;

//                int index = ui->cb_trace_mode->currentIndex();
//                orbitMode = index+1;
//                if(orbitMode == 3)orbitMode = 4;


//                //开始处理文件

//                float x = dataList.at(0).toFloat();
//                float y = dataList.at(1).toFloat();
//                float z = dataList.at(2).toFloat();
//                float Vx = dataList.at(3).toFloat();
//                float Vy = dataList.at(4).toFloat();
//                float Vz = dataList.at(5).toFloat();
//                quint32 Second = dataList.at(6).toUInt();
//                quint16 ms = dataList.at(7).toUShort();
//                //发送卫星位置数据
//                QString msg = QString("发送数据 X:%1 Y:%2 Z:%3 Vx:%4 Vy:%5 Vz:%6 模式:%7")
//                        .arg(x)
//                        .arg(y)
//                        .arg(z)
//                        .arg(Vx)
//                        .arg(Vy)
//                        .arg(Vz)
//                        .arg(ui->cb_trace_mode->currentText());
//                showSendTraceMsg(msg);
//                FrameAssemble::Instance()->RS422SatelliteOrbitDataSend(orbitMode,Second,ms,x,y,z,Vx,Vy,Vz);
//            }
//        }
//        //到达行尾结束
//        else
//        {
//            stopSendTraceData();
//            showSendTraceMsg("轨道数据广播完毕!");
//        }
//    }
//}

void frmPagesDataBroadcast::showSendTimeMsg(QString msg)
{
    static int count = 0;
    count++;
    if(count>5000)
    {
         ui->plainTextEdit_timeStatusShow->clear();
         count = 0;
    }
    QString text = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]:")+msg;
    ui->plainTextEdit_timeStatusShow->appendPlainText(text);
}

void frmPagesDataBroadcast::showSendTraceMsg(QString msg)
{

    static int count = 0;
    count++;
    if(count>5000)
    {
         ui->plainTextEdit_traceStatusShow->clear();
         count = 0;
    }
    QString text = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]:")+msg;
    ui->plainTextEdit_traceStatusShow->appendPlainText(text);
}

void frmPagesDataBroadcast::showSendSunMsg(QString msg){
    static int count = 0;
    count++;
    if(count>5000)
    {
         ui->plainTextEdit_sunStatusShow->clear();
         count = 0;
    }
    QString text = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]:")+msg;
    ui->plainTextEdit_sunStatusShow->appendPlainText(text);
}

void frmPagesDataBroadcast::showSendAttitudeMsg(QString msg){
    static int count = 0;
    count++;
    if(count>5000)
    {
         ui->plainTextEdit_attitudeStatusShow->clear();
         count = 0;
    }
    QString text = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]:")+msg;
    ui->plainTextEdit_attitudeStatusShow->appendPlainText(text);
}

void frmPagesDataBroadcast::updateBroadCount(int updateWhichBroadCount)
{
    switch (updateWhichBroadCount) {
        case 0:
            timeCount += 1;
            ui->label_timeBroadCount->setText(QString::number(timeCount));
            break;
        case 1:
            satallite+=1;
            ui->label_satalliteBroadCount->setText(QString::number(satallite));
            break;
        case 2:
            traceCount+=1;
            ui->label_traceBroadCount->setText(QString::number(traceCount));
            break;
        case 3:
            sunCount+=1;
            ui->label_sunBroadCount->setText(QString::number(sunCount));
            break;
        default:
            break;
    }
}

QCheckBox *frmPagesDataBroadcast::getSatelliteCheckBox()
{
    return ui->checkBox_addBroad_satellite;
}

void frmPagesDataBroadcast::resetBroadCount()
{
    timeCount = 0;
    traceCount = 0;
    satallite = 0;
    sunCount = 0;

    ui->label_timeBroadCount->setText("0");
    ui->label_traceBroadCount->setText("0");
    ui->label_satalliteBroadCount->setText("0");
    ui->label_sunBroadCount->setText("0");
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

void frmPagesDataBroadcast::on_pb_broadcastStart_clicked()
{

    if(ui->pb_broadcastStart->text()=="开始广播")
    {
        if(ui->checkBox_backToHead->isChecked()){
            resetCsvPosition(); // 重新从头读取csv文件数据
        }

        if(sendTimeTimer && ui->checkBox_addBoardTime->isChecked())
        {
            sendTimeTimer->stop();
            sendTimeTimer->start();
            showSendTimeMsg("时间数据开始广播!");
        }
        if(traceDataSendTimer && ui->checkBox_addBoardTrace->isChecked())
        {
            traceDataSendTimer->stop();
            traceDataSendTimer->start();
            showSendTraceMsg("轨道数据开始广播!");
        }
        if(satelliteSendTimer && ui->checkBox_addBroad_satellite->isChecked())
        {
            satelliteSendTimer->stop();
            satelliteSendTimer->start();
            showSendAttitudeMsg("姿控数据开始广播!");
        }
        if(sunSendTimer && ui->checkBox_addBroadSun->isChecked())
        {
            sunSendTimer->stop();
            sunSendTimer->start();
            showSendSunMsg("太阳矢量数据开始广播!");
        }
        ui->pb_broadcastStart->setText("停止广播");
    }
    else
    {
        ui->pb_broadcastStart->setText("开始广播");
        if(sendTimeTimer)
        {
            sendTimeTimer->stop();
        }
        if(traceDataSendTimer)
        {
            traceDataSendTimer->stop();
        }
        if(satelliteSendTimer)
        {
            satelliteSendTimer->stop();
        }
        if(sunSendTimer)
        {
            sunSendTimer->stop();
        }
    }
}

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

//void frmPagesDataBroadcast::on_pb_send_time_clc_clicked()
//{

//    uint v1 = ui->dt_base_time->dateTime().toTime_t();
//    uint v2 = ui->dt_start_time->dateTime().toTime_t();

//    if(v1>v2)
//    {
//        timeDiffValue = v1-v2;
//    }
//    else
//    {
//        timeDiffValue = v2-v1;
//    }

//    ui->lb_time_diff->setText(QString::number(timeDiffValue));

//    App::baseTime = v1;
//    App::startTime = v2;
//    App::writeConfig();
//}


void frmPagesDataBroadcast::on_pb_choose_time_clicked()
{
    QString filter =
        "数据文件 (*.txt *.csv);;"
        "文本文件 (*.txt);;"
        "CSV 文件 (*.csv);;"
        "所有文件 (*.*)";  // 多个过滤器用 ;; 分隔
    timeFileName = QFileDialog::getOpenFileName(this,"打开文件","",filter);
    //timeFileName = QFileDialog::getExistingDirectory(this, tr("选择文件保存路径"), "./", QFileDialog::ShowDirsOnly);
    if(timeFileName.isEmpty())return;
    ui->le_time_path->setText(timeFileName);
    timeFileHander.setFileName(timeFileName);
    //文件存在且大小不为0
    if(timeFileHander.size() == 0)
    {
        QMessageBox::information(this,"通知","选择的文件为空或者文件不存在!");
        return;
    }
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
    traceFileHander.setFileName(traceFileName);
    //文件存在且大小不为0
    if(traceFileHander.size() == 0)
    {
        QMessageBox::information(this,"通知","选择的文件为空或者文件不存在!");
        return;
    }
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
    satelliteFileHander.setFileName(satelliteAttiFileName);
    //文件存在且大小不为0
    if(satelliteFileHander.size() == 0)
    {
        QMessageBox::information(this,"通知","选择的文件为空或者文件不存在!");
        return;
    }
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
    sunFileHander.setFileName(sunVectorFileName);
    //文件存在且大小不为0
    if(sunFileHander.size() == 0)
    {
        QMessageBox::information(this,"通知","选择的文件为空或者文件不存在!");
        return;
    }
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

void frmPagesDataBroadcast::on_pb_clear_te_time2_clicked()
{
    ui->plainTextEdit_timeStatusShow->clear();
}

void frmPagesDataBroadcast::on_pb_clear_te_gps2_clicked()
{
    ui->plainTextEdit_traceStatusShow->clear();
}

void frmPagesDataBroadcast::on_pb_clear_sun_vector2_clicked()
{
    ui->plainTextEdit_sunStatusShow->clear();
}

void frmPagesDataBroadcast::on_pb_clear_satellite_attitude2_clicked()
{
    ui->plainTextEdit_attitudeStatusShow->clear();
}

QByteArray frmPagesDataBroadcast::outputBroadFrameTime(QStringList csvLine)
{
    // 根据协议生成时间广播帧
    for (int i = 0 ; i < csvLine.size(); i++) {
       qDebug()<<"单行数据：" <<csvLine.at(i);
    }

    // 指令码
    uint16_t comCode = 0x00FF;
    QByteArray comCodeArray;
    comCodeArray.append(dataTransfer->uintTo_2_Bytes(comCode));

    // 系统时间累积整秒 无符号整型
    QString systemTimeSecondString = csvLine.at(0); // int 4字节
    uint32_t systemTimeSecond = systemTimeSecondString.toUInt();
    QByteArray sysTimeSecArray;
    sysTimeSecArray.append(dataTransfer->uintTo_4_Bytes(systemTimeSecond));

    // 系统时间秒内毫秒计数
    QString systemTimeSecondMsCountString = csvLine.at(1); // int 2字节
    uint16_t systemTimeSecondMsCount = static_cast<uint16_t>(systemTimeSecondMsCountString.toUInt());
    QByteArray sysTimeSecMsCountArray;
    sysTimeSecMsCountArray.append(dataTransfer->uintTo_2_Bytes(systemTimeSecondMsCount));

    // UTC时间累积整秒
    QString UTCTimeSumString = csvLine.at(2); //int 4字节
    uint32_t UTCTimeSum = UTCTimeSumString.toUInt();
    QByteArray UTCTimeSumArray;
    UTCTimeSumArray.append(dataTransfer->uintTo_4_Bytes(UTCTimeSum));

    // 定轨定位有效标识
    uint8_t validIdentifier = 0; // 组装 1字节
    uint8_t orbitingResult = static_cast<uint8_t>(ui->comboBox_orbitingResult->currentData().toUInt()) ;
    uint8_t locatingResult = static_cast<uint8_t>(ui->comboBox_locatingResult->currentData().toUInt()) ;
    uint8_t solvingMode = static_cast<uint8_t>(ui->comboBox_solvingMode->currentData().toUInt()) ;
    uint8_t orbitingMode = static_cast<uint8_t>(ui->comboBox_orbitingMode->currentData().toUInt()) ;
    uint8_t PPSStatus = static_cast<uint8_t>(ui->comboBox_PPSStatus->currentData().toUInt()) ;
    validIdentifier |= (orbitingResult & 0x03) << 6;
    validIdentifier |= (locatingResult & 0x01) << 5;
    validIdentifier |= (solvingMode & 0x03) << 3;
    validIdentifier |= (orbitingMode & 0x03) << 1;
    validIdentifier |= PPSStatus & 0x01;
    QByteArray validIdenArray;
    validIdenArray.append(dataTransfer->uintTo_1_Bytes(validIdentifier));

    // 在这里插入 4 * 12 字节的数据，从GNSS定位X位置到GNSS定位位置Z速度
    QByteArray placeHolder1;
    placeHolder1.append(QByteArray(48,0xFF));

    // 6个位置
    QString Px = csvLine.at(3); // 有符号整形——>二进制补码 4字节
    double PxDouble = Px.toDouble();
    qDebug()<<"PxDouble"<<PxDouble;
    qint64 PxValue = static_cast<qint64>(PxDouble); // 只要整数部分
    QString PxTrans = dataTransfer->decimalToHexSigned(QString::number(PxValue),32);
    qDebug()<<"PxTrans"<<PxTrans;
    QByteArray PxArray = dataTransfer->string2Bytearray(PxTrans);
    qDebug()<<"PxArray"<<PxArray.toHex(' ').toUpper();

    QString Py = csvLine.at(4); // 有符号整形——>二进制补码 4字节
    double PyDouble = Py.toDouble();
    qint64 PyValue = static_cast<qint64>(PyDouble); // 只要整数部分
    QString PyTrans = dataTransfer->decimalToHexSigned(QString::number(PyValue),32);
    QByteArray PyArray = dataTransfer->string2Bytearray(PyTrans);
    qDebug()<<"PyArray"<<PyArray.toHex(' ').toUpper();

    QString Pz = csvLine.at(5); // 有符号整形——>二进制补码 4字节
    double PzDouble = Pz.toDouble();
    qint64 PzValue = static_cast<qint64>(PzDouble); // 只要整数部分
    QString PzTrans = dataTransfer->decimalToHexSigned(QString::number(PzValue),32);
    QByteArray PzArray = dataTransfer->string2Bytearray(PzTrans);
    qDebug()<<"PzArray"<<PzArray.toHex(' ').toUpper();

    QString Vx = csvLine.at(6); // 有符号整形——>二进制补码 4字节
    double VxDouble = Vx.toDouble();
    qint64 VxValue = static_cast<qint64>(VxDouble); // 只要整数部分
    QString VxTrans = dataTransfer->decimalToHexSigned(QString::number(VxValue),32);
    QByteArray VxArray = dataTransfer->string2Bytearray(VxTrans);
    qDebug()<<"VxArray"<<VxArray.toHex(' ').toUpper();

    QString Vy = csvLine.at(7); // 有符号整形——>二进制补码 4字节
    double VyDouble = Vy.toDouble();
    qint64 VyValue = static_cast<qint64>(VyDouble); // 只要整数部分
    QString VyTrans = dataTransfer->decimalToHexSigned(QString::number(VyValue),32);
    QByteArray VyArray = dataTransfer->string2Bytearray(VyTrans);
    qDebug()<<"VyArray"<<VyArray.toHex(' ').toUpper();

    QString Vz = csvLine.at(8); // 有符号整形——>二进制补码 4字节
    double VzDouble = Vz.toDouble();
    qint64 VzValue = static_cast<qint64>(VzDouble); // 只要整数部分
    QString VzTrans = dataTransfer->decimalToHexSigned(QString::number(VzValue),32);
    QByteArray VzArray = dataTransfer->string2Bytearray(VzTrans);
    qDebug()<<"VzArray"<<VzArray.toHex(' ').toUpper();


    // 在这里插入 4 * 15 字节的数据，从GNSS定位X位置到WGS-84 系椭球高度
    QByteArray placeHolder2;
    placeHolder2.append(QByteArray(60,0xFF));

    // 开始组装1
    QByteArray broadFrameArray;
    broadFrameArray.append(comCodeArray);
    broadFrameArray.append(sysTimeSecArray);
    broadFrameArray.append(sysTimeSecMsCountArray);
    broadFrameArray.append(UTCTimeSumArray);

    broadFrameArray.append(validIdenArray);
    broadFrameArray.append(placeHolder1);

    broadFrameArray.append(PxArray);
    broadFrameArray.append(PyArray);
    broadFrameArray.append(PzArray);
    broadFrameArray.append(VxArray);
    broadFrameArray.append(VyArray);
    broadFrameArray.append(VzArray);

    broadFrameArray.append(placeHolder2);

    qDebug()<<"一帧时间广播的长度："<<broadFrameArray.size();
//    qDebug()<<"一帧时间广播："<<broadFrameArray.toHex(' ').toUpper();


    QString msg = QString("发送数据 X:%1 Y:%2 Z:%3 Vx:%4 Vy:%5 Vz:%6")
            .arg(Px)
            .arg(Py)
            .arg(Pz)
            .arg(Vx)
            .arg(Vy)
            .arg(Vz);
    showSendTimeMsg(msg);

    return broadFrameArray;
}

QByteArray frmPagesDataBroadcast::outputBroadFrameTrace(QStringList csvLine)
{
    // 根据协议生成轨道广播帧
    for (int i = 0 ; i < csvLine.size(); i++) {
       qDebug()<<"单行数据：" <<csvLine.at(i);
    }

}

QByteArray frmPagesDataBroadcast::outputBroadFrameSun(QStringList csvLine)
{
    // 根据协议生成太阳矢量帧
    for (int i = 0 ; i < csvLine.size(); i++) {
       qDebug()<<"单行数据：" <<csvLine.at(i);
    }

}

QByteArray frmPagesDataBroadcast::outputBroadFrameAttitude(QStringList csvLine)
{
    // 根据协议生成姿控组播帧
    for (int i = 0 ; i < csvLine.size(); i++) {
       qDebug()<<"单行数据：" <<csvLine.at(i);
    }
    // 指令码
    uint16_t comCode = 0x007F;
    QByteArray comCodeArray;
    comCodeArray.append(dataTransfer->uintTo_2_Bytes(comCode));

    // 姿控时间整秒 无符号整型
    QString systemTimeSecondString = csvLine.at(0); // int 4字节
    uint32_t systemTimeSecond = systemTimeSecondString.toUInt();
    QByteArray sysTimeSecArray;
    sysTimeSecArray.append(dataTransfer->uintTo_4_Bytes(systemTimeSecond));

    // 姿控秒内毫秒计数
    QString systemTimeSecondMsCountString = csvLine.at(1); // int 2字节
    uint16_t systemTimeSecondMsCount = static_cast<uint16_t>(systemTimeSecondMsCountString.toUInt());
    QByteArray sysTimeSecMsCountArray;
    sysTimeSecMsCountArray.append(dataTransfer->uintTo_2_Bytes(systemTimeSecondMsCount));

    // 数据有效性标识
    uint8_t validIdentifier = 0; // 组装 1字节
    uint8_t quaternion = static_cast<uint8_t>(ui->comboBox_quaternion->currentData().toUInt()) ;
    uint8_t attiAnguVelo = static_cast<uint8_t>(ui->comboBox_attiAnguVelo->currentData().toUInt()) ;
    uint8_t sunVector = static_cast<uint8_t>(ui->comboBox_sunVector->currentData().toUInt()) ;
    uint8_t ephemeris1 = static_cast<uint8_t>(ui->comboBox_ephemeris1->currentData().toUInt()) ;
    uint8_t ephemeris2 = static_cast<uint8_t>(ui->comboBox_ephemeris2->currentData().toUInt()) ;
    uint8_t retain = static_cast<uint8_t>(ui->comboBox_retain->currentData().toUInt()) ;

    validIdentifier |= (quaternion & 0x01) << 7;
    validIdentifier |= (attiAnguVelo & 0x01) << 6;
    validIdentifier |= (sunVector & 0x01) << 5;
    validIdentifier |= (ephemeris1 & 0x01) << 4;
    validIdentifier |= (ephemeris2 & 0x01) <<3;
    validIdentifier |= retain & 0x03;
    QByteArray validIdenArray;
    validIdenArray.append(dataTransfer->uintTo_1_Bytes(validIdentifier));

    // 4个四元数 每个4字节
    QString q0 = csvLine.at(2);
    float q0Float = q0.toFloat();
    qDebug()<<"q0Float"<<q0Float;
    QByteArray q0Array;
    q0Array.append(dataTransfer->floatToByteArray(q0Float,4));
    qDebug()<<"q0Float"<<q0Array.toHex(' ').toUpper();

    QString q1 = csvLine.at(3);
    float q1Float = q1.toFloat();
    qDebug()<<"q1Float"<<q1Float;
    QByteArray q1Array;
    q1Array.append(dataTransfer->floatToByteArray(q1Float,4));
    qDebug()<<"q1Float"<<q0Array.toHex(' ').toUpper();

    QString q2 = csvLine.at(4);
    float q2Float = q2.toFloat();
    qDebug()<<"q2Float"<<q2Float;
    QByteArray q2Array;
    q2Array.append(dataTransfer->floatToByteArray(q2Float,4));
    qDebug()<<"q2Float"<<q0Array.toHex(' ').toUpper();

    QString q3 = csvLine.at(5);
    float q3Float = q3.toFloat();
    qDebug()<<"q3Float"<<q3Float;
    QByteArray q3Array;
    q3Array.append(dataTransfer->floatToByteArray(q3Float,4));
    qDebug()<<"q3Float"<<q0Array.toHex(' ').toUpper();



    // 插入4*3字节占位符 姿态角速度Wy到姿态角速度Wz 每个4字节
    QByteArray placeHolder1;
    placeHolder1.append(QByteArray(12,0xFF));

    // 3个太阳矢量 ，每个4字节
    QString sunX = csvLine.at(6);
    float sunXFloat = sunX.toFloat();
    QByteArray sunXArray;
    sunXArray.append(dataTransfer->floatToByteArray(sunXFloat,4));

    QString sunY = csvLine.at(7);
    float sunYFloat = sunY.toFloat();
    QByteArray sunYArray;
    sunYArray.append(dataTransfer->floatToByteArray(sunYFloat,4));

    QString sunZ = csvLine.at(8);
    float sunZFloat = sunZ.toFloat();
    QByteArray sunZArray;
    sunZArray.append(dataTransfer->floatToByteArray(sunZFloat,4));
    qDebug()<<"sunZArray"<<sunZArray.toHex(' ').toUpper();

    // 星历卫星1标识 lineEdit 正X向 2字节
    QString ephemeris1String =ui->lineEdit_ephemeris1->text();
    QByteArray ephemeris1Array;
    ephemeris1Array.append(dataTransfer->string2Bytearray(ephemeris1String));
    qDebug()<<"ephemeris1Array"<<ephemeris1Array.toHex(' ').toUpper();

    // 6个星历卫星1 ，每个4字节
    QString Px = csvLine.at(9); // 有符号整形——>二进制补码 4字节
    double PxDouble = Px.toDouble();
    qint64 PxValue = static_cast<qint64>(PxDouble); // 只要整数部分
    QString PxTrans = dataTransfer->decimalToHexSigned(QString::number(PxValue),32);
    QByteArray PxArray = dataTransfer->string2Bytearray(PxTrans);


    QString Py = csvLine.at(10); // 有符号整形——>二进制补码 4字节
    double PyDouble = Py.toDouble();
    qint64 PyValue = static_cast<qint64>(PyDouble); // 只要整数部分
    QString PyTrans = dataTransfer->decimalToHexSigned(QString::number(PyValue),32);
    QByteArray PyArray = dataTransfer->string2Bytearray(PyTrans);

    QString Pz = csvLine.at(11); // 有符号整形——>二进制补码 4字节
    double PzDouble = Pz.toDouble();
    qint64 PzValue = static_cast<qint64>(PzDouble); // 只要整数部分
    QString PzTrans = dataTransfer->decimalToHexSigned(QString::number(PzValue),32);
    QByteArray PzArray = dataTransfer->string2Bytearray(PzTrans);

    QString Vx = csvLine.at(12); // 有符号整形——>二进制补码 4字节
    double VxDouble = Vx.toDouble();
    qint64 VxValue = static_cast<qint64>(VxDouble); // 只要整数部分
    QString VxTrans = dataTransfer->decimalToHexSigned(QString::number(VxValue),32);
    QByteArray VxArray = dataTransfer->string2Bytearray(VxTrans);

    QString Vy = csvLine.at(13); // 有符号整形——>二进制补码 4字节
    double VyDouble = Vy.toDouble();
    qint64 VyValue = static_cast<qint64>(VyDouble); // 只要整数部分
    QString VyTrans = dataTransfer->decimalToHexSigned(QString::number(VyValue),32);
    QByteArray VyArray = dataTransfer->string2Bytearray(VyTrans);

    QString Vz = csvLine.at(14); // 有符号整形——>二进制补码 4字节
    double VzDouble = Vz.toDouble();
    qint64 VzValue = static_cast<qint64>(VzDouble); // 只要整数部分
    QString VzTrans = dataTransfer->decimalToHexSigned(QString::number(VzValue),32);
    QByteArray VzArray = dataTransfer->string2Bytearray(VzTrans);

    // 星历卫星2标识 lineEdit 负X向 2字节
    QString ephemeris2String =ui->lineEdit_ephemeris2->text();
    QByteArray ephemeris2Array;
    ephemeris2Array.append(dataTransfer->string2Bytearray(ephemeris2String));

    // 6个星历卫星2 每个4字节
    QByteArray placeHolder2;
    placeHolder2.append(QByteArray(24,0xFF));

    // 定制信息 119字节
    QByteArray placeHolder3;
    placeHolder3.append(QByteArray(119,0xEE));

    // 开始组装1
    QByteArray broadFrameArray;
    broadFrameArray.append(comCodeArray);
    broadFrameArray.append(sysTimeSecArray);
    broadFrameArray.append(sysTimeSecMsCountArray);

    broadFrameArray.append(validIdenArray);

    broadFrameArray.append(q0Array);
    broadFrameArray.append(q1Array);
    broadFrameArray.append(q2Array);
    broadFrameArray.append(q3Array);

    broadFrameArray.append(placeHolder1);

    broadFrameArray.append(sunXArray);
    broadFrameArray.append(sunYArray);
    broadFrameArray.append(sunZArray);

    broadFrameArray.append(ephemeris1Array);

    broadFrameArray.append(PxArray);
    broadFrameArray.append(PyArray);
    broadFrameArray.append(PzArray);
    broadFrameArray.append(VxArray);
    broadFrameArray.append(VyArray);
    broadFrameArray.append(VzArray);

    broadFrameArray.append(ephemeris2Array);

    broadFrameArray.append(placeHolder2);

    broadFrameArray.append(placeHolder3);

    qDebug()<<"一帧姿控组播的长度："<<broadFrameArray.size();

    QString msg = QString("发送数据 q0:%1 q1:%2 q2:%3 q3:%4 sunX:%5 sunY:%6 sunZ:%7 Y:%8 Z:%9 Vx:%10 Vy:%11 Vz:%12")
            .arg(q0)
            .arg(q1)
            .arg(q2)
            .arg(q3)
            .arg(sunX)
            .arg(sunY)
            .arg(sunZ)
            .arg(Px)
            .arg(Py)
            .arg(Pz)
            .arg(Vx)
            .arg(Vy)
            .arg(Vz);
    showSendAttitudeMsg(msg);

    return broadFrameArray;

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
