#include "datatransfer.h"

DataTransfer::DataTransfer(QObject *parent)
    : QObject(parent)
{
}

QByteArray DataTransfer::string2Bytearray(QString stringData)
{
    // 移除字符串中的空格
    QString hexStringData = stringData.remove(' ');
    // 将十六进制字符串转换为字节数组
    QByteArray byteArrayData = QByteArray::fromHex(hexStringData.toLatin1());
    // 发送信号
    // emit byteDataSignal(byteArrayData);
    return byteArrayData;
}

QString DataTransfer::Bytearray2string(QByteArray byteArrrayData){
    // 转换为十六进制字符串（带空格分隔）
    QString hexString;
    for(int i=0; i<byteArrrayData.size(); ++i) {
        hexString += QString("%1 ")
        .arg(static_cast<quint8>(byteArrrayData[i]), 2, 16, QLatin1Char('0'))
            .toUpper();
    }
    hexString = hexString.trimmed();  // 去掉末尾多余空格
    return hexString;
}

QByteArray DataTransfer::uintTo_2_Bytes(uint16_t value)
{
    QByteArray arr;
    arr.append((value >> 8) & 0xFF);
    arr.append(value & 0xFF);
    return arr;
}

QByteArray DataTransfer::uintTo_3_Bytes(uint32_t value)
{
    QByteArray arr;
    arr.append((value >> 16) & 0xFF);
    arr.append((value >> 8) & 0xFF);
    arr.append(value & 0xFF);
    return arr;
}

// 符号数转换
qint64 DataTransfer::convertSigned(quint64 value, int bitWidth)
{
    qint64 mask = (1LL << bitWidth);
    qint64 signBit = (1LL << (bitWidth - 1));

    if(value & signBit)
        return static_cast<qint64>(value) - mask;

    return static_cast<qint64>(value);
}
