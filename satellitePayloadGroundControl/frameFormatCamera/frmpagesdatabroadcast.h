#ifndef FRMPAGESDATABROADCAST_H
#define FRMPAGESDATABROADCAST_H

#include <QWidget>
#include <QFile>
// 自动生成的ui文件，注意确保这个文件路径正确
#include "ui_frmpagesdatabroadcast.h"

//namespace Ui {
//class frmPagesDataBroadcast; // ✅ 只做前向声明
//}

class frmPagesDataBroadcast : public QWidget
{
    Q_OBJECT

public:
    explicit frmPagesDataBroadcast(QWidget *parent = nullptr);
    ~frmPagesDataBroadcast();
    void initForms();

public slots:
    void sendTimerTimeout();
    //发送 trace数据的定时处理函数
//    void sendTraceDataTimerTimeout();
    void showSendTimeMsg(QString msg);
    void showSendTraceMsg(QString msg);

public:
    QCheckBox* getSatelliteCheckBox();
signals:
    void timeBroadcastSignal(QString timeBroadcast);
    void attitudeBroadcastSignal(QString attitudeBroadcast);
    void sunBroadcastSignal(QString sunBroadcast);
    void traceBroadcastSignal(QString traceBroadcast);

private slots:
    void on_pb_send_location_clicked();

    void on_pb_broadcastStart_clicked();

//    void on_pb_send_satellite_location_clicked();

    void on_pb_send_satellite_attitude_clicked();

    void on_pb_begin_inject_clicked();

    void on_pb_begin_cmd_inject_clicked();


//    void on_pb_send_time_clc_clicked();

    void on_pb_choose_time_clicked();

    void on_pb_choose_trace_clicked();
    void on_pb_choose_satellite_clicked();
    void on_pb_choose_sunVector_clicked();


    void on_pb_clear_te_time_clicked();
    void on_pb_clear_te_gps_clicked();
    void on_pb_clear_sun_vector_clicked();
    void on_pb_clear_satellite_attitude_clicked();



private:
    Ui_frmpagesdatabroadcast *ui;
    //时间差
    quint32 timeDiffValue = 0;
    quint32 startTimeCount = 0;

    QTimer * sendTimeTimer = nullptr;

    //注入曲线的文件名称
     QString curevFileName;
     //轨道文件
     QString traceFileName;

     //轨道文件的句柄
     QFile traceFileHander;
     QTimer* traceDataSendTimer = nullptr;

     // 太阳矢量文件
     QString sunVectorFileName;
     // 卫星姿态文件
     QString satelliteAttiFileName;

     //开始发送
     void startSendTraceData();
     //停止发送
     void stopSendTraceData();
};

#endif // FRMPAGESDATABROADCAST_H
