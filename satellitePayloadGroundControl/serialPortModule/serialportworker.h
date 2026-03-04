#ifndef SERIALPORTWORKER_H
#define SERIALPORTWORKER_H

#include <QObject>
#include <QSerialPort>
#include "datatransfer.h"
#include "dataanalysishelper.h"
#include "framehelper.h"
#include "ringbuffer.h"

class SerialPortWorker : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortWorker(QObject *parent = nullptr);
    ~SerialPortWorker();  // 析构函数声明

    void initSerial(QString portName,int baudRate, int dataBit, int parity, int stopBit); // 初始化串口
    void openCloseSerial(); // 打开关闭串口
    void writeData(QByteArray byteDataSend); // 写数据到串口
    void readData(); // 读串口的数据

    void clearFIFO();

    // 创建串口对象
    QSerialPort *serialPort;
    // 手动维护一个缓存区，用来缓存串口接收到的数据
    QByteArray *byteArraySerial;

    int tempTestData = 0;
    int tempTest = 0;

    // 串口接收或发送帧帧格式设置
    // 发送（Serial1）
    static const QByteArray PARA_COMMAND_FRAME_HEADER; // 相机指令帧头 115200
    static const QByteArray TELECHECK_COMMAND_FRAME_HEADER; // 遥测查询指令帧头 115200
    static const int PARA_COMMAND_FRAME_LENGTH; // 相机指令帧长
    static const int TELECHECK_COMMAND_FRAME_LENGTH; // 遥测查询指令帧长
    // 接收（Serial1）
    static const QByteArray RESPONSE_FRAME_HEADER; // 相机响应帧头 115200
    static const int RESPONSE_FRAME_LENGTH;
    // 接收（Serial2）
    static const QByteArray DATA_FRAME_HEADER; // 相机质心数据帧头 3M
    static const int DATA_FRAME_LENGTH;

    QByteArray CHECKSUM_16; // 单字节校验和

    QByteArray dataTemp;

    RingBuffer *m_ringBuffer;
    void setRingBuffer(RingBuffer *buffer);  // 关键函数声明

    void closeSerial();

private:

    // 创建数据处理对象
    DataTransfer *datatransfer;

    QMutex m_mutex;  // 保护缓冲区指针的互斥锁

    FrameHelper *frameHelper;


signals:
    // 直接发送数据到ui界面
    void dataReceived();
    void updatePortStateSignal(bool portStatus);
    void dataToUiDirect(QString dataReceivedString); // 串口数据直接显示
//    void imageToShowUiSignal(QImage image);

    void RunError(QString errorText);


public slots:
};

#endif // SERIALPORTWORKER_H
