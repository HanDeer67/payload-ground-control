#include "app.h"

QString App::ConfigFile = "config.ini";
QString App::LayoutPath = "layout";

QString App::WorkMode = "timer";
QString App::MapStyle = "point";
QString App::Ratio = "4096*2160";
QString App::Title = QString::fromUtf8("钟组运行管理系统");
QString App::Layout = QString::fromUtf8("完整布局");
QString App::Theme = QString::fromUtf8("深蓝风格");
QString App::VideoAddr = "http://hls01open.ys7.com/openlive/f01018a141094b7fa138b9d0b856507b.hd.m3u8";
QString App::HttpUrl = "http://127.0.0.1:6000";

bool App::AutoRun = false;
bool App::MoveEnable = true;
bool App::CutLeftBottom = true;
bool App::CutRightBottom = true;
bool App::StaticLine = true;
bool App::ShowPercent = true;
int App::StepY = 6;
int App::CursorHideTime = 5;

QString App::ColorMainBg = QColor(4, 7, 38).name();
QString App::ColorPanelBg = QColor(26, 29, 60).name();
QString App::ColorLine = QColor(255, 0, 0).name();
QString App::ColorLine1 = QColor(0, 176, 180).name();
QString App::ColorLine2 = QColor(32, 159, 223).name();
QString App::ColorLine3 = QColor(255, 192, 0).name();

QString App::ColorTitleBg = QColor(48, 48, 85).name();
QString App::ColorTitleText = QColor(255, 255, 255).name();
QString App::ColorChartBg = QColor(38, 41, 74).name();
QString App::ColorChartText = QColor(250, 250, 250).name();
QString App::ColorChartGrid = QColor(180, 180, 180).name();

QString App::ColorOk = QColor(0, 176, 180).name();
QString App::ColorLow = QColor(255, 192, 0).name();
QString App::ColorAlarm = QColor(214, 77, 84).name();
QString App::ColorDisable = QColor(210, 210, 210).name();
QString App::ColorPercent = QColor(0, 254, 254).name();

QString App::ColorStatusBar = QColor("#67DDFB").name();

int App::MainFont = 12;
int App::NameFont = 19;
int App::LabFont = 12;
int App::DeviceFont = 12;
int App::SubTitleFont = 13;
int App::TitleFont = 15;
int App::TitleHeight = 23;
int App::HeadHeight = 28;
int App::RowHeight = 25;
int App::LayoutSpacing = 6;
int App::SwitchBtnWidth = 65;
int App::SwitchBtnHeight = 26;
int App::ScrollWidth = 5;

int App::IntervalModule1 = 5000;
int App::IntervalModule2 = 5100;
int App::IntervalModule3 = 5200;
int App::IntervalModule4 = 5300;
int App::IntervalModule5 = 5400;
int App::IntervalModule6 = 5500;
int App::IntervalModule7 = 5600;
int App::IntervalModule8 = 5700;

QString App::LocalDBType = "MySql";
QString App::LocalDBIP = "127.0.0.1";
int App::LocalDBPort = 3306;
QString App::LocalDBName = "test";
QString App::LocalUserName = "root";
QString App::LocalUserPwd = "root";


//板卡编号
 int App::lvds_card_num = 0;
 int App::rs422_card_num = 1;
 int App::can_card_num = 2;

//波特率
 int App::high_lvds_baud_rate = 48000000;
 int App::low_lvds_baud_rate = 8000000;
 int App::rs422_baud_rate = 115200;
 int App::can_baud_rate = 115200;

//通道编号
 int App::lvds_channel_num = 2;
 int App::rs422_channel_num = 0;
 int App::can_channel_num = 1;

 //默认是高速模式
 bool App::lvds_run_high_mode = true;

 //命令配置目录
 QString App::cmdConfigDirPath = "Config/CMD";
 //遥测配置目录
 QString App::tmConfigDirPath = "Config/TM";

 QString App::saveRawPath = "rawData";
 QString App::saveDataPath = "data";
 QString App::saveImageDataPath;
 QString App::saveTMDataPath;
 QString App::saveLVDSDataPath;
 QString App::saveRequestPath = "";

 bool App::isSaveSendRawData = false;
 bool App::isSaveRecvRawData = false;


 uint App::baseTime = 0;
 uint App::startTime = 0;

 bool App::isLowSpeedMode = false;

 //默认200
 int App::fpgaBinSendSpan = 200;

void App::readConfig()
{
    if (!checkConfig()) {
        return;
    }

    QSettings set(App::ConfigFile, QSettings::IniFormat);

    set.beginGroup("BaseConfig");
    App::WorkMode = set.value("WorkMode", App::WorkMode).toString();
    App::MapStyle = set.value("MapStyle", App::MapStyle).toString();
    App::Ratio = set.value("Ratio", App::Ratio).toString();
    App::Title = set.value("Title", App::Title).toString();
    App::Layout = set.value("Layout", App::Layout).toString();
    App::Theme = set.value("Theme", App::Theme).toString();
    App::VideoAddr = set.value("VideoAddr", App::VideoAddr).toString();
    App::HttpUrl = set.value("HttpUrl", App::HttpUrl).toString();

    App::AutoRun = set.value("AutoRun", App::AutoRun).toBool();
    App::MoveEnable = set.value("MoveEnable", App::MoveEnable).toBool();
    App::CutLeftBottom = set.value("CutLeftBottom", App::CutLeftBottom).toBool();
    App::CutRightBottom = set.value("CutRightBottom", App::CutRightBottom).toBool();
    App::StaticLine = set.value("StaticLine", App::StaticLine).toBool();
    App::ShowPercent = set.value("ShowPercent", App::ShowPercent).toBool();
    App::StepY = set.value("StepY", App::StepY).toInt();
    App::CursorHideTime = set.value("CursorHideTime", App::CursorHideTime).toInt();
    set.endGroup();

    set.beginGroup("ColorConfig");
    App::ColorMainBg = set.value("ColorMainBg", App::ColorMainBg).toString();
    App::ColorPanelBg = set.value("ColorPanelBg", App::ColorPanelBg).toString();
    App::ColorLine = set.value("ColorLine", App::ColorLine).toString();
    App::ColorLine1 = set.value("ColorLine1", App::ColorLine1).toString();
    App::ColorLine2 = set.value("ColorLine2", App::ColorLine2).toString();
    App::ColorLine3 = set.value("ColorLine3", App::ColorLine3).toString();

    App::ColorTitleBg = set.value("ColorTitleBg", App::ColorTitleBg).toString();
    App::ColorTitleText = set.value("ColorTitleText", App::ColorTitleText).toString();
    App::ColorChartBg = set.value("ColorChartBg", App::ColorChartBg).toString();
    App::ColorChartText = set.value("ColorChartText", App::ColorChartText).toString();
    App::ColorChartGrid = set.value("ColorChartGrid", App::ColorChartGrid).toString();

    App::ColorOk = set.value("ColorOk", App::ColorOk).toString();
    App::ColorLow = set.value("ColorLow", App::ColorLow).toString();
    App::ColorAlarm = set.value("ColorAlarm", App::ColorAlarm).toString();
    App::ColorDisable = set.value("ColorDisable", App::ColorDisable).toString();
    App::ColorPercent = set.value("ColorPercent", App::ColorPercent).toString();
    set.endGroup();

    set.beginGroup("SizeConfig");
    App::MainFont = set.value("MainFont", App::MainFont).toInt();
    App::NameFont = set.value("NameFont", App::NameFont).toInt();
    App::LabFont = set.value("LabFont", App::LabFont).toInt();
    App::DeviceFont = set.value("DeviceFont", App::DeviceFont).toInt();
    App::SubTitleFont = set.value("SubTitleFont", App::SubTitleFont).toInt();
    App::TitleFont = set.value("TitleFont", App::TitleFont).toInt();
    App::TitleHeight = set.value("TitleHeight", App::TitleHeight).toInt();
    App::HeadHeight = set.value("HeadHeight", App::HeadHeight).toInt();
    App::RowHeight = set.value("RowHeight", App::RowHeight).toInt();
    App::LayoutSpacing = set.value("LayoutSpacing", App::LayoutSpacing).toInt();
    set.endGroup();

    set.beginGroup("IntervalConfig");
    App::IntervalModule1 = set.value("IntervalModule1", App::IntervalModule1).toInt();
    App::IntervalModule2 = set.value("IntervalModule2", App::IntervalModule2).toInt();
    App::IntervalModule3 = set.value("IntervalModule3", App::IntervalModule3).toInt();
    App::IntervalModule4 = set.value("IntervalModule4", App::IntervalModule4).toInt();
    App::IntervalModule5 = set.value("IntervalModule5", App::IntervalModule5).toInt();
    App::IntervalModule6 = set.value("IntervalModule6", App::IntervalModule6).toInt();
    App::IntervalModule7 = set.value("IntervalModule7", App::IntervalModule7).toInt();
    App::IntervalModule8 = set.value("IntervalModule8", App::IntervalModule8).toInt();
    set.endGroup();

    set.beginGroup("LocalDBConfig");
    App::LocalDBType = set.value("LocalDBType").toString();
    App::LocalDBIP = set.value("LocalDBIP").toString();
    App::LocalDBPort = set.value("LocalDBPort").toInt();
    App::LocalDBName = set.value("LocalDBName").toString();
    App::LocalUserName = set.value("LocalUserName").toString();
    App::LocalUserPwd = set.value("LocalUserPwd").toString();
    set.endGroup();




    set.beginGroup("CardConfig");
    App::lvds_card_num = set.value("lvds_card_num", App::lvds_card_num).toInt();
    App::rs422_card_num = set.value("rs422_card_num", App::rs422_card_num).toInt();
    App::can_card_num = set.value("can_card_num", App::can_card_num).toInt();

    App::high_lvds_baud_rate = set.value("high_lvds_baud_rate", App::high_lvds_baud_rate).toInt();
    App::low_lvds_baud_rate = set.value("low_lvds_baud_rate", App::low_lvds_baud_rate).toInt();
    App::rs422_baud_rate = set.value("rs422_baud_rate", App::rs422_baud_rate).toInt();
    App::can_baud_rate = set.value("can_baud_rate", App::can_baud_rate).toInt();

    App::lvds_channel_num = set.value("lvds_channel_num", App::lvds_channel_num).toInt();
    App::rs422_channel_num = set.value("rs422_channel_num", App::rs422_channel_num).toInt();
    App::can_channel_num = set.value("can_channel_num", App::can_channel_num).toInt();

    App::lvds_run_high_mode = set.value("lvds_run_high_mode", App::lvds_run_high_mode).toBool();

    App::isSaveSendRawData = set.value("isSaveSendRawData", App::isSaveSendRawData).toBool();
    App::isSaveRecvRawData = set.value("isSaveRecvRawData", App::isSaveRecvRawData).toBool();
    App::baseTime = set.value("baseTime", App::baseTime).toUInt();
    App::startTime = set.value("startTime", App::startTime).toUInt();
    App::fpgaBinSendSpan = set.value("fpgaBinSendSpan", App::fpgaBinSendSpan).toInt();



    set.endGroup();

    App::CursorHideTime = App::CursorHideTime < 3 ? 3 : App::CursorHideTime;
}

void App::writeConfig()
{
    QSettings set(App::ConfigFile, QSettings::IniFormat);

    set.beginGroup("BaseConfig");
    set.setValue("WorkMode", App::WorkMode);
    set.setValue("MapStyle", App::MapStyle);
    set.setValue("Ratio", App::Ratio);
    set.setValue("Title", App::Title);
    set.setValue("Layout", App::Layout);
    set.setValue("Theme", App::Theme);
    set.setValue("VideoAddr", App::VideoAddr);
    set.setValue("HttpUrl", App::HttpUrl);

    set.setValue("AutoRun", App::AutoRun);
    set.setValue("MoveEnable", App::MoveEnable);
    set.setValue("CutLeftBottom", App::CutLeftBottom);
    set.setValue("CutRightBottom", App::CutRightBottom);
    set.setValue("StaticLine", App::StaticLine);
    set.setValue("ShowPercent", App::ShowPercent);
    set.setValue("StepY", App::StepY);
    set.setValue("CursorHideTime", App::CursorHideTime);

    set.endGroup();

    set.beginGroup("ColorConfig");
    set.setValue("ColorMainBg", App::ColorMainBg);
    set.setValue("ColorPanelBg", App::ColorPanelBg);
    set.setValue("ColorLine", App::ColorLine);
    set.setValue("ColorLine1", App::ColorLine1);
    set.setValue("ColorLine2", App::ColorLine2);
    set.setValue("ColorLine3", App::ColorLine3);

    set.setValue("ColorTitleBg", App::ColorTitleBg);
    set.setValue("ColorTitleText", App::ColorTitleText);
    set.setValue("ColorChartBg", App::ColorChartBg);
    set.setValue("ColorChartText", App::ColorChartText);
    set.setValue("ColorChartGrid", App::ColorChartGrid);

    set.setValue("ColorOk", App::ColorOk);
    set.setValue("ColorLow", App::ColorLow);
    set.setValue("ColorAlarm", App::ColorAlarm);
    set.setValue("ColorDisable", App::ColorDisable);
    set.setValue("ColorPercent", App::ColorPercent);
    set.endGroup();

    set.beginGroup("SizeConfig");
    set.setValue("MainFont", App::MainFont);
    set.setValue("NameFont", App::NameFont);
    set.setValue("LabFont", App::LabFont);
    set.setValue("DeviceFont", App::DeviceFont);
    set.setValue("SubTitleFont", App::SubTitleFont);
    set.setValue("TitleFont", App::TitleFont);
    set.setValue("TitleHeight", App::TitleHeight);
    set.setValue("HeadHeight", App::HeadHeight);
    set.setValue("RowHeight", App::RowHeight);
    set.setValue("LayoutSpacing", App::LayoutSpacing);
    set.endGroup();

    set.beginGroup("IntervalConfig");
    set.setValue("IntervalModule1", App::IntervalModule1);
    set.setValue("IntervalModule2", App::IntervalModule2);
    set.setValue("IntervalModule3", App::IntervalModule3);
    set.setValue("IntervalModule4", App::IntervalModule4);
    set.setValue("IntervalModule5", App::IntervalModule5);
    set.setValue("IntervalModule6", App::IntervalModule6);
    set.setValue("IntervalModule7", App::IntervalModule7);
    set.setValue("IntervalModule8", App::IntervalModule8);
    set.endGroup();

    set.beginGroup("LocalDBConfig");
    set.setValue("LocalDBType", App::LocalDBType);
    set.setValue("LocalDBIP", App::LocalDBIP);
    set.setValue("LocalDBPort", App::LocalDBPort);
    set.setValue("LocalDBName", App::LocalDBName);
    set.setValue("LocalUserName", App::LocalUserName);
    set.setValue("LocalUserPwd", App::LocalUserPwd);
    set.endGroup();


    set.beginGroup("CardConfig");
    set.setValue("lvds_card_num", App::lvds_card_num);
    set.setValue("rs422_card_num", App::rs422_card_num);
    set.setValue("can_card_num", App::can_card_num);

    set.setValue("high_lvds_baud_rate", App::high_lvds_baud_rate);
    set.setValue("low_lvds_baud_rate", App::low_lvds_baud_rate);
    set.setValue("rs422_baud_rate", App::rs422_baud_rate);
    set.setValue("can_baud_rate", App::can_baud_rate);

    set.setValue("lvds_channel_num", App::lvds_channel_num);
    set.setValue("rs422_channel_num", App::rs422_channel_num);
    set.setValue("can_channel_num", App::can_channel_num);

    set.setValue("lvds_run_high_mode", App::lvds_run_high_mode);
    set.setValue("isSaveSendRawData", App::isSaveSendRawData);
    set.setValue("isSaveRecvRawData", App::isSaveRecvRawData);


    set.setValue("baseTime", App::baseTime);
    set.setValue("startTime", App::startTime);
    set.setValue("fpgaBinSendSpan", App::fpgaBinSendSpan);
    set.endGroup();
}

void App::newConfig()
{
#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
#endif
    writeConfig();
}

bool App::checkConfig()
{
    //如果配置文件大小为0,则以初始值继续运行,并生成配置文件
    QFile file(App::ConfigFile);
    if (file.size() == 0) {
        newConfig();
        return false;
    }

    //如果配置文件不完整,则以初始值继续运行,并生成配置文件
    if (file.open(QFile::ReadOnly)) {
        bool ok = true;
        while (!file.atEnd()) {
            QString line = file.readLine();
            line = line.replace("\r", "");
            line = line.replace("\n", "");
            QStringList list = line.split("=");

            if (list.count() == 2) {
                if (list.at(1) == "") {
                    ok = false;
                    break;
                }
            }
        }

        if (!ok) {
            newConfig();
            return false;
        }
    } else {
        newConfig();
        return false;
    }

    return true;
}
