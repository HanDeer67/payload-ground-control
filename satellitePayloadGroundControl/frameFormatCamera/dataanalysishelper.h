#ifndef DATAANALYSISHELPER_H
#define DATAANALYSISHELPER_H

#include <QObject>
#include "framehelper.h"
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include "ringbuffer.h"
#include <QMetaType>
#include "Structs.h"
#include "datatransfer.h"
#include "canparser.h"

//struct ImageAttributes{
//    int imgFrameNo = 0; // 图像帧编号
//    QString imgFrameNoString;
//    int totalImgNum = 0; // 总图像帧计数
//    int validImgNum = 0; // 正确图像帧计数
//    int errorImgNum = 0; // 错误图像帧计数，校验和不对
//    int imgWinSize = 0; // 开窗大小
//    int overThreshold = 0; // 超阈值点数量
//    QString timeCode; // 时间码 解析成XXXXX 秒 XXXXX 毫秒
//};

class DataAnalysisHelper : public QObject
{
    Q_OBJECT
public:
    explicit DataAnalysisHelper(QObject *parent = nullptr);

public:
    /// 串口通信接收数据解析函数
    void onDataAnalysisSerial1(); // 对串口1数据进行所需的解析（3M 质心遥测）
    void onDataAnalysisSerial2(); // 对串口2数据进行所需的解析（115200 温度等遥测）
    void processSerial1Frame(QByteArray frame); // 对LVDS板卡数据进行所需的解析（图像拼接）
    void processSerial2Frame(QByteArray frame); // 对LVDS板卡数据进行所需的解析（图像拼接）

    /// 板卡通信接收数据解析函数
    void onDataAnalysisRS422(); // 抓取422数据
    void onDataAnalysisLVDS();  // 抓取数据
    void processRS422Frame(QByteArray frame); // 对LVDS板卡数据进行所需的解析（图像拼接）
    void processLVDSFrame(QByteArray frame); // 对LVDS板卡数据进行所需的解析（图像拼接）

    /// CAN卡通信接收数据解析函数
    void onDataAnalysisCAN(); // 抓取CAN数据
    void processCANFrame(QByteArray frame); // 对CAN板卡数据进行所需的解析
    void processCANFrame_slow(QByteArray frame);
    void processCANFrame_all(QByteArray frame);

    const int FRAME_SIZE = 2048;// 确定帧长
    const int FRAME_SIZE_TEM = 11;// 确定串口的温度帧帧长11字节
    const int FRAME_SIZE_CENTROID = 19;// 确定串口的质心帧帧长19字节
    const int HEADER_SIZE = 2;
    const int frameSizePure = 2027; // 确定有效灰度数据帧长

    QByteArray serialBuffer1; // 串口1接收数据缓存区
    QByteArray serialBuffer2; // 串口2接收数据缓存区
    QByteArray rs422Buffer; // rs422板卡接收数据缓存区
    QByteArray lvdsBuffer; // LVDS接收数据缓存区

    QByteArray lvdsBufferTemp; // LVDS接收数据缓存区
    QByteArray lvdsImgDataBuffer; // LVDS图像数据缓存区
    bool headFrameExist = false; // 当前是否已经找到首帧，默认没有
    QByteArray imageNo; // 当前正在处理的图像包序号
    bool imgIsError = false; // 图像是否错误，校验和不对

//    int totalImgNum = 0; // 总图像帧计数
//    int validImgNum = 0; // 正确图像帧计数
//    int errorImgNum = 0; // 错误图像帧计数，校验和不对
    int imgSize = 0;

    ImageAttributes imgAttrs;
    TemperatureFrameAttributes temFrameAttrs;
    CentroidFrameAttributes centroidFrameAttrs;


    int grayThrehold = 0;
    int peakingThrehold = 0;
    void updataUiSettingsRec(const SettingsUiToSub &settingsUi);

    int tempNum = 0;
    QVector<int> arr;

    FrameHelper *frameHelper;

    void showGrayImage(const QByteArray &singleImgData, int imgSize);

    void setRingBuffer(RingBuffer *buffer);  // 关键函数声明
    RingBuffer *m_ringBuffer;

    void setRingBuffer_slow(RingBuffer *buffer);  // 关键函数声明
    RingBuffer *m_ringBuffer_slow;

    void startProcessing() {
        QMutexLocker locker(&m_mutex);
        m_running = true;
    }

    void stopProcessing() {
        QMutexLocker locker(&m_mutex);
        m_running = false;
    }

//    void startProcessingCAN() {
//        QMutexLocker locker(&m_mutex_can);
//        m_running_can = true;
//    }

//    void stopProcessingCAN() {
//        QMutexLocker locker(&m_mutex_can);
//        m_running_can = false;
//    }

    void imgToPeaking(bool isImgToPeakingUi, int peakingThresholdUi);


    QVector<QPoint> overThresholdCoords;


    void processAckFrame(const QByteArray &frame);
    void processTelemetryFrame(const QByteArray &frame);

    void getXmlParser(QVector<TMitemQueue> paraQueueVector);
    QVector<TMitemQueue> paraQueueVector;
    void getXmlParser_slow(QVector<TMitemQueue> paraQueueVectorRec);
    QVector<TMitemQueue> paraQueueVector_slow;

    ParaListUi paraListUi;
    QVector<ParaListUi> paraListUiList;
    ParaListUi paraListUi_slow;
    QVector<ParaListUi> paraListUiList_slow;


    QString applyRule(const QString &rule, double value);

    FrameTM frameTm;
    FrameTM frameTm_slow;


    quint8 h0_xml;
    quint8 h1_xml;
    quint8 t0_xml;
    quint8 t1_xml;
    int lengthHead = 2;
    int lengthTail = 2;

    quint8 h0_xml_fast;
    quint8 h1_xml_fast;
    quint8 t0_xml_fast;
    quint8 t1_xml_fast;
    int lengthHead_fast = 4;
    int lengthTail_fast = 4;

    quint8 h0_xml_slow;
    quint8 h1_xml_slow;
    quint8 t0_xml_slow;
    quint8 t1_xml_slow;
    int lengthHead_slow = 4;
    int lengthTail_slow = 4;

    quint8 h0_xml_all;
    quint8 h1_xml_all;
    quint8 t0_xml_all;
    quint8 t1_xml_all;
    int lengthHead_all = 4;
    int lengthTail_all = 4;


    // 新的帧头帧尾定义
//    QByteArray headFrame;
//    QByteArray tailFrame;

//    void updateFrameRecSetting();

    QByteArray canFrameBuffer;  // 用于收集所有 CAN 子帧的数据
    int canSubFrameCount = 0;   // 已收集子帧数量
    DataTransfer *dataTransfer;

    CANParser *canParser1;
    QByteArray TMArray;


    void processTelemetryFrameCAN(const QByteArray &frame);
    void processTelemetryFrameCAN_slow(const QByteArray &frame);

private:
//    FrameHelper *frameHelper;
    volatile bool m_running = false;  // 必须加volatile防止编译器优化
    QMutex m_mutex;  // 保护缓冲区指针的互斥锁

//    volatile bool m_running_can = false;  // 必须加volatile防止编译器优化
//    QMutex m_mutex_can;  // 保护缓冲区指针的互斥锁

    bool isImgToPeaking = false; // 图像是否要锐化处理
    bool isSaveCoordinate = false; // 保存超阈值点像素坐标

signals:

    void imageSignal(QImage image);
    void upgradeErrorImgNumUi(const ImageAttributes &attrs);
    /// 推荐用 const &，效率更高，为什么呢？
    void upgradeCentroidUi(const CentroidFrameAttributes &attrs);
    void upgradeTemUi(const TemperatureFrameAttributes &attrs);
    void dataToUiDirect(QString dataReceivedString);
    void updateParaListUiSignal(QVector<ParaListUi> paraListUiList);
    void updateParaListUiSignal_slow(QVector<ParaListUi> paraListUiList);

    void updateFrameCheckSignal(bool isValid);

public slots:
    void updateSaveCoordinate(bool isSaveCoordinateUi);


};

#endif // DATAANALYSISHELPER_H
