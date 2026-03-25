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
*版 本 号 : V23.6.0
*创建日期 : 2017.11.20
*作    者 : wangyl
*说    明 ：本函数为了提供通用功能，有一些参数和函数是考虑到最大包络而设计
*****************************************************************************
*修改记录 :(日期 作者 版本 修改内容)
1） 2018.10.17 wangyl 修改BUF_SIZE大小为32K，修改ucBuf类型为UCHAR;
2） 2018.11.23 wangyl 增加总线查询接口，增加IsOpen，修改数据结构名称避免重定义;
3） 2019.08.02 wangyl LVDS修改LVDS_IntStart实现方式,解决因静态局部变量可能导致的数据错乱情况;
4） 2019.11.11 wangyl LVDS修改支持的最大板卡数据量由16修改为32;
5） 2020.04.09 wangyl V2.5 按照修订协议V2.1做相应修改,1)时钟可查2)接收发送波特率等可设3)发送接收帧可设置4)扩展通道数为32;
6） 2020.05.20 wangyl V2.6 按照修订协议V2.2做相应修改,1)增加RAM模式发送数据;
7） 2020.06.03 wangyl V2.7 函数LVDS_GetCount增加超时时间;
8） 2020.07.07 wangyl V2.8 修改函数LVDS_Send在单次发送量超出发送FIFO大小时错误;
9） 2020.07.08 wangyl V2.9 为解决库调用方在使用库字节对齐与库不一致导致问题，增加pack对齐编译设置,只修改头文件;
10）2020.07.09 wangyl V2.10 函数LVDS_RstRAMFrmHadTCnt修改返回值类型，由DWORD修改为BOOL;
11）2020.12.21 wangyl V3.0 增加DMA发送函数;
12）2021.01.07 wangyl V3.1 增加查询VendorID和DevID功能,增加函数LVDS_GetDrvVer,增加LVDS_CheckVer校验驱动版本,修改统计板卡数机制,实现互斥打开板卡;
13）2021.03.05 wangyl V3.2 增加状态统计函数(接收FIFO满次数,驱动缓存覆盖次数,中断计数),增加接收通道数据量统计,RAM设置数据增加RAM地址参数,增加版本匹配函数;
14）2021.06.19 wangyl V3.3 增加最大板卡数量定义,增加总线映射表查询函数LVDS_GetBusTable和映射函数LVDS_MapBus2CardNo;
15）2021.09.28 wangyl V3.4 修改状态量统计:增加FGPA、驱动、动态库发送和接收(读取)字节数统计;
16）2021.10.12 wangyl V3.5 为适配16位版本修改版本判断,另外RAM增加帧长设置;
17）2021.12.20 wangyl V3.6 设置和获取RAM帧间隔增加是否使用时钟单位选项，修改是否支持RAM判断;
18）2022.04.11 wangyl V3.7 使用内存映射提升读写性能,查询接收优化,状态统计更新到驱动,增加BAR读写接口,增加串口接口类型配置,修改以支持并行发送;
19）2022.04.13 wangyl V4.0 增加PPS、指令应答和连续发送，增加校验方式,增加LVDS_OpenWithInit;
20）2022.09.02 wangyl V4.1 ST_LVDS_CFG增加ucBitMode,应答函数接口进行优化修改,3.0协议修改波特率计算公式;
21）2022.10.10 wangyl V4.2 修正比特模式设置地址错误,修正2线制帧头帧尾字节数设置,32位动态库导出名称去掉下划线,修改PPS一些问题;
22）2022.10.31 wangyl V4.3 字节顺序配置不再判版本,增加中断触发字节设置函数,增加单通道多组帧接收配置;
23）2022.11.14 wangyl V4.4 修改秒脉冲时间相关函数单位为us;
24）2022.12.07 wangyl V4.5 增加SN号相关接口函数(LVDS_GetKeyTable,LVDS_MapSN2CardNo,LVDS_MapSlot2CardNo,LVDS_GetSlot,LVDS_GetSN),增加调试开关接口LVDS_SetDebugOut;
25）2022.12.14 wangyl V4.6 修改BAR索引以适配不同BAR情况,异步串口数据位支持16位,解决双FIFO打印信息异常问题,双FIFO发送支持非DMA方式;
26）2023.04.06 wangyl V23.4.0 修改版本格式(年,月,版本号);修改BAR空间访问以适配Win10;
27）2023.05.05 wangyl V23.5.0 PPS对于时间数据类型修改为浮点类型;
28）2023.05.29 wangyl V23.5.1 增加LVDS_AbortSend和LVDS_SetRAMFrmDataEx;
29）2023.06.08 wangyl V23.6.0 1)修改PPS状态统计:增加当前周期和脉宽统计,函数LVDS_PPS_GetStas与之前版本不兼容,增加LVDS_QueryStartEx增加使能通道参数;
                              2)增加监视模式函数LVDS_SetWatchMode和LVDS_GetWatchMode,增加实例统计函数LVDS_GetInstanceCnt;
30）2023.07.07 wangyl V23.7.0 补充PPS常高和常低说明,修改PPS通道数量为32,增加信号配置时钟频率配置和查询;
*****************************************************************************/

//====================================================================
//以下为类型定义：包括缓存大小、通道号、数据结构体、配置结构体、回调类型
//====================================================================
#define LVDS_BUF_SIZE            (32*1024) //定义BUF大小为32K

#define LVDS_MIN_CARDNO         0  //最小板卡号
#define LVDS_MAX_CARDNO         31 //最大板卡号
#define LVDS_MAX_CARDCNT	    (LVDS_MAX_CARDNO+1) //支持最大板卡数量

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

//以下为常用波特率定义(对于未列出的可直接使用常数)
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

//以下为数据位定义(建议使用该定义,不要使用自定义魔数)
enum
{
	LVDS_DATABITS_1 = 0,//1位数据位
	LVDS_DATABITS_2,//2位数据位
	LVDS_DATABITS_3,//3位数据位
	LVDS_DATABITS_4,//4位数据位
	LVDS_DATABITS_5,//5位数据位
	LVDS_DATABITS_6,//6位数据位
	LVDS_DATABITS_7,//7位数据位
	LVDS_DATABITS_8,//8位数据位
	LVDS_DATABITS_16,//16位数据位
	LVDS_DATABITS_16X,//
};

//以下为停止位定义(建议使用该定义,不要使用自定义魔数)
enum
{
	LVDS_STOPBITS_10 = 0,//1位停止位
	LVDS_STOPBITS_15,//1.5位停止位
	LVDS_STOPBITS_20,//2位停止位
};

//以下为校验位定义(建议使用该定义,不要使用自定义魔数)
enum
{
	LVDS_PARITY_NO = 0,  //无校验
	LVDS_PARITY_ODD, //奇校验
	LVDS_PARITY_EVEN,//偶校验
	LVDS_PARITY_MARK,//MARK校验
	LVDS_PARITY_SPACE,//Space校验
};

//以下为收发比特模式定义
enum
{
	LVDS_BIT_LOW_FIRST,  //低位先
	LVDS_BIT_HIGH_FIRST,  //高位先
};

//定义异步串口通道配置参数结构体(如果对于有些成员没用到可忽略)
typedef struct 
{
	DWORD  dwBaudRate;//波特率:对应LVDS_BAUD_1200~LVDS_BAUD_256000,对于未列出的波特率也支持(如果使用时钟则该参数单位为时钟个数)
	UCHAR  ucDataBits;//数据位:为LVDS_DATABITS_1~LVDS_DATABITS_8
	UCHAR  ucStopBits;//停止位:为LVDS_STOPBITS_10~LVDS_STOPBITS_20
	UCHAR  ucParity;  //校验位:为LVDS_PARITY_NO~LVDS_PARITY_EVEN
	UCHAR  ucBitMode;// Bit顺序:LVDS_BIT_LOW_FIRST:低位先,LVDS_BIT_HIGH_FIRST:高位先
	DWORD  dwByteTime;//字节间隔:单位为系统时钟(LVDS_GetFpgaFreqEx查询)个数
}ST_LVDS_CFG,*PLVDSCFG;

//定义帧设置结构体
typedef struct
{
	BOOL  bEnable;  //帧使能:TRUE为使能,FALSE为禁止
	DWORD dwFrmLen; //帧长:单位为字节,例如0x400表示1024帧长
	DWORD dwFrmTime;//帧间隔:单位为FPGA时钟个数(时钟频率可通过LVDS_GetFpgaFreqEx进行查询)，对于接收无效
	DWORD dwFrmHead;//帧头:最多4字节,例如0x1ACFFC1D表示帧头1A CF FC 1D
	DWORD dwFrmEnd; //帧尾:最多4字节,例如0xEB9155AA表示帧尾EB 91 55 AA
}ST_LVDS_FRM_CFG,*PSTLVDSFRMCFG;

#ifndef TYPEDEF_ST_PCI_INFO
#define TYPEDEF_ST_PCI_INFO
typedef struct 
{
	ULONG ulBusNo;//总线号
	ULONG ulDevNo;//设备号
	ULONG ulFunc;//功能号
	ULONG ulVenID;//设备VendorID(与设备管理器的VendID对应)
	ULONG ulDevID;//设备DeviceID(与设备管理器的DevID对应)
}ST_PCI_INFO,*PPCIINFO;
#endif

//识别信息结构体定义
#ifndef TYPEDEF_ST_KEY_INFO
#define TYPEDEF_ST_KEY_INFO
#define INVALID_SN_NO   0xFFFFFFFFFFFFFFFF //无效SN号定义
#define INVALID_SLOT_NO 0xFF //无效插槽号定义
#define SLOT_NO_MIN     1 //最小插槽号
#define SLOT_NO_MAX     32//最大插槽号
#pragma pack(8) //为了兼容其他板卡的对齐
typedef struct 
{
	UCHAR     ucCard;//板卡号(从0开始)
	UCHAR     ucSlot;//所在机箱插槽号(从1开始)
	ULONGLONG ulSN;  //SN号(唯一标识)
}ST_KEY_INFO,*PKEYINFO;
#pragma pack(4) //恢复默认对齐
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

//版本校验结果
#ifndef PXI_CHECK_RESULT
#define PXI_CHECK_RESULT
enum
{
	PXI_CHECK_ERR,         //校验失败:板卡未打开
	PXI_CHECK_OK,          //校验成功:动态库匹配驱动和FPGA
	PXI_CHECK_ERR_DRV,     //校验失败:动态库匹配FPGA,不匹配驱动
	PXI_CHECK_ERR_FPGA,    //校验失败:动态库匹配驱动,不匹配FPGA
	PXI_CHECK_ERR_DRV_FPGA,//校验失败:动态库不匹配FPGA,不匹配驱动
};
#endif

//通信状态结构体定义
typedef struct 
{
	//FPGA层收发数据量统计
	ULONGLONG ulFpgaBusTBytes[LVDS_DATA_CNT];//FPGA层总线:各通道发送字节数(内部间)
	ULONGLONG ulFpgaIntTBytes[LVDS_DATA_CNT];//FGPA层接口:各通道发送字节数(与外部)
	ULONGLONG ulFpgaBusRBytes[LVDS_DATA_CNT];//FPGA层总线:各通道接收字节数(内部间)
	ULONGLONG ulFpgaIntRBytes[LVDS_DATA_CNT];//FGPA层接口:各通道接收字节数(与外部)

	//驱动层收发数据量统计
	ULONGLONG ulDrvTBytes[LVDS_DATA_CNT];//驱动层:各通道发送字节数
	ULONGLONG ulDrvRBytes[LVDS_DATA_CNT];//驱动层:各通道接收字节数

	//动态库层收发数据量统计
	ULONGLONG ulDllSendBytes[LVDS_DATA_CNT];//动态库层:各通道发送字节数(调用动态库发送数据的字节数)
	ULONGLONG ulDllReadBytes[LVDS_DATA_CNT];//动态库层:各通道读取字节数(调用动态库读取数据的字节数)

	//中断计数统计
	ULONGLONG ulFpgaIntTCnt[LVDS_DATA_CNT];//FPGA层：发送中断计数
	ULONGLONG ulFpgaIntRCnt[LVDS_DATA_CNT];//FPGA层：接收中断计数
	ULONGLONG ulDrvIntTCnt[LVDS_DATA_CNT]; //驱动层：发送中断计数
	ULONGLONG ulDrvIntRCnt[LVDS_DATA_CNT]; //驱动层：接收中断计数

	//溢出和空统计
	ULONGLONG ulDrvBufTEmptyCnt;  //驱动发送缓存空闲次数(所有通道)
	ULONGLONG ulDrvBufRFullCnt;   //驱动接收缓存覆盖次数(所有通道)
	ULONGLONG ulFpgaFifoTEmptyCnt[LVDS_DATA_CNT];//FPGA发送FIFO空次数
	ULONGLONG ulFpgaFifoTFullCnt[LVDS_DATA_CNT]; //FPGA发送FIFO满次数
	ULONGLONG ulFpgaFifoREmptyCnt[LVDS_DATA_CNT];//FPGA接收FIFO空次数
	ULONGLONG ulFpgaFifoRFullCnt[LVDS_DATA_CNT]; //FPGA接收FIFO满次数
}ST_LVDS_STATUS,*PSTLVDSSTATUS;

//定义串口接口类型
enum
{
	LVDS_INTTYPE_RS232 = 0x00,//串口232
	LVDS_INTTYPE_RS422 = 0x01,//串口422
	LVDS_INTTYPE_RS485 = 0x02,//串口485
};

//以下为帧计数结构体定义
typedef struct 
{
	BOOL  bUseFrmCnt;//帧计数模式,TRUE为使用,FALSE为不使用
	DWORD dwBits; //帧计数占用bit数(例如8表示1字节)
	DWORD dwPos;  //帧计数起始bit(从0开始)
	DWORD dwStart;//帧计数起始值
	DWORD dwStop; //帧计数结束值
	int   iStep;  //帧计数步进值:负值表示递减
}ST_FRMCNT_CFG,*PSTFRMCNTCFG;

//定义接收回调函数类型
typedef void (*LVDS_CALLBACK)(UCHAR ucCard,  PLVDSDATA pstData, DWORD dwCount); //ucCard为板卡号,pstData为结构体数组指针 dwCount为结构体数组的个数

#ifndef INVALID_CARD_NO
#define INVALID_CARD_NO 0xFF
#endif

//====================================================================
//0.以下为插槽映射相关函数-----------用于解决多块同类型板卡插槽识别
//====================================================================
LVDS_DLL_EXPORT int    __stdcall  LVDS_GetBusTable(ST_PCI_INFO *pstInfo, int iMaxCnt);//获取所有该类型板卡所在总线表 iMaxCnt为LVDS_MAX_CARDCNT 返回值为表条目数(也为实际板卡数量),数组索引为板卡号(例如pstInfo[0]为板卡0信息)
LVDS_DLL_EXPORT UCHAR  __stdcall  LVDS_MapBus2CardNo(ULONG ulBusNo, ULONG ulDevNo); //由插槽的总线号和设备号转换为板卡号 ,返回值INVALID_CARD_NO表示该插槽未插入该类型板卡(或驱动不支持该功能)
LVDS_DLL_EXPORT int    __stdcall  LVDS_GetKeyTable(ST_KEY_INFO *pstInfo, int iMaxCnt);//获取所有该类型板卡所有识别信息表 iMaxCnt为PXI3011_MAX_CARDCNT 返回值为表条目数(也为实际板卡数量),数组索引为板卡号(例如pstInfo[0]为板卡0信息)
LVDS_DLL_EXPORT UCHAR  __stdcall  LVDS_MapSN2CardNo(ULONGLONG ulSN);//映射板卡SN号到板卡号ucCard 返回值INVALID_CARD_NO表示该SN号不存在
LVDS_DLL_EXPORT UCHAR  __stdcall  LVDS_MapSlot2CardNo(UCHAR ucSlot);//映射机箱插槽号(从1开始)到板卡号ucCard,返回值INVALID_CARD_NO表示指定插槽不存在响应板卡

//====================================================================
//1.以下为板卡基本操作-----------操作板卡必须接口
//====================================================================
LVDS_DLL_EXPORT int  __stdcall  LVDS_CountCards();//获取实际板卡数量
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_Open (UCHAR ucCard);//打开板卡(排他性打开) ucCard范围为LVDS_MIN_CARDNO~LVDS_MAX_CARDNO
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_Close(UCHAR ucCard);//关闭板卡
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_Reset(UCHAR ucCard);//复位板卡
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_Start(UCHAR ucCard, BOOL bStart);//启动或停止板卡
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_IsOpen(UCHAR ucCard);//查询板卡是否已经打开
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetInfo(UCHAR ucCard,PPCIINFO pstInfo);//查询板卡所在总线信息
LVDS_DLL_EXPORT UCHAR       __stdcall  LVDS_GetSlot(UCHAR ucCard);//获取板卡所在插槽号 返回INVALID_SLOT_NO无效插槽号
LVDS_DLL_EXPORT ULONGLONG   __stdcall  LVDS_GetSN(UCHAR ucCard);//获取板卡SN号 返回INVALID_SN_NO为无效SN号
LVDS_DLL_EXPORT BOOL        __stdcall  LVDS_SetSN(UCHAR ucCard,ULONGLONG ulSn);//写入板卡SN号

LVDS_DLL_EXPORT BOOL __stdcall  LVDS_OpenWithInit (UCHAR ucCard);//打开板卡并进行初始化(LVDS_Open+LVDS_Reset+LVDS_Start+LVDS_ClrFIFO)

LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetWatchMode  (UCHAR ucCard);//设置监视模式
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetWatchMode  (UCHAR ucCard);//查询监视模式
LVDS_DLL_EXPORT DWORD __stdcall LVDS_GetInstanceCnt(UCHAR ucCard);//获取打开板卡进程实例个数


//====================================================================
//2.以下为调试接口-----------操作板卡辅助接口
//====================================================================
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetStart(UCHAR ucCard);//获取板卡启动状态(FPGA寄存器)
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetVer(UCHAR ucCard);//获取版本(FPGA寄存器)
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetID(UCHAR ucCard);//获取设备ID(FPGA寄存器)
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetFpgaDate(UCHAR ucCard);//获取编译日期(FPGA寄存器)
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetPowSta(UCHAR ucCard);//获取供电状态(FPGA寄存器)
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetIntEn(UCHAR ucCard);//获取接收中断使能状态(FPGA寄存器)
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetFpgaFreq(UCHAR ucCard);  //查询FPGA总线时钟频率Hz(FPGA寄存器)(用于波特率、帧间隔等计算)
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetFpgaFreqEx(UCHAR ucCard);//查询FPGA系统时钟频率Hz(FPGA寄存器)(用于波特率、帧间隔等计算)
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_GetChDataWidth(UCHAR ucCard, BOOL bRcv, BYTE byCh, BYTE *pbyWidth);//查询指定通道发送或接收数据位宽 pbyWidth为LVDS_DATAWIDTH_1BYTE~LVDS_DATAWIDTH_4BYTE
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetDrvVer(UCHAR ucCard);//获取驱动版本
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetDllVer(UCHAR ucCard);//获取动态库版本 例如0x0102表示V1.2
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetFitDrvVers(DWORD *pdwVers,DWORD dwCnt=32);//查询该动态库适用的驱动版本,返回值为版本个数,例如返回3,pdwVers[0]=0x0500,pdwVers[1]=0x0501,pdwVers[2]=0x0502,表示支持V5.0,V5.1,V5.2
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetFitFpgaVers(DWORD *pdwVers,DWORD dwCnt=32);//查询该动态库适用的FPGA版本,返回值为版本个数
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_CheckVer(UCHAR ucCard);//校验当前动态库版本是否匹配当前设备的FPGA程序版本和驱动版本
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_RstStatus(UCHAR ucCard);//重置统计状态
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_GetStatus(UCHAR ucCard, ST_LVDS_STATUS *pstStatus);//读取统计状态
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_SetDebugOut(UCHAR ucCard = 0,BOOL bOut=TRUE);//设置指定板卡调试信息输出到调试工具(DebugView)
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_GetDebugOut(UCHAR ucCard = 0);//查询指定板卡调试信息输出开关

//====================================================================
//3.以下为板卡属性配置接口-----------操作板卡辅助接口
//====================================================================
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_ClrFIFO(UCHAR ucCard);//清空所有通道FIFO以及驱动中的待发送和已接收数据
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetOutTime(UCHAR ucCard,DWORD dwTimeMs=200);//设置所有通道接收中断超时时间(单位为ms)，默认为200ms,范围为[1,5000]
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetChOutTime(UCHAR ucCard, BYTE byCh, DWORD dwTimeMs=200);//设置指定通道接收中断超时时间(单位为ms)，默认为200ms,范围为[1,5000](不设置则默认使用全局)
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetChOutTime(UCHAR ucCard, BYTE byCh, DWORD *pdwTimeMs);//查询指定通道接收中断超时时间(单位为ms)
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetCfg(UCHAR ucCard, BOOL bRcv, BYTE byCh, PLVDSCFG pstCfg,BOOL bUseClk=FALSE);//设置指定发送通道或接收通道波特率等参数 bRcv为TRUE表示接收通道,FALSE表示发送通道,bUseClk为TRUE则dwBaudRate和dwByteTime单位为时钟个数
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetCfg(UCHAR ucCard, BOOL bRcv, BYTE byCh, PLVDSCFG pstCfg,BOOL bUseClk=FALSE);//查询指定发送通道或接收通道波特率等参数 bRcv为TRUE表示接收通道,FALSE表示发送通道,bUseClk为TRUE则dwBaudRate和dwByteTime单位为时钟个数
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetChFrmCfg(UCHAR ucCard, BOOL bRcv, BYTE byCh, PSTLVDSFRMCFG pstCfg);//设置指定通道发送或接收帧配置 bRcv为TRUE表示接收通道,FALSE表示发送通道
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetChFrmCfg(UCHAR ucCard, BOOL bRcv, BYTE byCh, PSTLVDSFRMCFG pstCfg);//查询指定通道发送或接收帧配置 bRcv为TRUE表示接收通道,FALSE表示发送通道
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetChRFrmCfg(UCHAR ucCard, BYTE byCh, BYTE byNo, PSTLVDSFRMCFG pstCfg);//设置指定通道第N组接收帧配置 byNo范围[0,7]表示0到7组
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetChRFrmCfg(UCHAR ucCard, BYTE byCh, BYTE byNo, PSTLVDSFRMCFG pstCfg);//查询指定通道第N组接收帧配置 byNo范围[0,7]表示0到7组
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetChIntType (UCHAR ucCard, BOOL bRcv, BYTE byCh, BYTE byType);//设置指定通道发送或接收串口接口类型 byType为LVDS_INTTYPE_RS232~LVDS_INTTYPE_RS485
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetChIntType (UCHAR ucCard, BOOL bRcv, BYTE byCh, BYTE *pbyType);//查询指定通道发送或接收串口接口类型
LVDS_DLL_EXPORT BOOL   __stdcall LVDS_SetIntTrigBytes(UCHAR ucCard, BOOL bRcv, BYTE byCh,DWORD dwBytes);//设置指定通道发送或接收中断触发字节数
LVDS_DLL_EXPORT DWORD  __stdcall LVDS_GetIntTrigBytes(UCHAR ucCard, BOOL bRcv, BYTE byCh);//获取指定通道发送或接收中断触发字节数

LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetSynTEn(UCHAR ucCard, BOOL bSynTEn);//设置同步发送使能
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetSynTEn(UCHAR ucCard, BOOL *pbSynTEn);//查询同步发送使能
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetSynTCh(UCHAR ucCard, DWORD dwSynCh);//设置同步通道：BIT3~BIT0分别表示通道3~通道0,比特位为1表示需要同步
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetSynTCh(UCHAR ucCard, DWORD *pdwSynCh);//查询同步通道

LVDS_DLL_EXPORT BOOL __stdcall  LVDS_SetSynTDelay(UCHAR ucCard, BYTE byCh, DWORD dwTimeNs);  //设置同步发送延时时间:单位为纳秒
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetSynTDelay(UCHAR ucCard, BYTE byCh, DWORD *pdwTimeNs);//查询同步发送延时时间:单位为纳秒
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_GetSynRUnLockCnt(UCHAR ucCard, BYTE byCh, DWORD *pdwCnt);//查询同步接收失锁次数
LVDS_DLL_EXPORT BOOL __stdcall  LVDS_RstSynRUnLockCnt(UCHAR ucCard, BYTE byCh);//重置同步接收失锁次数

//====================================================================
//4.以下为进行普通数据发送-----------单FIFO方式
//====================================================================
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_Send(UCHAR ucCard, BYTE byCh, BYTE *pbyBuf, DWORD dwBytes,DWORD dwWaitTime=LVDS_WAIT_INFINITE);//指定通道发送数据,返回值为成功放入发送FIFO字节数,dwWaitTime为等待时间(ms),LVDS_WAIT_INFINITE为一直等待发送数据放入FIFO
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_DmaSend(UCHAR ucCard, BYTE byCh, BYTE *pbyBuf, DWORD dwBytes,DWORD dwWaitTime=LVDS_WAIT_INFINITE);//指定通道使用DMA发送数据,返回值为成功放入发送FIFO发送字节数,dwWaitTime为等待时间(ms),LVDS_WAIT_INFINITE为一直等待发送数据放入FIFO
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_AbortSend(UCHAR ucCard);//终止发送(停止LVDS_Send或LVDS_DmaSend内部死循环)
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetChSendBytes(UCHAR ucCard, BYTE byCh);//获取指定发送通道实际(FPGA)已经发送字节数 与ST_LVDS_STATUS的ulFpgaBusTBytes相同
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_RstChSendBytes(UCHAR ucCard, BYTE byCh);//重置指定发送通道实际(FPGA)已经发送字节数

//====================================================================
//5.以下为进行连续数据发送-----------双FIFO方式
//====================================================================
//双FIFO统计信息结构体定义
typedef struct 
{
	DWORD dwStart;//启动状态
	DWORD dwFifo1Size;//FIFO1大小
	DWORD dwFifo1Free;//FIFO1可配置数据个数
	DWORD dwFifo1HadCfg;//FIFO1已配置字节数
	DWORD dwFifo1EmptyCnt;//FIFO1空闲次数
	DWORD dwFifo1RemainCnt;//FIFO1剩余个数
	DWORD dwFifo1ThisBytes; //往FIFO1当次配置字节数
	DWORD dwFifo1FullCnt;//FIFO1溢出计数

	DWORD dwFifo2Size;//FIFO2大小
	DWORD dwFifo2Free;//FIFO2可配置数据个数
	DWORD dwFifo2HadCfg;//FIFO2已配置帧数
	DWORD dwFifo2EmptyCnt;//FIFO2空闲次数
	DWORD dwFifo2RemainCnt;//FIFO2剩余个数
	DWORD dwFifo2FullCnt;//FIFO2溢出计数
	DWORD dwTotalTBytes;//接口已发送字节数
}ST_LVDS_2FIFO_STAS,*PSTLVDS2FIFOSTAS;

//以下为连续发送结构体定义
typedef struct 
{
	DWORD dwFrmLen; //帧长:单位为字节
	DWORD dwFrmTime;//帧间隔:单位系统时钟(LVDS_GetFpgaFreqEx查询)个数,1表示1周期秒,比如对于24M时钟,1表示1/24000000秒
	DWORD dwTMode;  //发送模式：为LVDS_BIT_LOW_FIRST或LVDS_BIT_HIGH_FIRST(未使用)
	DWORD dwByteTime;//字节间隔:单位系统时钟(LVDS_GetFpgaFreqEx查询)个数,1表示1周期秒,比如对于24M时钟,1表示1/24000000秒
}ST_T_CFG,*PSTTCFG;
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_2FIFO_Send   (UCHAR ucCard, BYTE byCh, BYTE *pbyBuf, DWORD dwBytes, ST_T_CFG *pstSeqCfg, DWORD dwSeqCnt, DWORD dwWaitTime=LVDS_WAIT_INFINITE);//指定通道发送数据,发送dwBytes字节, pstSeqCfg为帧序列,返回值为成功放入发送FIFO字节数
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_2FIFO_DmaSend(UCHAR ucCard, BYTE byCh, BYTE *pbyBuf, DWORD dwBytes, ST_T_CFG *pstSeqCfg, DWORD dwSeqCnt, DWORD dwWaitTime=LVDS_WAIT_INFINITE);//指定通道DMA发送数据,发送dwBytes字节, pstSeqCfg为帧序列,返回值为成功放入发送FIFO字节数
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_2FIFO_Rst(UCHAR ucCard, BYTE byCh);//复位双FIFO发送(重置所有统计状态和配置信息)
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_2FIFO_GetStas(UCHAR ucCard, BYTE byCh, ST_LVDS_2FIFO_STAS *pstStas);//查询双FIFO统计和配置信息
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_2FIFO_ClrFifo( UCHAR ucCard,BYTE byCh );

//====================================================================
//6.以下为进行连续数据发送-----------RAM方式
//====================================================================
enum 
{
	LVDS_RAM_SEND_INFINITE = 0,
};
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_IsSupportRAM(UCHAR ucCard);//查询是否支持RAM模式 只有FPGA版本V2.2及以上才支持(LVDS_GetVer低16位>=0x0202)
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetRAMSize(UCHAR ucCard, BYTE byCh);//查询指定通道RAM大小，单位为字节数
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_SetRAMFrmTime(UCHAR ucCard, BYTE byCh, DWORD dwTime,BOOL bUseClk=FALSE);//设置指定通道帧间隔，bUseClk为TRUE则单位为时钟个数,否则为us
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetRAMFrmTime(UCHAR ucCard, BYTE byCh,BOOL bUseClk=FALSE);//查询指定通道帧间隔，bUseClk为TRUE则返回值单位单位为时钟个数,否则为us
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_SetRAMFrmData(UCHAR ucCard, BYTE byCh, BYTE *pbyBuf, DWORD dwBytes,DWORD dwRamAdrBase = 0);//设置指定通道RAM帧数据，返回值为实际设置成功字节数
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_SetRAMFrmDataEx(UCHAR ucCard, BYTE byCh, DWORD *pdwBuf, DWORD dwCnt,DWORD dwRamAdrBase = 0);//设置指定通道RAM帧数据(按双字设置)，返回值为实际设置成功双字数
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_SetRAMFrmLen(UCHAR ucCard, BYTE byCh, DWORD dwFrmLen);//设置指定通道帧长，单位为字节数(不能超过RAM大小和LVDS_SetRAMFrmData大小)
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetRAMFrmLen(UCHAR ucCard, BYTE byCh);//查询指定通道帧长，单位为字节数
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_SetRAMFrmTCnt(UCHAR ucCard, BYTE byCh,DWORD dwFrmCnt);//设置指定通道发送帧数 dwFrmCnt为LVDS_RAM_SEND_INFINITE(0)表示无限发送，其他值为有限发送
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetRAMFrmTCnt(UCHAR ucCard, BYTE byCh);//查询指定通道设置发送帧数
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_RstRAMFrmHadTCnt(UCHAR ucCard, BYTE byCh);//重置指定通道实际已发送帧数
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetRAMFrmHadTCnt(UCHAR ucCard, BYTE byCh);//查询指定通道实际已发送帧数
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_StartRAMSend(UCHAR ucCard, BYTE byCh, BOOL bStart);//启动或停止指定通道RAM方式发送
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_GetRAMSta(UCHAR ucCard, BYTE byCh);//查询指定通道RAM发送状态:TRUE为正在发送,FALSE为查询失败或已停止发送
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_SetRAMFrmCntCfg(UCHAR ucCard, BYTE byCh, ST_FRMCNT_CFG *pstCfg);//设置指定通道帧计数配置
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_GetRAMFrmCntCfg(UCHAR ucCard, BYTE byCh, ST_FRMCNT_CFG *pstCfg);//获取指定通道帧计数配置
LVDS_DLL_EXPORT BOOL  __stdcall  LVDS_RstRAMHadTBytes(UCHAR ucCard, BYTE byCh);//重置指定通道实际已发送字节数
LVDS_DLL_EXPORT DWORD __stdcall  LVDS_GetRAMHadTBytes(UCHAR ucCard, BYTE byCh);//查询指定通道实际已发送字节数

//====================================================================
//7.以下为查询方式进行数据读取-----------建议采用该种方式
//====================================================================
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_QueryStart(UCHAR ucCard);//启动接收
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_QueryStartEx(UCHAR ucCard,DWORD dwMask=0xFFFFFFFF);//启动接收(与LVDS_QueryStart相比增加了dwMask) dwMask对应32通道接收使能,BIT0为1对应通道0使能
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_QueryStop (UCHAR ucCard);//停止接收
LVDS_DLL_EXPORT DWORD __stdcall LVDS_GetCount(UCHAR ucCard, DWORD dwWaitMs = 20);//获取数据结构个数,dwWaitMs为等待数据时间,单位为ms,LVDS_WAIT_NONE为不等待,LVDS_WAIT_INFINITE为无限等待
LVDS_DLL_EXPORT DWORD __stdcall LVDS_ReadData(UCHAR ucCard, PLVDSDATA pstData, DWORD dwCount);//读取dwCount个数据结构 返回值为实际读取个数
LVDS_DLL_EXPORT DWORD __stdcall LVDS_GetChRcvdBytes(UCHAR ucCard, BYTE byCh);//获取指定接收通道实际(FPGA)已经接收字节数(也可通过LVDS_GetStatus获取更详细信息)
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_RstChRcvBytes (UCHAR ucCard, BYTE byCh);//重置指定接收通道实际(FPGA)已经接收字节数

//====================================================================
//8.以下为进行应答数据配置
//====================================================================
#define LVDS_REPLY_ITEM_MIN        0     //应答条目表最小索引号
#define LVDS_REPLY_ITEM_MAX        15    //应答条目表最大索引号
#define LVDS_REPLY_ITEM_CNT        (LVDS_REPLY_ITEM_MAX-LVDS_REPLY_ITEM_MIN+1)    //应答条目表数

#define LVDS_MATCH_BYTE_MIN        1     //指令匹配最小字节数
#define LVDS_MATCH_BYTE_MAX        31    //指令匹配最大字节数
#define LVDS_REPLY_BYTE_MIN        1     //应答数据最小字节数
#define LVDS_REPLY_BYTE_MAX        1022  //应答数据最大字节数

//应答索引源定义
enum
{
	REPLY_SRC_BACKBOARD = 0x100,//背板
	REPLY_SRC_EXTERNAL  = 0x200,//外部信号
};

//应答参数配置结构体
typedef struct 
{
	BOOL  bMatchEnable;//使能匹配(默认使能)
	BOOL  bReplyEnable;//使能应答(默认使能)
	DWORD dwReplyDelayTime; //应答延时(单位为系统时钟个数：LVDS_GetFpgaFreqEx)
	DWORD dwSrc; //解析源索引   [0x00,0xFF]为板内通道,REPLY_SRC_BACKBOARD为背板,REPLY_SRC_EXTERNAL为外部信号(默认板内通道)
	DWORD dwDest;//应答目的索引 [0x00,0xFF]为板内通道,REPLY_SRC_BACKBOARD为背板,REPLY_SRC_EXTERNAL为外部信号(默认板内通道)
}ST_REPLY_CFG,*PSTREPLYCFG;

//应答状态统计结构体
typedef struct 
{
	DWORD dwMatchCnt;  //已匹配次数
	DWORD dwMatchBytes;//已匹配字节数
	DWORD dwReplyCnt;  //已应答次数
	DWORD dwReplyBytes;//已应答字节数
}ST_REPLY_STAS,*PSTREPLYSTAS;

LVDS_DLL_EXPORT BOOL  __stdcall LVDS_SetReply(UCHAR ucCard,BYTE byCh,int iItemIndex,BYTE *pbyMatchBuf,DWORD dwMatchBytes,BYTE *pbyReplyBuf,DWORD dwReplyBytes);//进行应答数据配置并使能,iItemIndex为[LVDS_REPLY_ITEM_MIN,LVDS_REPLY_ITEM_MAX],pbyMatchBuf为匹配缓存(dwMatchBytes为其有效字节数),pbyReplyBuf为应答缓存(dwReplyBytes为其有效字节数)

LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Reply_Rst(UCHAR ucCard, BYTE byCh);//进行应答数据配置重置
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Reply_SetCfg(UCHAR ucCard, BYTE byCh, ST_REPLY_CFG *pstCfg);//进行应答参数配置
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Reply_GetCfg(UCHAR ucCard, BYTE byCh, ST_REPLY_CFG *pstCfg);//获取应答参数配置
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Reply_SetFrmCfg(UCHAR ucCard, BYTE byCh, ST_FRMCNT_CFG *pstCfg);//进行应答帧计数配置
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Reply_GetFrmCfg(UCHAR ucCard, BYTE byCh, ST_FRMCNT_CFG *pstCfg);//获取应答帧计数配置
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Reply_GetStas(UCHAR ucCard, BYTE byCh, ST_REPLY_STAS *pstStas);//查询应答统计
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Reply_RstStas(UCHAR ucCard, BYTE byCh);//重置应答统计

//====================================================================
//9.以下为进行秒脉冲功能配置
//====================================================================
#define LVDS_PPS_MIN_CHNO         0  //秒脉冲最小通道号
#define LVDS_PPS_MAX_CHNO         31 //秒脉冲最大通道号
#define LVDS_PPS_MAX_CHCNT	     (LVDS_PPS_MAX_CHNO-LVDS_PPS_MIN_CHNO+1) //秒脉冲支持最大通道数量

//脉冲模式
enum
{
	LVDS_PPS_MODE_LOW = 0x00,//低脉冲
	LVDS_PPS_MODE_HIGH ,//高脉冲
};

enum
{
	LVDS_PPS_WIDTH_CONSTANT = 0,//配合LVDS_PPS_MODE_LOW和LVDS_PPS_MODE_HIGH设置为常高或常低
};
enum
{
	LVDS_PPS_T_INFINITE = 0,//连续发送
};
//秒脉冲接收信息结构体定义
typedef struct
{
	BYTE      byCh;//通道号
	DWORD     dwFlag;//标识
	ULONGLONG ulTimeTag;//时间戳(单位为PPS时钟计数,通过LVDS_PPS_GetStas可查询PPS时钟频率)
	double    dWidthUs;//脉宽(单位为us)
	double    dCycleUs;//周期(单位为us)
	DWORD     dwSeqNo;//计次(从1开始)
}ST_LVDS_PPS_DATA,*PSTLVDSPPSDATA;

//秒脉冲统计信息结构体定义
typedef struct 
{
	DWORD dwTMode;//发送:模式设置(只读)
	double dTWidthUs;//发送:脉宽设置(只读)
	double dTCycleUs;//发送:周期设置(只读)
	DWORD dwTSetCnt;//发送:设置脉冲计数(只读)
	DWORD dwHadTCnt;//发送:已累计发脉冲计数
	DWORD dwTStart;//发送:发送使能(只读)
	DWORD dwRMode;//接收:模式设置(只读)
	DWORD dwRCnt;//接收:可读取脉冲个数(只读)
	double dRMaxWidthUs;//接收:脉宽最大统计
	double dRMinWidthUs;//接收:脉宽最小统计
	double dRMaxCycleUs;//接收:周期最大统计
	double dRMinCycleUs;//接收:周期最小统计
	double dRCurWidthUs;//接收:脉宽当前统计（新增）
	double dRCurCycleUs;//接收:周期当前统计（新增）
	DWORD dwHadRCnt;//接收:已累计脉冲计数
	DWORD dwRStart;//接收:接收使能(只读)
	DWORD dwFreq; //时钟频率:单位Hz(只读)
}ST_LVDS_PPS_STAS,*PSTLVDSPPSSTAS;
LVDS_DLL_EXPORT BOOL   __stdcall LVDS_PPS_Rst(UCHAR ucCard, BYTE byCh);//复位PPS所有配置和状态信息
LVDS_DLL_EXPORT BOOL   __stdcall LVDS_PPS_RstStas(UCHAR ucCard, BYTE byCh);//重置PPS统计信息(不会重置只读状态)
LVDS_DLL_EXPORT BOOL   __stdcall LVDS_PPS_GetStas(UCHAR ucCard, BYTE byCh,ST_LVDS_PPS_STAS *pstStas,int iSize=sizeof(ST_LVDS_PPS_STAS));//获取PPS统计信息

//★★★注意：dWidthUs为LVDS_PPS_WIDTH_CONSTANT(0)时,如果dwMode为LVDS_PPS_MODE_LOW则为常低,dwMode为LVDS_PPS_MODE_HIGH则为常高★★★
LVDS_DLL_EXPORT BOOL   __stdcall LVDS_PPS_SetT   (UCHAR ucCard, BYTE byCh, DWORD dwMode, double dWidthUs, double dCycleUs, DWORD dwPulseCnt);//设置PPS发送:byMode位模式(低脉冲或高脉冲),dwWidthUs为脉宽(单位us)，dwCycleUs为周期(单位为us),dwPulseCnt为个数(LVDS_PPS_T_INFINITE为连续)
LVDS_DLL_EXPORT BOOL   __stdcall LVDS_PPS_StartT (UCHAR ucCard, BYTE byCh, BOOL bStart);//启停PPS发送:bStart为TRUE表示启动,为FALSE表示停止

LVDS_DLL_EXPORT BOOL   __stdcall LVDS_PPS_SetR   (UCHAR ucCard, BYTE byCh, DWORD dwMode);//设置PPS接收:byMode脉冲模式(低脉冲或高脉冲)
LVDS_DLL_EXPORT BOOL   __stdcall LVDS_PPS_StartR (UCHAR ucCard, BYTE byCh, BOOL bStart);//启停PPS接收:bStart为TRUE表示启动,为FALSE表示停止
LVDS_DLL_EXPORT BOOL   __stdcall LVDS_PPS_ClrRData(UCHAR ucCard, BYTE byCh);//清空秒脉冲已接收数据
LVDS_DLL_EXPORT DWORD  __stdcall LVDS_PPS_GetRCnt (UCHAR ucCard, BYTE byCh);//查询秒脉冲已接收计数
LVDS_DLL_EXPORT DWORD  __stdcall LVDS_PPS_GetRData(UCHAR ucCard, BYTE byCh, ST_LVDS_PPS_DATA *pstDatas, DWORD dwPulseCnt);//读取PPS接收脉冲信息,dwPulseCnt为最大读取个数,返回值为实际读取个数

//====================================================================
//10.以下为回调方式进行数据读取-----------不建议建议采用该种方式
//====================================================================
//LVDS_DLL_EXPORT BOOL __stdcall LVDS_IntStart(UCHAR ucCard,LVDS_CALLBACK pfCallBack);
//LVDS_DLL_EXPORT BOOL __stdcall LVDS_IntStop (UCHAR ucCard);

//====================================================================
//11.提供信号配置读取接口-----------调试接口(具体协议咨询FPGA人员)
//====================================================================
//门控有效设置
enum
{
	LVDS_SIG_GATEVALID_LOW  = 0x00,//门控低有效
	LVDS_SIG_GATEVALID_HIGH = 0x03,//门控高有效
};

//置数时钟沿设置
enum
{
	LVDS_SIG_CLKDPUSH_DOWN  = 0x00,//时钟下降沿置数
	LVDS_SIG_CLKDPUSH_UP    = 0x03,//时钟上升沿置数
};

//无时钟时时钟线状态
enum
{
	LVDS_SIG_CLKSTATIC_LOW  = 0x00,//低
	LVDS_SIG_CLKSTATIC_HIGH = 0x01,//高
};

//发送数据上线前及保持后的状态
enum
{
	LVDS_SIG_DATASTATIC_LOW   = 0x00,//低电平
	LVDS_SIG_DATASTATIC_HIGH  = 0x01,//高电平
	LVDS_SIG_DATASTATIC_HOLD  = 0x02,//保持最后一位
};

//发送时钟是否常有
enum
{
	LVDS_SIG_CLKALWS_NO   = 0x00,//不常有
	LVDS_SIG_CLKALWS_YES  = 0x01,//常有
};

//发送码型
enum
{
	LVDS_SIG_CODETYPE_RZ     = 0x00,//RZ
	LVDS_SIG_CODETYPE_NRZ    = 0x01,//NRZ
	LVDS_SIG_CODETYPE_NRZI   = 0x02,//NRZI
	LVDS_SIG_CODETYPE_MANCH  = 0x03,//曼彻斯特
};

//发送数据单位
enum
{
	LVDS_SIG_DATAWIDTH_BYTE   = 0x00,//字节
	LVDS_SIG_DATAWIDTH_WORD   = 0x01,//字
	LVDS_SIG_DATAWIDTH_DWORD  = 0x02,//双字
};

//bit顺序
enum
{
	LVDS_SIG_BITORDER_LSB   = 0x00,//低位先
	LVDS_SIG_BITORDER_HSB   = 0x01,//高位先
};

//停止发送模式
enum
{
	LVDS_SIG_STOPMODE_IMMEDIATELY = 0x00,//立即停止
	LVDS_SIG_STOPMODE_AFTERCURFRM = 0x01,//发送完当前帧停止
};

//取数时钟沿设置
enum
{
	LVDS_SIG_CLKDPULL_DOWN  = 0x00,//时钟下降沿取数
	LVDS_SIG_CLKDPULL_UP    = 0x01,//时钟上升沿取数
};

//发送时序配置
typedef struct 
{
	DWORD dwGateValid;  //门控有效设置(LVDS_SIG_GATEVALID_LOW,LVDS_SIG_GATEVALID_HIGH)
	DWORD dwClkDPush;   //置数时钟沿设置(LVDS_SIG_CLKDPUSH_DOWN,LVDS_SIG_CLKDPUSH_UP)
	DWORD dwClkDuty;    //时钟占空比(单位为百分比,45~55)
	DWORD dwClkFreq;    //接口时钟频率(单位Hz)
	DWORD dwClkStatic;  //无时钟时时钟线状态(LVDS_SIG_CLKSTATIC_LOW,LVDS_SIG_CLKSTATIC_HIGH)
	DWORD dwDataStatic; //数据上线前及保持后的状态(LVDS_SIG_DATASTATIC_LOW~LVDS_SIG_DATASTATIC_HOLD)
	DWORD dwTg2fc;      //门控到第一个时钟沿的时间(系统时钟个数)
	DWORD dwThg;        //门控到最后一个时钟沿保持时间(系统时钟个数)
	DWORD dwTd2c;       //数据置数与时钟置数沿之间的时间(系统时钟个数)
	DWORD dwTfd2fc;     //首数据位上线与第一个时钟置数沿之间的时间(系统时钟个数)
	DWORD dwThld;       //未数据位下线到最后一个时钟置数沿的保持时间(系统时钟个数)
	DWORD dwClkAlws;    //时钟是否常有(LVDS_SIG_CLKALWS_NO,LVDS_SIG_CLKALWS_YES)
	DWORD dwCodeType;   //发送码型(LVDS_SIG_CODETYPE_RZ~LVDS_SIG_CODETYPE_MANCH)
	DWORD dwDataWidth;  //发送数据单位(LVDS_SIG_DATAWIDTH_BYTE~LVDS_SIG_DATAWIDTH_DWORD)
	DWORD dwBitOder;    //发送bit顺序(LVDS_SIG_BITORDER_LSB,LVDS_SIG_BITORDER_HSB)
	DWORD dwStopMode;   //停止发送模式(LVDS_SIG_STOPMODE_IMMEDIATELY,LVDS_SIG_STOPMODE_AFTERCURFRM)
}ST_CLK_CFG_T,*PSTCLKCFGT;

//接收时序配置
typedef struct 
{
	DWORD dwGateValid;  //门控有效设置(LVDS_SIG_GATEVALID_LOW,LVDS_SIG_GATEVALID_HIGH)
	DWORD dwClkDPull;   //接收取数时钟沿(LVDS_SIG_CLKDPULL_DOWN,LVDS_SIG_CLKDPULL_UP)
	DWORD dwDataWidth;  //接收数据单位(LVDS_SIG_DATAWIDTH_BYTE~LVDS_SIG_DATAWIDTH_DWORD)
	DWORD dwBitOder;    //接收bit顺序(LVDS_SIG_BITORDER_LSB,LVDS_SIG_BITORDER_HSB)
}ST_CLK_CFG_R,*PSTCLKCFGR;


#define MAX_2LINE_R_SYN_BYTES 8
//2线制时序配置
typedef struct  
{
	BOOL bTSynEnable;//两线发送同步使能
	BYTE byTSynData;//两线发送同步数据

	BOOL bRSynHeadEnable;//接收数据帧头使能
	BYTE byRSynHeadBytes;//接收数据帧头字节数(标识byRSynHead有效字节数，例如1A CF FC,则byRSynHeadBytes为3)
	BYTE byRSynHead[MAX_2LINE_R_SYN_BYTES];//接收同步帧头

	BOOL bRSynTailEnable;//接收数据帧尾使能
	BYTE byRSynTailBytes;//接收数据帧尾字节数(标识byRSynTail有效字节数，例如eb 90,则byRSynTailBytes为2)
	BYTE byRSynTail[MAX_2LINE_R_SYN_BYTES];//接收同步帧尾

	DWORD dwRFrmLen;//接收数据帧帧长度
	DWORD dwRFrmLenBytes;//接收数据帧帧长度字节数
	DWORD dwRFrmLenStart;//接收数据帧帧长度起始字节位置
	BOOL  bRFrmLenEnable;//接收数据帧帧长度使能

	BOOL bRSynEnable;//两线接收同步使能
	BYTE byRSynData;//两线接收同步数据
}ST_CLK_CFG_2LINE,*PSTSIGCFG2LINE;

LVDS_DLL_EXPORT DWORD __stdcall LVDS_Cfg_GetClk(UCHAR ucCard, BYTE byCh);//获取发送时钟频率
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Cfg_SetClk(UCHAR ucCard, BYTE byCh, DWORD dwClkFreq);//设置发送时钟频率,dwClkFreq与ST_CLK_CFG_T的dwClkFreq一样,为便于使用单独提出接口

LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Cfg_GetT(UCHAR ucCard, BYTE byCh, ST_CLK_CFG_T *pstCfg);//获取发送时序配置
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Cfg_SetT(UCHAR ucCard, BYTE byCh, ST_CLK_CFG_T *pstCfg);//设置发送时序配置

LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Cfg_GetR(UCHAR ucCard, BYTE byCh, ST_CLK_CFG_R *pstCfg);//获取接收时序配置
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Cfg_SetR(UCHAR ucCard, BYTE byCh, ST_CLK_CFG_R *pstCfg);//设置接收时序配置

LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Cfg_Get2Line(UCHAR ucCard, BYTE byCh, ST_CLK_CFG_2LINE *pstCfg);//获取2线制时序配置
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Cfg_Set2Line(UCHAR ucCard, BYTE byCh, ST_CLK_CFG_2LINE *pstCfg);//设置2线制时序配置

LVDS_DLL_EXPORT BOOL  __stdcall LVDS_Cfg_SaveToEeprom(UCHAR ucCard, BYTE byCh);//保存时序配置:写入到EEPROM中

//====================================================================
//12.提供基本的读写双字接口-----------根据需要调用(具体协议咨询FPGA人员)
//====================================================================
LVDS_DLL_EXPORT DWORD __stdcall LVDS_ReadDWord (UCHAR ucCard, DWORD dwOffset);//dwOffset为FPGA地址
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_WriteDWord(UCHAR ucCard, DWORD dwOffset,DWORD dwData);//dwOffset为FPGA地址
LVDS_DLL_EXPORT DWORD __stdcall LVDS_ReadBarDWord (UCHAR ucCard, DWORD dwBar, DWORD dwOffset);//dwBar为Bar索引(0~5,按照实际Bar空间使用)
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_WriteBarDWord(UCHAR ucCard, DWORD dwBar, DWORD dwOffset, DWORD dwData);//dwBar为Bar索引(0~5,按照实际Bar空间使用)

//====================================================================
//13.提供自定义寄存器接口-----------根据需要调用(具体协议咨询FPGA人员)
//====================================================================
#define LVDS_REG_INDEX_MIN    0
#define LVDS_REG_INDEX_MAX    100000
LVDS_DLL_EXPORT DWORD __stdcall LVDS_ReadRegByIndex (UCHAR ucCard,DWORD dwIndex);//读取自定义寄存器,dwIndex为LVDS_REG_INDEX_MIN~LVDS_REG_INDEX_MAX
LVDS_DLL_EXPORT BOOL  __stdcall LVDS_WriteRegByIndex(UCHAR ucCard,DWORD dwIndex,DWORD dwData);//写入自定义寄存器,dwIndex为LVDS_REG_INDEX_MIN~LVDS_REG_INDEX_MAX
LVDS_DLL_EXPORT DWORD __stdcall LVDS_GetRegOffByIndex(UCHAR ucCard,DWORD dwIndex);//通过寄存器索引查询寄存器地址,dwIndex为LVDS_REG_INDEX_MIN~LVDS_REG_INDEX_MAX


#pragma pack()//恢复默认对齐，避免影响其它库
#ifdef __cplusplus
}
#endif

#endif
