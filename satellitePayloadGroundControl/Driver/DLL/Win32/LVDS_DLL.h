#ifndef _LVDS_DLL_H_
#define _LVDS_DLL_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <Windows.h>

#ifdef LVDS_DLL_EXPORTS
#	define LVDS_DLL_EXPORT  __declspec(dllexport)
#else
#	define LVDS_DLL_EXPORT  __declspec(dllimport)
#endif
#pragma pack(4)//按照4字节对齐，避免使用库的程序使用其他对齐导致发生问题

/*****************************************************************************
*文件名称 : LVDS_DLL.h
*功能介绍 ：提供通用LVDS\串口\异步串口的接口函数
*版 本 号 : V2.10
*创建日期 : 2017.11.20
*作    者 : wangyl
*说    明 ：本函数为了提供通用功能，有一些参数和函数是考虑到最大包络而设计
*****************************************************************************
*修改记录 :(日期 作者 修改内容)
1） 2018.10.17 wangyl 修改BUF_SIZE大小为32K，修改ucBuf类型为UCHAR
2） 2018.11.23 wangyl 增加总线查询接口，增加IsOpen，修改数据结构名称避免重定义
3） 2019.08.02 wangyl LVDS修改LVDS_IntStart实现方式,解决因静态局部变量可能导致的数据错乱情况
4） 2019.11.11 wangyl LVDS修改支持的最大板卡数据量由16修改为32
5） 2020.04.09 wangyl V2.5按照修订协议V2.1做相应修改,1)时钟可查2)接收发送波特率等可设3)发送接收帧可设置4)扩展通道数为32
6） 2020.05.20 wangyl V2.6按照修订协议V2.2做相应修改,1)增加RAM模式发送数据
7） 2020.06.03 wangyl V2.7函数LVDS_GetCount增加超时时间
8） 2020.07.07 wangyl V2.8修改函数LVDS_Send在单次发送量超出发送FIFO大小时错误
9） 2020.07.08 wangyl V2.9为解决库调用方在使用库字节对齐与库不一致导致问题，增加pack对齐编译设置,只修改头文件
10）2020.07.09 wangyl V2.10函数LVDS_RstRAMFrmHadTCnt修改返回值类型，由DWORD修改为BOOL
*****************************************************************************/

//====================================================================
//以下为类型定义：包括缓存大小、通道号、数据结构体、配置结构体、回调类型
//====================================================================
#define LVDS_BUF_SIZE            (32*1024) //定义BUF大小为32K

//定义发送和接收通道号,各为32路 ★★★注意通道号LVDS_DATA_CH1为0★★★
enum
{
	LVDS_DATA_CH1 = 0,
	LVDS_DATA_CH2,
	LVDS_DATA_CH3,
	LVDS_DATA_CH4,
	LVDS_DATA_CH5,
	LVDS_DATA_CH6,
	LVDS_DATA_CH7,
	LVDS_DATA_CH8,
	LVDS_DATA_CH9,
	LVDS_DATA_CH10,
	LVDS_DATA_CH11,
	LVDS_DATA_CH12,
	LVDS_DATA_CH13,
	LVDS_DATA_CH14,
	LVDS_DATA_CH15,
	LVDS_DATA_CH16,
	LVDS_DATA_CH17,
	LVDS_DATA_CH18,
	LVDS_DATA_CH19,
	LVDS_DATA_CH20,
	LVDS_DATA_CH21,
	LVDS_DATA_CH22,
	LVDS_DATA_CH23,
	LVDS_DATA_CH24,
	LVDS_DATA_CH25,
	LVDS_DATA_CH26,
	LVDS_DATA_CH27,
	LVDS_DATA_CH28,
	LVDS_DATA_CH29,
	LVDS_DATA_CH30,
	LVDS_DATA_CH31,
	LVDS_DATA_CH32,
	LVDS_DATA_CNT
};

//定义接收数据结构体
typedef struct
{
	UCHAR emType;//数据类型:标识数据从哪个通道接收的 范围为[LVDS_DATA_CH1,LVDS_DATA_CNT)
	ULONG usLen; //数据长度:标识接收的字节数，即标识ucBuf数组中有效数据的长度 范围为[1,LVDS_BUF_SIZE]
	UCHAR ucBuf[LVDS_BUF_SIZE];//接收数据数组:其中数组中的前usLen个字节为接收的有效数据
}ST_LVDS_DATA, *PLVDSDATA;

//以下为常用波特率定义
enum
{
	LVDS_BAUD_1200   = 1200,
	LVDS_BAUD_2400   = 2400,
	LVDS_BAUD_4800   = 4800,
	LVDS_BAUD_9600   = 9600,
	LVDS_BAUD_19200  = 19200,
	LVDS_BAUD_38400  = 38400,
	LVDS_BAUD_57600  = 57600, //24M时钟有误差
	LVDS_BAUD_115200 = 115200,//24M时钟有误差
	LVDS_BAUD_128000 = 128000,//24M时钟有误差
	LVDS_BAUD_256000 = 256000,//24M时钟有误差
};

//以下为数据位定义
enum
{
	LVDS_DATABITS_1,//1位数据位
	LVDS_DATABITS_2,//2位数据位
	LVDS_DATABITS_3,//3位数据位
	LVDS_DATABITS_4,//4位数据位
	LVDS_DATABITS_5,//5位数据位
	LVDS_DATABITS_6,//6位数据位
	LVDS_DATABITS_7,//7位数据位
	LVDS_DATABITS_8,//8位数据位
};

//以下为停止位定义
enum
{
	LVDS_STOPBITS_10,//1位停止位
	LVDS_STOPBITS_15,//1.5位停止位
	LVDS_STOPBITS_20,//2位停止位
};

//以下为校验位定义
enum
{
	LVDS_PARITY_NO,  //无校验
	LVDS_PARITY_ODD, //奇校验
	LVDS_PARITY_EVEN,//偶校验
};

//定义通道配置参数结构体(如果对于有些成员没用到可忽略)
typedef struct 
{
	DWORD  dwBaudRate;//波特率:对应LVDS_BAUD_1200~LVDS_BAUD_256000,对于未列出的波特率也支持
	UCHAR  ucDataBits;//数据位:为LVDS_DATABITS_1~LVDS_DATABITS_8
	UCHAR  ucStopBits;//停止位:为LVDS_STOPBITS_10~LVDS_STOPBITS_20
	UCHAR  ucParity;  //校验位:为LVDS_PARITY_NO~LVDS_PARITY_EVEN
}ST_LVDS_CFG,*PLVDSCFG;

//定义帧设置结构体
typedef struct
{
	BOOL  bEnable;  //帧使能:TRUE为使能,FALSE为禁止
	DWORD dwFrmLen; //帧长:单位为字节 默认为0x10000
	DWORD dwFrmTime;//帧间隔:单位为FPGA时钟个数(时钟频率可进行查询)
	DWORD dwFrmHead;//帧头:默认为0xEB90EB90
	DWORD dwFrmEnd; //帧尾:默认为0x55AA55AA
}ST_LVDS_FRM_CFG,*PSTLVDSFRMCFG;

#ifndef TYPEDEF_ST_PCI_INFO
#define TYPEDEF_ST_PCI_INFO
typedef struct 
{
	ULONG ulBusNo;//总线号
	ULONG ulDevNo;//设备号
	ULONG ulFunc;//功能号
}ST_PCI_INFO,*PPCIINFO;
#endif

//定义发送等待时间类型
enum
{
	LVDS_WAIT_NONE     = 0x00000000,//不等待,立即返回
	LVDS_WAIT_INFINITE = 0xFFFFFFFF,//无限等待
};

//定义数据字宽类型
enum
{
	LVDS_DATAWIDTH_1BYTE,//数据字宽:1字节
	LVDS_DATAWIDTH_2BYTE,//数据字宽:2字节
	LVDS_DATAWIDTH_3BYTE,//数据字宽:3字节
	LVDS_DATAWIDTH_4BYTE,//数据字宽:4字节
};

//定义接收回调函数类型
typedef void (*LVDS_CALLBACK)(UCHAR ucCard,  PLVDSDATA pstData, DWORD dwCount); //ucCard为板卡号,pstData为结构体数组指针 dwCount为结构体数组的个数

//====================================================================
//1.以下为板卡基本操作-----------操作板卡必须接口
//====================================================================
LVDS_DLL_EXPORT int  __stdcall  LVDS_CountCards();//获取板卡数量 在Open之后不能再调用
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_Open (UCHAR ucCard);//打开板卡 
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_Close(UCHAR ucCard);//关闭板卡
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_Reset(UCHAR ucCard);//复位板卡
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_Start(UCHAR ucCard, BOOL bStart);//启动板卡
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_IsOpen(UCHAR ucCard);//板卡是否已经打开
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetInfo(UCHAR ucCard,PPCIINFO pstInfo);//查询板卡总线信息

//====================================================================
//2.以下为调试接口-----------操作板卡辅助接口
//====================================================================
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetStart(UCHAR ucCard);//板卡启动状态
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetVer(UCHAR ucCard);//获取版本
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetID(UCHAR ucCard);//获取设备ID
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetFpgaDate(UCHAR ucCard);//获取编译日期
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetPowSta(UCHAR ucCard);//获取供电状态
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetIntEn(UCHAR ucCard);//获取中断使能状态
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetFpgaFreq(UCHAR ucCard); //查询FPGA时钟频率,默认为24000000,时钟频率将影响波特率计算
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_GetChDataWidth(UCHAR ucCard, BOOL bRcv, BYTE byCh, BYTE *pbyWidth);//查询指定通道发送或接收数据位宽 pbyWidth为LVDS_DATAWIDTH_1BYTE~LVDS_DATAWIDTH_4BYTE
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_CheckVer(UCHAR ucCard);//校验当前动态库版本是否匹配当前设备的FPGA程序版本

//====================================================================
//3.以下为板卡属性配置接口-----------操作板卡辅助接口
//====================================================================
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_ClrFIFO(UCHAR ucCard);//清空FIFO以及驱动中已经接收的数据
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetOutTime(UCHAR ucCard,DWORD dwTime=200);//设置接收中断超时时间(单位为ms)，默认为200ms,范围为[1,5000]
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetCfg(UCHAR ucCard, BOOL bRcv, BYTE byCh, PLVDSCFG pstCfg);//设置指定发送通道或接收通道波特率等参数 bRcv为TRUE表示接收通道,FALSE表示发送通道
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetCfg(UCHAR ucCard, BOOL bRcv, BYTE byCh, PLVDSCFG pstCfg);//查询指定发送通道或接收通道波特率等参数 bRcv为TRUE表示接收通道,FALSE表示发送通道
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetChFrmCfg (UCHAR ucCard, BOOL bRcv, BYTE byCh, PSTLVDSFRMCFG pstCfg);//设置指定通道发送或接收帧配置 bRcv为TRUE表示接收通道,FALSE表示发送通道
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetChFrmCfg (UCHAR ucCard, BOOL bRcv, BYTE byCh, PSTLVDSFRMCFG pstCfg);//查询指定通道发送或接收帧配置 bRcv为TRUE表示接收通道,FALSE表示发送通道

//====================================================================
//4.以下为进行单次数据发送-----------FIFO方式
//====================================================================
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_Send(UCHAR ucCard, BYTE byCh, BYTE *pbyBuf, DWORD dwBytes,DWORD dwWaitTime=LVDS_WAIT_INFINITE);//指定通道发送数据,发送dwBytes字节, dwWaitTime为等待时间(ms)LVDS_WAIT_INFINITE为等待到发送完成,返回值为实际发送字节数
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetChSendBytes(UCHAR ucCard, BYTE byCh);//获取指定发送通道实际(FPGA)已经发送字节数
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_RstChSendBytes(UCHAR ucCard, BYTE byCh);//重置指定发送通道实际(FPGA)已经发送字节数

//====================================================================
//5.以下为进行连续数据发送-----------RAM方式
//====================================================================
enum 
{
	LVDS_RAM_SEND_INFINITE = 0,
};
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_IsSupportRAM(UCHAR ucCard);//查询是否支持RAM模式
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetRAMSize(UCHAR ucCard, BYTE byCh);//查询指定通道RAM大小，单位为字节数
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_SetRAMFrmTime(UCHAR ucCard, BYTE byCh, DWORD dwTime);//设置指定通道帧间隔，单位为us
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetRAMFrmTime(UCHAR ucCard, BYTE byCh);//查询指定通道帧间隔，返回值单位为us
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_SetRAMFrmData(UCHAR ucCard, BYTE byCh, BYTE *pbyBuf, DWORD dwFrmLen);//设置指定通道RAM帧数据，返回值为实际设置成功字节数
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetRAMFrmLen(UCHAR ucCard, BYTE byCh);//查询指定通道帧长，单位为字节数
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_SetRAMFrmTCnt(UCHAR ucCard, BYTE byCh,DWORD dwFrmCnt);//设置指定通道发送帧数 dwFrmCnt为LVDS_RAM_SEND_INFINITE(0)表示无限发送，其他值为有限发送
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetRAMFrmTCnt(UCHAR ucCard, BYTE byCh);//查询指定通道设置发送帧数
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_RstRAMFrmHadTCnt(UCHAR ucCard, BYTE byCh);//重置指定通道实际已发送帧数
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetRAMFrmHadTCnt(UCHAR ucCard, BYTE byCh);//查询指定通道实际已发送帧数
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_StartRAMSend(UCHAR ucCard, BYTE byCh, BOOL bStart);//启动或停止指定通道RAM方式发送
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_GetRAMSta(UCHAR ucCard, BYTE byCh);//查询指定通道RAM发送状态

//====================================================================
//6.以下为查询方式进行数据读取-----------建议采用采用该种方式
//====================================================================
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_QueryStart(UCHAR ucCard);//启动查询
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_QueryStop (UCHAR ucCard);//停止查询
LVDS_DLL_EXPORT DWORD __stdcall LVDS_GetCount(UCHAR ucCard, DWORD dwWaitMs = 20);//获取数据结构个数,dwWaitMs为等待数据时间,单位为ms,LVDS_WAIT_NONE为不等待,LVDS_WAIT_INFINITE为无限等待
LVDS_DLL_EXPORT DWORD __stdcall LVDS_ReadData(UCHAR ucCard, PLVDSDATA pstData, DWORD dwCount);//读取dwCount个数据结构 返回值为实际读取个数

//====================================================================
//7.以下为回调方式进行数据读取-----------不建议建议采用该种方式
//====================================================================
LVDS_DLL_EXPORT BOOL __stdcall LVDS_IntStart(UCHAR ucCard,LVDS_CALLBACK pfCallBack);
LVDS_DLL_EXPORT BOOL __stdcall LVDS_IntStop (UCHAR ucCard);

//====================================================================
//8.提供基本的读写双字接口-----------根据需要调用(具体协议咨询FPGA人员)
//====================================================================
LVDS_DLL_EXPORT DWORD __stdcall LVDS_ReadDWord (UCHAR ucCard,DWORD dwOffset);
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_WriteDWord(UCHAR ucCard,DWORD dwOffset,DWORD dwData);

#pragma pack()//恢复默认对齐，避免影响其它库
#ifdef __cplusplus
}
#endif

#endif
