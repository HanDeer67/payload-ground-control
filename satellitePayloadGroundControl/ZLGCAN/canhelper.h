// 周立功can卡基础函数库
#ifndef CANHELPER_H
#define CANHELPER_H

#include <QObject>
#include <Windows.h>
#include "ControlCAN.h"


class canHelper : public QObject
{
    Q_OBJECT
public:
    explicit canHelper(QObject *parent = nullptr);

public:

    // 开启CAN板卡
    DWORD __stdcall OPEN_DEVICE(DWORD DevType, DWORD DevIndex, DWORD Reserved);
    // 关闭CAN板卡
    DWORD __stdcall CLOSE_DEVICE(DWORD DevType,DWORD DevIndex);
    // 初始化CAN
    DWORD __stdcall INIT_CAN(DWORD DevType, DWORD DevIndex, DWORD CANIndex,
                             PVCI_INIT_CONFIG pInitConfig);
    // 获取设备信息
    DWORD __stdcall  READ_BOARD_INFO(DWORD DevType, DWORD DevIndex, PVCI_BOARD_INFO pInfo);
    //

    // 获取CAN通道的接收缓冲区中接收到但尚未被读取的帧数量
    DWORD __stdcall GETRECEIVENUM(DWORD DevType,DWORD DevIndex,DWORD CANIndex);

    // 清空缓存
    DWORD __stdcall CLEARBUFFER(DWORD DevType,DWORD DevIndex,DWORD CANIndex);
    // 启动CAN卡
    DWORD __stdcall STARTCAN(DWORD DevType,DWORD DevIndex,DWORD CANIndex);
    // 复位CAN卡
    DWORD __stdcall RESETCAN(DWORD DevType,DWORD DevIndex,DWORD CANIndex);
    // 发送数据
    DWORD __stdcall TRANSMIT(DWORD DeviceType,DWORD DeviceInd,DWORD
                                CANInd,PVCI_CAN_OBJ pSend,DWORD Length);
    // 接收数据
    ULONG __stdcall RECEIVE(DWORD DevType, DWORD DevIndex, DWORD  CANIndex,
                                PVCI_CAN_OBJ pReceive, ULONG Len, INT WaitTime);


private:



signals:

public slots:

};

#endif // CANHELPER_H
