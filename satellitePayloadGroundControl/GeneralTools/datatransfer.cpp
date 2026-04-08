#include "datatransfer.h"
#include <QtEndian>

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

QByteArray DataTransfer::uintTo_1_Bytes(uint16_t value)
{
    QByteArray arr;
    arr.append(value & 0xFF);
    return arr;
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

QByteArray DataTransfer::uintTo_4_Bytes(uint32_t value)
{
    QByteArray arr;
    arr.append((value >> 24) & 0xFF);
    arr.append((value >> 16) & 0xFF);
    arr.append((value >> 8) & 0xFF);
    arr.append(value & 0xFF);
    return arr;
}

// 符号数转换，value：原始无符号数，bitWidth：这个数据占据多少bit
qint64 DataTransfer::convertSigned(quint64 value, int bitWidth)
{
    qint64 mask = (1LL << bitWidth); //1LL表示long long（64位），左移bitWidth位，得到2^bitWidth
    qint64 signBit = (bitWidth == 64) ? 0 : (1LL << bitWidth);; // 计算符号位，找到“最高位”，是0（表示正数）或者1（表示负数）

    if(value & signBit)
        return static_cast<qint64>(value) - mask;

    return static_cast<qint64>(value);
}

// QString转换成有符号整型对应的十六进制数据:将一个表示有符号十进制数的字符串转换为指定位宽（bit width）的十六进制补码表示
QString DataTransfer::decimalToHexSigned(QString str, int bitWidth)
{
    bool ok;
    qint64 value = str.toLongLong(&ok, 10);

    if(!ok) return "ERROR";

    quint64 mask;
    if(bitWidth == 64)
        mask = 0xFFFFFFFFFFFFFFFFULL;
    else
        mask = (1ULL << bitWidth) - 1;

    quint64 hexValue = static_cast<quint64>(value) & mask;

    QString hexStr = QString::number(hexValue, 16).toUpper();

    // 固定长度
    int hexLen = bitWidth / 4;
    hexStr = hexStr.rightJustified(hexLen, '0');

    return hexStr;
}

QByteArray DataTransfer::floatToByteArray(float value, int byteNum)
{
    QByteArray bytes;
    bytes.resize(byteNum);
//    float tmp = value;
//    bytes.append(reinterpret_cast<const char*>(&value), byteNum);
    // 1. 将 float 的内存数据视为 32 位无符号整数
    quint32 temp;
    memcpy(&temp, &value, 4);
    // 2. 转换为大端序
    temp = qToBigEndian(temp);
    memcpy(bytes.data(), &temp, byteNum); // 安全拷贝
    return bytes;
}
