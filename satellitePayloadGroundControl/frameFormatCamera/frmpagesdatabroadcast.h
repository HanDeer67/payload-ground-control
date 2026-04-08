#ifndef FRMPAGESDATABROADCAST_H
#define FRMPAGESDATABROADCAST_H

#include <QWidget>
#include <QFile>
// 自动生成的ui文件，注意确保这个文件路径正确
#include "ui_frmpagesdatabroadcast.h"
#include "datatransfer.h"

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

    DataTransfer *dataTransfer;

public slots:
    void sendTimerTimeout();
    void sendTimerTimeout_attitude();
    void sendTimerTimeout_trace();
    void sendTimerTimeout_sun();

    //发送 trace数据的定时处理函数

    void showSendTimeMsg(QString msg);
    void showSendTraceMsg(QString msg);
    void showSendSunMsg(QString msg);
    void showSendAttitudeMsg(QString msg);

public:

    void updateBroadCount(int updateWhichBroadCount);

    QCheckBox* getSatelliteCheckBox();
    void resetBroadCount();
    int timeCount = 0;
    int traceCount = 0;
    int satallite = 0;
    int sunCount = 0;


    void resetCsvPosition();
signals:
    void timeBroadcastSignal(QByteArray timeBroadcast);
    void attitudeBroadcastSignal(QByteArray attitudeBroadcast);
    void sunBroadcastSignal(QByteArray sunBroadcast);
    void traceBroadcastSignal(QByteArray traceBroadcast);

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
    void on_pb_clear_te_time2_clicked();
    void on_pb_clear_te_gps2_clicked();
    void on_pb_clear_sun_vector2_clicked();
    void on_pb_clear_satellite_attitude2_clicked();

    QByteArray outputBroadFrameTime(QStringList csvLine);
    QByteArray outputBroadFrameTrace(QStringList csvLine);
    QByteArray outputBroadFrameSun(QStringList csvLine);
    QByteArray outputBroadFrameAttitude(QStringList csvLine);




private:
    Ui_frmpagesdatabroadcast *ui;

    //注入曲线的文件名称
     QString curevFileName;

    //1时间广播文件的句柄
    QFile timeFileHander;
    // 时间广播文件
    QTimer * sendTimeTimer = nullptr;
    QString timeFileName;

    //2轨道文件的句柄
    QFile traceFileHander;
    //轨道文件
    QString traceFileName;
    QTimer* traceDataSendTimer = nullptr;

    // 3太阳矢量文件的句柄
    QFile sunFileHander;
    // 太阳矢量文件
    QString sunVectorFileName;
    QTimer* sunSendTimer = nullptr;

    // 4姿态文件的句柄
    QFile satelliteFileHander;
    // 卫星姿态文件
    QString satelliteAttiFileName;
    QTimer* satelliteSendTimer = nullptr;


     //开始发送
     void startSendTraceData();
     //停止发送
     void stopSendTraceData();
};

#endif // FRMPAGESDATABROADCAST_H
