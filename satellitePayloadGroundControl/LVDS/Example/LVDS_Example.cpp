// LVDS_Example.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "..\\LVDS_DLL\LVDS_DLL.h"           //包含头文件 根据自己实际目录包含
#pragma comment(lib,"..\\lib\\LVDS_DLL.lib") //包含lib文件 根据自己实际目录包含

#define LVDS_CARD_NO        0 //板卡号 根据自己实际定义

#define USED_SEND_DEMO      1

#if USED_SEND_DEMO
//以下进行数据发送演示
int _tmain(int argc, _TCHAR* argv[])
{
	BOOL bRet = TRUE;

	if (LVDS_CountCards() <= 0)
	{
		return 0;
	}

	if (TRUE != LVDS_Open(LVDS_CARD_NO))
	{
		return 0;
	}

	bRet &= LVDS_Reset(LVDS_CARD_NO);//板卡复位 可选
	bRet &= LVDS_Start(LVDS_CARD_NO,TRUE);//启动板卡

	/*
	//通道参数配置 根据需要配置
	ST_LVDS_CFG stCfg; 
	memset(&stCfg,0,sizeof(stCfg));
	stCfg.dwBaudRate   = 9600;//波特率9600
	stCfg.ucDataBits   = LVDS_DATABITS_8;//8位停止位;
	stCfg.ucParity     = LVDS_PARITY_NO;//无校验
	stCfg.ucParity     = LVDS_STOPBITS_10;//1位停止位
	for (int i = 0; i < LVDS_DATA_CH16; i++)
	{
		bRet &= LVDS_SetCfg(LVDS_CARD_NO,FALSE,LVDS_DATA_CH1+i,&stCfg);//发送通道配置
	}*/

	/*
	//发送帧参数配置 根据需要配置
	ST_LVDS_FRM_CFG stFrmCfg; 
	memset(&stFrmCfg,0,sizeof(stFrmCfg));
	stFrmCfg.bEnable    = TRUE;       //帧使能
	stFrmCfg.dwFrmLen   = 0x1000;     //帧长:0x1000字节
	stFrmCfg.dwFrmTime  = 100;        //帧间隔:100个时钟周期
	stFrmCfg.dwFrmHead  = 0xEB90EB90;//帧头:0xEB90EB90
	stFrmCfg.dwFrmEnd   = 0x55AA55AA;//帧尾:0x55AA55AA
	for (int i = 0; i < LVDS_DATA_CH16; i++)
	{
		bRet &= LVDS_SetChFrmCfg(LVDS_CARD_NO,FALSE,LVDS_DATA_CH1+i,&stFrmCfg);//发送通道帧配置
	}*/

	/*********************下面演示普通发送数据(单次发送)*********************/
	BYTE byBuf[8]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};//待发送数据
	DWORD dwSendRet = LVDS_DmaSend(LVDS_CARD_NO,LVDS_DATA_CH1,byBuf,sizeof(byBuf)/sizeof(byBuf[0]),LVDS_WAIT_INFINITE);//超时等待,根据实际需要设置,LVDS_WAIT_INFINITE为一直等待

	/*********************下面演示RAM发送数据(连续发送)*********************/
	if (1)
	{
		BYTE byFrmBuf[10]={0xEB,0x90,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};

		LVDS_StartRAMSend(LVDS_CARD_NO,LVDS_DATA_CH1,FALSE);//先停止帧发送 
		LVDS_SetRAMFrmTime(LVDS_CARD_NO,LVDS_DATA_CH1,100);//设置帧间隔为100us
		LVDS_SetRAMFrmData(LVDS_CARD_NO,LVDS_DATA_CH1,byFrmBuf,sizeof(byFrmBuf)/sizeof(byFrmBuf[0]));//设置帧数据
		LVDS_SetRAMFrmLen(LVDS_CARD_NO,LVDS_DATA_CH1,10);//设置帧长（按照实际设置）
		LVDS_RstRAMFrmHadTCnt(LVDS_CARD_NO,LVDS_DATA_CH1);//重置已发送帧数 便于统计实际已发送帧数
		LVDS_SetRAMFrmTCnt(LVDS_CARD_NO,LVDS_DATA_CH1,10);//设置发送帧数：10帧
		LVDS_StartRAMSend(LVDS_CARD_NO,LVDS_DATA_CH1,TRUE);//启动帧发送 
		while (LVDS_GetRAMFrmHadTCnt(LVDS_CARD_NO,LVDS_DATA_CH1) < 10)
		{
			//实时查询实际已发送帧数
			Sleep(20);
		}
	}

	//退出程序时 进行板卡停止和板卡关闭操作
	LVDS_Start(LVDS_CARD_NO,FALSE);
	LVDS_Close(LVDS_CARD_NO);
	return 0;
}

#else
//以下进行数据接收演示
#define RCV_MAX_ST_CNT  16 //最大接收结构体个数
int _tmain(int argc, _TCHAR* argv[])
{
	BOOL bRet = TRUE;
	ST_LVDS_DATA *pstInfo = NULL;

	if (LVDS_CountCards() <= 0)
	{
		return 0;
	}

	if (TRUE != LVDS_Open(LVDS_CARD_NO))
	{
		return 0;
	}

	pstInfo = (ST_LVDS_DATA*)malloc(sizeof(ST_LVDS_DATA)*RCV_MAX_ST_CNT);
	bRet &= LVDS_Reset(LVDS_CARD_NO);//板卡复位 可选
	bRet &= LVDS_ClrFIFO(LVDS_CARD_NO);//清空缓存
	bRet &= LVDS_Start(LVDS_CARD_NO,TRUE);//启动板卡

	//通道参数配置 根据需要配置
	ST_LVDS_CFG stCfg; 
	memset(&stCfg,0,sizeof(stCfg));
	stCfg.dwBaudRate   = 9600;//波特率9600
	stCfg.ucDataBits   = LVDS_DATABITS_8;//8位停止位;
	stCfg.ucParity     = LVDS_PARITY_NO;//无校验
	stCfg.ucParity     = LVDS_STOPBITS_10;//1位停止位
	for (int i = 0; i < LVDS_DATA_CH16; i++)
	{
		bRet &= LVDS_SetCfg(LVDS_CARD_NO,TRUE,LVDS_DATA_CH1+i,&stCfg);//接收通道配置
	}

	//接收帧参数配置 根据需要配置
	ST_LVDS_FRM_CFG stFrmCfg; 
	memset(&stFrmCfg,0,sizeof(stFrmCfg));
	stFrmCfg.bEnable    = TRUE;       //帧使能
	stFrmCfg.dwFrmLen   = 0x1000;     //帧长:0x1000字节
	stFrmCfg.dwFrmTime  = 100;        //帧间隔:100个时钟周期
	stFrmCfg.dwFrmHead  = 0xEB90EB90;//帧头:0xEB90EB90
	stFrmCfg.dwFrmEnd   = 0x55AA55AA;//帧尾:0x55AA55AA
	for (int i = 0; i < LVDS_DATA_CH16; i++)
	{
		bRet &= LVDS_SetChFrmCfg(LVDS_CARD_NO,TRUE,LVDS_DATA_CH1+i,&stFrmCfg);//接收通道帧配置
	}

	/*********************下面演示数据接收(演示10次数据接收)*********************/
	DWORD dwRcvCnt = 0;//接收计数
	DWORD dwSTCnt = 0;//查询结构体个数
	LVDS_QueryStart(LVDS_CARD_NO);//启动数据接收
	while (dwRcvCnt < 10)
	{
		dwSTCnt = LVDS_GetCount(LVDS_CARD_NO);//查询缓存中已接收的结构体个数
		if (dwSTCnt > 0)
		{
			dwSTCnt = dwSTCnt > RCV_MAX_ST_CNT?RCV_MAX_ST_CNT:dwSTCnt;//读取个数不能超过RCV_MAX_ST_CNT
			DWORD dwRetCnt = LVDS_ReadData(LVDS_CARD_NO, pstInfo, dwSTCnt);//读取dwSTCnt个结构数据
			for (DWORD i = 0; i < dwRetCnt; i++)
			{
				printf("通道:%d,字节数:%d,数据:",pstInfo[i].emType,pstInfo[i].usLen);
				for (DWORD j = 0; j < pstInfo[i].usLen; j++)
				{
					printf("0x%02x ",pstInfo[i].ucBuf[j]);
				}
				printf("\r\n");
			}
		}
	}

	//退出程序时 进行板卡停止和板卡关闭操作
	LVDS_Start(LVDS_CARD_NO,FALSE);
	LVDS_Close(LVDS_CARD_NO);
	free(pstInfo);
	return 0;
}
#endif