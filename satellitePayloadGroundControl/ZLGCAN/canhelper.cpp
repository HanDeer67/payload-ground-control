#include "canhelper.h"
#include <qDebug>
#include <QThread>

canHelper::canHelper(QObject *parent) : QObject(parent)
{

}

DWORD canHelper::OPEN_DEVICE(DWORD DevType, DWORD DevIndex, DWORD Reserved)
{
    DWORD dwRel;
    dwRel = VCI_OpenDevice(DevType, DevIndex, Reserved);
    return dwRel;
//    if(dwRel != STATUS_OK){
//        qDebug()<<"打开设备失败!";
    //    }
}

DWORD canHelper::CLOSE_DEVICE(DWORD DevType, DWORD DevIndex)
{
    DWORD dwRel;
    dwRel = VCI_CloseDevice(DevType,DevIndex);
    return dwRel;
}

DWORD canHelper::INIT_CAN(DWORD DevType, DWORD DevIndex, DWORD CANIndex, PVCI_INIT_CONFIG pInitConfig)
{
    DWORD dwRel;
    dwRel = VCI_InitCAN(DevType,DevIndex,CANIndex,pInitConfig);
    return dwRel;
}

DWORD canHelper::READ_BOARD_INFO(DWORD DevType, DWORD DevIndex, PVCI_BOARD_INFO pInfo)
{
    DWORD dwRel;
    dwRel = VCI_ReadBoardInfo(DevType,DevIndex,pInfo);
    return dwRel;
}

DWORD canHelper::GETRECEIVENUM(DWORD DevType, DWORD DevIndex, DWORD CANIndex)
{
    DWORD dwRel;
    dwRel = VCI_GetReceiveNum(DevType,DevIndex,CANIndex);
    return dwRel;
}

DWORD canHelper::CLEARBUFFER(DWORD DevType, DWORD DevIndex, DWORD CANIndex)
{
    DWORD dwRel;
    dwRel = VCI_ClearBuffer(DevType,DevIndex,CANIndex);
    return dwRel;
}

DWORD canHelper::STARTCAN(DWORD DevType, DWORD DevIndex, DWORD CANIndex)
{
    DWORD dwRel;
    dwRel = VCI_StartCAN(DevType,DevIndex,CANIndex);
    return dwRel;
}

DWORD canHelper::RESETCAN(DWORD DevType, DWORD DevIndex, DWORD CANIndex)
{
    DWORD dwRel;
    dwRel = VCI_ResetCAN(DevType,DevIndex,CANIndex);
    return dwRel;
}

DWORD canHelper::TRANSMIT(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend, DWORD Length)
{
    DWORD dwRel;
    dwRel = VCI_Transmit(DeviceType,DeviceInd,CANInd,pSend,Length);
    return dwRel;
}

ULONG canHelper::RECEIVE(DWORD DevType, DWORD DevIndex, DWORD CANIndex, PVCI_CAN_OBJ pReceive, ULONG Len, INT WaitTime)
{
    ULONG dwRel = 0;
    dwRel = VCI_Receive(DevType,DevIndex,CANIndex,pReceive,Len, WaitTime);
    return dwRel;
}



