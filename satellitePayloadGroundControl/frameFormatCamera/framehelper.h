/*用于检查接收帧格式是否正确，对发送帧进行包装*/
#ifndef FRAMEHELPER_H
#define FRAMEHELPER_H

#include <QObject>
#include <QWidget>

class FrameHelper : public QObject
{
    Q_OBJECT
public:
    explicit FrameHelper(QObject *parent = nullptr);

public:
    void frameCheckLVDS(QByteArray byteArray);
    void frameAssemble(QByteArray byteArray);
    QByteArray frameChecksum(QByteArray byteArray, int checkSumLength);

signals:

public slots:
};

#endif // FRAMECHECK_H
