#include "serialportworker.h"
#include "datatransfer.h"
//#include "dataanalysishelper.h"
#include "framehelper.h"
#include <QThread>
#include <QDebug>

#include <windows.h>
#include <QtSerialPort/QSerialPort>

SerialPortWorker::SerialPortWorker(QObject *parent) : QObject(parent)
{

    datatransfer = new DataTransfer(this);


    serialPort = new QSerialPort(this);
    // 关键：增大接收缓冲区
    serialPort->setReadBufferSize(1024 * 1024 * 50); // 1MB缓冲区
    connect(serialPort, &QSerialPort::readyRead, this, &SerialPortWorker::readData);
    // 手动维护一个缓存区，用来缓存串口接收到的数据
    byteArraySerial = new QByteArray;
    CHECKSUM_16 = QByteArray::fromHex("0000"); // 初始化校验和
    frameHelper = new FrameHelper(this);


    connect(serialPort, &QSerialPort::errorOccurred, [](QSerialPort::SerialPortError error) {
        if (error != QSerialPort::NoError) {
            qCritical() << "串口错误:" << error; // 打印错误（如缓冲区溢出、 parity错误）
        }
    });
}

SerialPortWorker::~SerialPortWorker(){
    byteArraySerial->clear();
}

// 串口接收或发送帧帧格式设置
// 发送（Serial1） 115200
const QByteArray SerialPortWorker::PARA_COMMAND_FRAME_HEADER = QByteArray::fromHex("B9E0"); // 相机参数命令帧
const int SerialPortWorker::PARA_COMMAND_FRAME_LENGTH = 18;
const QByteArray SerialPortWorker::TELECHECK_COMMAND_FRAME_HEADER = QByteArray::fromHex("B9F0"); // 相机遥测命令帧（遥测查询）
const int SerialPortWorker::TELECHECK_COMMAND_FRAME_LENGTH = 5;
// 接收（Serial1） 115200
const QByteArray SerialPortWorker::RESPONSE_FRAME_HEADER = QByteArray::fromHex("B9E0"); // 相机响应帧
const int SerialPortWorker::RESPONSE_FRAME_LENGTH = 11;
// 接收（Serial2） 3M
const QByteArray SerialPortWorker::DATA_FRAME_HEADER = QByteArray::fromHex("B9C0"); // 相机质心数据帧
const int SerialPortWorker::DATA_FRAME_LENGTH = 19;

// 初始化串口
void SerialPortWorker::initSerial(QString portName,int baudRate, int dataBit, int parity, int stopBit){
    serialPort->setPortName(portName);
    serialPort->setBaudRate(static_cast<QSerialPort::BaudRate>(baudRate));
    serialPort->setDataBits(static_cast<QSerialPort::DataBits>(dataBit));
    serialPort->setStopBits(static_cast<QSerialPort::StopBits>(stopBit));
    serialPort->setParity(static_cast<QSerialPort::Parity>(parity));
}

void SerialPortWorker::closeSerial(){
    if (serialPort->isOpen())
        serialPort->close();
}

// 打开关闭串口
void SerialPortWorker::openCloseSerial(){
    if (serialPort->isOpen()){
        serialPort->close();
        emit updatePortStateSignal(false);
    }
    else{
        if (serialPort->open(QIODevice::ReadWrite)) {
            qDebug() << "Serial port opened in thread:" << QThread::currentThread();
            emit updatePortStateSignal(true);
        } else {
            qDebug() << "Failed to open serial port";
            emit updatePortStateSignal(false);
        }
    }
}

// 写数据到串口
void SerialPortWorker::writeData(QByteArray byteDataSend){
    if(serialPort->isOpen()){
        qDebug()<<"串口打开！";
        serialPort->write(byteDataSend);
    }
    else{
        emit RunError("错误,串口未打开！");
        qDebug()<<"错误,串口未打开！";
    }
}

void SerialPortWorker::readData(){
    /*笔记 2025.7.26 2:00
    当我使用串口模式来模拟单帧LVDS图像的解析操作时，我设置窗口大小为128*128，一个需要9帧2048字节的数据，我将这9帧数据组合在一起，组成了一个长为18432字节
    大小的长帧，当我使用串口传输这帧数据并在另一个口接收时发现，在航星电脑台式机上时，这个接收的数据帧出现丢失的情况，丢失的部分在帧的中间部分，但是当我使用我
    自己的拯救者笔记本时，数据被成功且完整接收，图像也正常解析。这里有两个猜测：①由于两个电脑上的串口都是虚拟串口，所以当时在使用软件创建这个虚拟串口时是不是有
    类似缓存大小这种需要设置，而我当时设置的不同。②如果两个电脑上的虚拟串口是一样的，那么就是航星台式机的硬件层面不如拯救者，或者其设置的缓存大小是小于拯救者的，
    如果是这种情况的话，我们可以尝试用代码增大缓存区大小

    注意：在该软件的实际使用中，上述问题其实基本上不会出现，因为当使用串口时，不会有这么长的帧数据一次写入串口，该数据本来是属于LVDS板卡的数据，即使是LVDS板卡在接收数据时，
    其单帧似乎也不会是整幅图像，这一点后续有待验证。

    */
    QByteArray data = serialPort->readAll();
    // 直接将数据发送给主UI界面
    QString dataReceivedString = datatransfer->Bytearray2string(data);
    emit dataToUiDirect(dataReceivedString); // 主页面显示串口接收

//    QByteArray dataHex = QByteArray::fromHex(data);
//    qDebug()<<"data"<<data.toHex(' ').toUpper();
//    qDebug()<<"dataHex"<<dataHex.toHex(' ').toUpper();

    // 将数据不断写入环形缓冲区
    if (m_ringBuffer) {
            m_ringBuffer->push(data);  // 非阻塞写入数据
        }


    /// 下面这些代码才是最终需要被解除注释的
//    // 在这里当场执行数据的检测，包括判断数据是来自哪个串口，数据帧校验是否正确等
//    byteArraySerial->append(data); // 接收到的数据加入缓存
//    while (true) {
//        int bufferSize = byteArraySerial->size();
//        qDebug()<<"bufferSize size:"<<bufferSize;
//        // 判断是否有可能有帧头
//        if (bufferSize < 2) break;
//        // 查找帧头位置
//        int headerIndex = -1;
//        int frameLength = 0;
//        // 遍历查找帧头
//        for (int i = 0; i <= bufferSize - 2; ++i) {
//            QByteArray maybeHeader = byteArraySerial->mid(i, 2);
//            if (maybeHeader == RESPONSE_FRAME_HEADER) {
//                headerIndex = i;
//                frameLength = RESPONSE_FRAME_LENGTH;
//                break;
//            } else if (maybeHeader == DATA_FRAME_HEADER) {
//                headerIndex = i;
//                frameLength = DATA_FRAME_LENGTH;
//                break;
//            }
//        }
//        // 如果没找到帧头，丢弃旧数据
//        if (headerIndex == -1) {
//            qDebug()<<"没找到帧头，丢弃旧数据";
//            byteArraySerial->clear();
//            break;
//        }
//        // 如果帧头前面有无效数据，丢弃
//        if (headerIndex > 0) {
//            qDebug()<<"帧头前面有无效数据，丢弃";
//            byteArraySerial->remove(0, headerIndex);
//        }
//        // 检查是否有完整帧
//        if (byteArraySerial->size() < frameLength) {
//            qDebug()<<"帧缺失，等待下一帧";
//            // 等待下一次readData()调用
//            break;
//        }
//        // 取出完整帧
//        QByteArray frame = byteArraySerial->left(frameLength);
//        qDebug()<<"frame:"<<frame.toHex(' ').toUpper();;
//        byteArraySerial->remove(0, frameLength);
//        // 判断帧类型并处理
//        if (frame.startsWith(RESPONSE_FRAME_HEADER)) {
//            qDebug()<<"frame:"<<frame.toHex(' ').toUpper();;
//            // 去掉帧头帧尾
//            QByteArray framePure = frame.mid(2,frame.size() - 3); // 注意，计算校验和的时候，输入的是纯累加数据
//            qDebug()<<"去掉帧头帧尾的数据："<<framePure.toHex(' ').toUpper();;
//            // 检查校验和，检查帧是否有效
//            QByteArray frameChecksum= frameHelper->frameChecksum(framePure,1);
//            qDebug() << "frameChecksum：" << frameChecksum.toHex(' ').toUpper();
//            qDebug() << "frame.right(1)：" << frame.right(1).toHex(' ').toUpper();
//            if(!frameChecksum.isEmpty() && frameChecksum == frame.right(1)){
//                qDebug() << "[响应帧]:" << frame.toHex(' ').toUpper();
//                dataAnalysisHelper->onDataAnalysisSerial1(dataReceivedString,data);
//            }
//            else {
//                qDebug() << "校验位出错";
//            }
//            // emit processResponseFrame(frame);
//        } else if (frame.startsWith(DATA_FRAME_HEADER)) {
//            qDebug()<<"frame:"<<frame.toHex(' ').toUpper();;
//            // 去掉帧头帧尾
//            QByteArray framePure = frame.mid(2,frame.size() - 3); // 注意，计算校验和的时候，输入的是纯累加数据
//            qDebug()<<"去掉帧头帧尾的数据："<<framePure.toHex(' ').toUpper();;
//            // 检查校验和，检查帧是否有效
//            QByteArray frameChecksum= frameHelper->frameChecksum(framePure,1);
//            qDebug() << "frameChecksum：" << frameChecksum.toHex(' ').toUpper();
//            qDebug() << "frame.right(1)：" << frame.right(1).toHex(' ').toUpper();
//            if(!frameChecksum.isEmpty() && frameChecksum == frame.right(1)){
//                qDebug() << "[质心帧]:" << frame.toHex(' ').toUpper();
//                dataAnalysisHelper->onDataAnalysisSerial2(dataReceivedString,data);
//            }
//            else {
//                qDebug() << "校验位出错";
//            }
//            // emit processDataFrame(frame);
//        } else {
//            qDebug() << "未知帧头，可能数据出错";
//        }
//    }

}

void SerialPortWorker::clearFIFO()
{
    // 打开串口后执行（仅需一次）
    serialPort->clear(); // 清空输入和输出缓冲区（默认行为）
}

void SerialPortWorker::setRingBuffer(RingBuffer *buffer)
{
    QMutexLocker locker(&m_mutex);  // 加锁保证线程安全
     if (m_ringBuffer != buffer) {
         m_ringBuffer = buffer;  // 更新缓冲区指针
         qDebug() << "RingBuffer updated in thread:" << QThread::currentThread();
     }
}
