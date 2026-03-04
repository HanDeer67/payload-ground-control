#ifndef APP_H
#define APP_H

#include <QtWidgets>

class App
{
public:
    static QString ConfigFile;          //配置文件文件路径及名称
    static QString LayoutPath;          //布局文件夹

    //基本配置参数
    static QString WorkMode;            //工作模式 timer-模拟数据 db-数据库采集 tcp-网络采集 http-post请求
    static QString MapStyle;            //中间地图样式 image point move
    static QString Ratio;               //大屏分辨率
    static QString Title;               //标题
    static QString Layout;              //布局方案
    static QString Theme;               //配色方案
    static QString VideoAddr;           //视频地址
    static QString HttpUrl;             //网络请求地址

    static bool AutoRun;                //开机启动
    static bool MoveEnable;             //模块是否可以拖动
    static bool CutLeftBottom;          //裁掉左下角合并到左侧布局
    static bool CutRightBottom;         //裁掉右下角合并到右侧布局
    static bool StaticLine;             //静态固定线条
    static bool ShowPercent;            //Y轴百分比显示
    static int StepY;                   //Y轴间隔数
    static int CursorHideTime;          //鼠标指针隐藏时间

    //颜色配置参数
    static QString ColorMainBg;         //主背景颜色
    static QString ColorPanelBg;        //面板颜色
    static QString ColorLine;           //十字线颜色
    static QString ColorLine1;          //线条1颜色
    static QString ColorLine2;          //线条2颜色
    static QString ColorLine3;          //线条3颜色

    static QString ColorTitleBg;        //标题栏背景颜色
    static QString ColorTitleText;      //标题栏文字颜色
    static QString ColorChartBg;        //曲线图表背景颜色
    static QString ColorChartText;      //曲线图表文字颜色
    static QString ColorChartGrid;      //曲线图表网格颜色

    static QString ColorOk;             //正常颜色
    static QString ColorLow;            //警戒颜色
    static QString ColorAlarm;          //报警颜色
    static QString ColorDisable;        //禁用颜色
    static QString ColorPercent;        //百分比颜色

    static QString ColorStatusBar;      //状态栏颜色

    //字体和尺寸配置参数
    static int MainFont;                //全局字号
    static int NameFont;                //软件名称字号
    static int LabFont;                 //加粗标签字号
    static int DeviceFont;              //设备面板字号
    static int SubTitleFont;            //子标题栏字号
    static int TitleFont;               //标题栏字号
    static int TitleHeight;             //标题栏高度
    static int HeadHeight;              //表头高度
    static int RowHeight;               //行高度
    static int LayoutSpacing;           //布局间距
    static int SwitchBtnWidth;          //开关宽度
    static int SwitchBtnHeight;         //开关高度
    static int ScrollWidth;             //滚动条宽度

    //采集速度配置参数,单位毫秒
    static int IntervalModule1;         //模块1采集间隔
    static int IntervalModule2;         //模块2采集间隔
    static int IntervalModule3;         //模块3采集间隔
    static int IntervalModule4;         //模块4采集间隔
    static int IntervalModule5;         //模块5采集间隔
    static int IntervalModule6;         //模块6采集间隔
    static int IntervalModule7;         //模块7采集间隔
    static int IntervalModule8;         //模块8采集间隔

    //本地数据库配置
    static QString LocalDBType;         //本地数据库类型
    static QString LocalDBIP;           //本地数据库主机
    static int LocalDBPort;             //本地数据库端口
    static QString LocalDBName;         //本地数据库名称
    static QString LocalUserName;       //本地数据库用户名
    static QString LocalUserPwd;        //本地数据库密码

    static void readConfig();           //读取配置文件,在main函数最开始加载程序载入
    static void writeConfig();          //写入配置文件,在更改配置文件程序关闭时调用
    static void newConfig();            //以初始值新建配置文件
    static bool checkConfig();          //校验配置文件


    //2023年5月29日11:11:06

    //板卡编号
    static int lvds_card_num;
    static int rs422_card_num;
    static int can_card_num;

    //波特率
    static int high_lvds_baud_rate;
    static int low_lvds_baud_rate;
    static int rs422_baud_rate;
    static int can_baud_rate;

    //通道编号
    static int lvds_channel_num;
    static int rs422_channel_num;
    static int can_channel_num;

    //LVDS运行模式
    static bool lvds_run_high_mode;

    //命令配置目录
    static QString cmdConfigDirPath;
    //遥测配置目录
    static QString tmConfigDirPath;
    //原始数据目录
    static QString saveRawPath;
    //所有数据目录
    static QString saveDataPath;
    static QString saveImageDataPath;
    static QString saveTMDataPath;
    static QString saveLVDSDataPath;
    //遥测数据
    static QString saveRequestPath;

    //是否保存发送原始数据
    static bool isSaveSendRawData;
    //是否保存接收原始数据
    static bool isSaveRecvRawData;

    //广播基准时间
    static uint baseTime;

    //广播开始时间
    static uint startTime;

    //是否开启监听是否是低速数据
    static bool isLowSpeedMode;

    //FPGA升级每包数据的时间间隔
    static int fpgaBinSendSpan;
};

#endif // APP_H
