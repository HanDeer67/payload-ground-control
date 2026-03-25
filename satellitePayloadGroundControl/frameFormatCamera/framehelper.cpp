#include "framehelper.h"
#include <QDebug>


FrameHelper::FrameHelper(QObject *parent) : QObject(parent)
{

    dataTransfer = new DataTransfer(this);
}

// 检查帧长度和校验和是否正确
/* 不同帧格式：
 *  接收：
 *      ① LVDS图像帧
 *      帧长：2(E225)+ 2042 + 2 + 2(2B) = 2048字节
 *      ② RS422相机应答帧
 *      帧长：5字节
 *      ③ RS422数据帧
 *      帧长：2+16+1 = 19字节
 *  发送：
 *      RS422指令帧
 *      帧长：2+15+1 = 18字节
 *
 *
 */

/// 上位机接收帧检查
// 1. LVDS接收图像帧
void FrameHelper::frameCheckLVDS(QByteArray byteArray){
    qDebug()<<"byteArray："<<byteArray;
    qDebug()<<"byteArray size："<<byteArray.size();
}


/// 上位机发送指令帧组装
void FrameHelper::frameAssemble(QByteArray byteArray){

}



/// 校验和计算
QByteArray FrameHelper::frameChecksum(QByteArray byteArray, int checkSumLength){
//    qDebug()<<"当前的数据是："<<byteArray.toHex(' ').toUpper();
    quint32 sum = 0;
    for (int i = 0; i < byteArray.size(); ++i) {
        sum += static_cast<quint8>(byteArray[i]);  // 使用无符号累加，防止负数
    }

    QByteArray result;
    if (checkSumLength == 1) {
        quint8 checksum8 = static_cast<quint8>(sum & 0xFF);
//        checksum8 = ~checksum8; //取反
        result.append(static_cast<char>(checksum8));
    } else if (checkSumLength == 2) {
        quint16 checksum16 = static_cast<quint16>(sum & 0xFFFF);
//        checksum16 = ~checksum16; // 取反
        // 小端格式
//        result.append(static_cast<char>(checksum16 & 0xFF));         // 低字节
//        result.append(static_cast<char>((checksum16 >> 8) & 0xFF));  // 高字节
        // 大端格式
        result.append(static_cast<char>((checksum16 >> 8) & 0xFF));  // 高字节
        result.append(static_cast<char>(checksum16 & 0xFF));         // 低字节
    }
    qDebug()<<"当前的校验和是："<<result.toHex(' ').toUpper();
    return result;

    /// 这里使用了很多static_cast<char>或static_cast<quint8>，注意byteArray本身存储的都是char类型，也就是有符号类型的数，00-FF表示的是-128~127
    /// ☆☆☆☆☆当我想要将无符号数存进byteArray时需要先将数据转成有符号，当我从byteArray中拿数据出来计算时需要先将数据转成无符号。☆☆☆☆☆
    /// 访问 QByteArray 作为字节数组	总是 static_cast<quint8>(byteArray[i])
    /// 累加做校验和	必须 quint32 sum += static_cast<quint8>(...)
    /// 解析 8 位图像像素	使用 quint8 pixel = static_cast<quint8>(...)
    /// 写入 QByteArray	用 static_cast<char>(quint8_value)
    /// ✔ 往 QByteArray 写入数据时：quint8 → static_cast<char>
    /// ✔ 从 QByteArray 读取数据时：char → static_cast<quint8>
}

/// 组装长指令序列源码 帧头+纯指令数据+校验和+帧尾
QByteArray FrameHelper::lenthCodeAssemble(QList<QDomElement> docNodeList,  QString pureStringData, int row){
    QString frameHead;
    QString frameTail;
    int checkSumLength;
    QDomElement  tempElem  = docNodeList.at(row).toElement();
    frameHead  = tempElem.attribute("FrameHeader");
    frameTail = tempElem.attribute("FrameTail");
    checkSumLength = tempElem.attribute("CheckSumLength").toInt();
    qDebug()<<"listHeader"<<frameHead;
    qDebug()<<"listTail"<<frameTail;
    qDebug()<<"listCheckSumLength"<<checkSumLength;

   // 添加校验和
   QByteArray immediateCommand = dataTransfer->string2Bytearray(pureStringData);
   QByteArray checkSum =  frameChecksum(immediateCommand,checkSumLength);
   QString checkSumString = QString(checkSum.toHex().toUpper());

   // 添加帧头帧尾
   QString frameHead0 = "B9E0"; // 是否所有指令都是这个帧头
   QString byteDataSendString = frameHead + pureStringData + checkSumString + frameTail;
//    QString byteDataSendString = frameHead0 + tempStringData + checkSumString;
   qDebug()<<"byteDataSendString"<<byteDataSendString;
   QByteArray byteDataSend = dataTransfer->string2Bytearray(byteDataSendString);
}
