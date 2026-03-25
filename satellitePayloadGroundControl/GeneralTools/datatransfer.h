#ifndef DATATRANSFER_H
#define DATATRANSFER_H

#include <QObject>
#include <QByteArray>

class DataTransfer : public QObject
{
    Q_OBJECT

signals:
    // void byteDataSignal(QByteArray data);  // 声明信号

public:
    explicit DataTransfer(QObject *parent = nullptr);
    QByteArray string2Bytearray(QString stringData);
    QString Bytearray2string(QByteArray byteArrrayData);

    QByteArray uintTo_3_Bytes(uint32_t value);
    QByteArray uintTo_2_Bytes(uint16_t value);
    QByteArray uintTo_4_Bytes(uint32_t value);
    qint64 convertSigned(quint64 value, int bitWidth);

private:
    // 私有成员...
};

#endif // DATATRANSFER_H
