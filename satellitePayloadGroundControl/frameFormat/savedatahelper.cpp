#include "savedatahelper.h"
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTextStream>

#include "app.h"

#define FILE_SIZE (10*1024*1024)

#include <QMutex>
SaveDataHelper *SaveDataHelper::self = nullptr;
SaveDataHelper *SaveDataHelper::Instance()
{
    if (!self) {
        QMutex mutex;
        QMutexLocker locker(&mutex);
        if (!self) {
            self = new SaveDataHelper;
        }
    }

    return self;
}
SaveDataHelper::SaveDataHelper(QObject *parent) : QObject(parent)
{
    //创建头部信息
    for (int i =1;i<15;i++)
    {
        lvdsLowCSVFileHead+=QString("粗跟踪指向角%1(方位轴),").arg(i);
        lvdsLowCSVFileHead+=QString("粗跟踪指向角%1(俯仰轴),").arg(i);
        lvdsLowCSVFileHead+=QString("粗跟踪质心数据%1(X方向),").arg(i);
        lvdsLowCSVFileHead+=QString("粗跟踪质心数据%1(Y方向),").arg(i);
        lvdsLowCSVFileHead+=QString("粗跟踪预指向角%1(方位轴),").arg(i);
        lvdsLowCSVFileHead+=QString("粗跟踪预指向角%1(俯仰轴),").arg(i);
        lvdsLowCSVFileHead+=QString("粗跟踪超前瞄准%1(X方向),").arg(i);
        lvdsLowCSVFileHead+=QString("粗跟踪超前瞄准%1(Y方向),").arg(i);
        lvdsLowCSVFileHead+=QString("时间码%1(秒),").arg(i);
        lvdsLowCSVFileHead+=QString("时间码%1(毫秒),").arg(i);

    }

    for (int i =1;i<218;i++)
    {
        lvdsLowCSVFileHead+=QString("精跟踪质心数据%1(X方向),").arg(i);
        lvdsLowCSVFileHead+=QString("精跟踪质心数据%1(Y方向),").arg(i);
        lvdsLowCSVFileHead+=QString("时间码%1(秒),").arg(i);

        if(i == 217)
        {
            lvdsLowCSVFileHead+=QString("时间码%1(毫秒)").arg(i);
        }
        else
        {
            lvdsLowCSVFileHead+=QString("时间码%1(毫秒),").arg(i);
        }
    }

}

void SaveDataHelper::saveTMRawData(const QByteArray &data)
{
    QDateTime dt = QDateTime::currentDateTime();

    QString strDate = dt.currentDateTime().toString("yyyy-MM-dd");

    QString timeStamp = dt.currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    QString fileDir = App::saveRawPath+"/tmRawData/";

    //添加年月日的判断
    QString tempFileDir = QString("%1/%2/%3").arg(fileDir).arg(dt.currentDateTime().toString("yyyy")).arg(dt.currentDateTime().toString("MM"));

    //如果目录不存在则创建
    QDir dir;
    if(!dir.exists(tempFileDir))
    {
        dir.mkpath(tempFileDir);
    }

    QString fileName=QString("%1/%2.txt").arg(tempFileDir).arg(strDate);

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream<<timeStamp+":"+data.toHex(' ')+"\n";
        file.close();
    }
}

void SaveDataHelper::saveTMData(const QByteArray &data)
{

}

void SaveDataHelper::saveRs422RawData(const QByteArray &data)
{
    QDateTime dt = QDateTime::currentDateTime();

    QString strDate = dt.currentDateTime().toString("yyyy-MM-dd");

    QString timeStamp = dt.currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    QString fileDir = App::saveRawPath+"/RS422RawData/";

    //添加年月日的判断
    QString tempFileDir = QString("%1/%2/%3").arg(fileDir).arg(dt.currentDateTime().toString("yyyy")).arg(dt.currentDateTime().toString("MM"));

    //如果目录不存在则创建
    QDir dir;
    if(!dir.exists(tempFileDir))
    {
        dir.mkpath(tempFileDir);
    }

    QString fileName=QString("%1/%2.txt").arg(tempFileDir).arg(strDate);

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream<<timeStamp+":"+data.toHex(' ')+"\n";
        file.close();
    }
}

void SaveDataHelper::saveLowLVDSRawData(const QByteArray &data)
{
    QDateTime dt = QDateTime::currentDateTime();
    if(lvdsLowFileName.isEmpty())
    {
        lvdsLowFileName = dt.currentDateTime().toString("yyyyMMdd_hhmmsszzz");
    }

    QString fileName=QString("%1/%2.bin").arg(App::saveLVDSDataPath).arg(lvdsLowFileName);
    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        //QTextStream stream(&file);
        //stream.setCodec("UTF-8");
        //stream<<data;
        file.write(data);
        file.close();
    }
}

void SaveDataHelper::saveLowLVDSData(const QString &data)
{
    //
    QDateTime dt = QDateTime::currentDateTime();
    if(lvdsLowCSVFileName.isEmpty())
    {
        lvdsLowCSVFileName = dt.currentDateTime().toString("yyyyMMdd_hhmmsszzz");
    }

    QString fileName=QString("%1/%2.csv").arg(App::saveLVDSDataPath).arg(lvdsLowCSVFileName);
    QFile file(fileName);
    //如果不存在则打印
    bool isFistCreat = !file.exists();
    if(file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
        QTextStream stream(&file);
        stream.setCodec("GB2312");
        //如果是第一次创建则需要打印头部信息
        if(isFistCreat)
        {
            stream<<lvdsLowCSVFileHead<<endl;
        }
        stream<<data<<endl;
        file.close();
    }
}

void SaveDataHelper::saveLVDSRawData(const QByteArray &data)
{
    QDateTime dt = QDateTime::currentDateTime();

    QString fileDir = App::saveRawPath+"/LVDSRawData/";
    //如果目录不存在则创建
    QDir dir;
    if(!dir.exists(fileDir))
    {
        dir.mkpath(fileDir);
    }

    if(lvdsRawFileName.isEmpty())
    {
        lvdsRawFileName = dt.currentDateTime().toString("yyyyMMdd_hhmmsszzz");
    }

    QString fileName=QString("%1/%2.bin").arg(fileDir).arg(lvdsRawFileName);
    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        //QTextStream stream(&file);
        //stream.setCodec("UTF-8");
        //stream<<data;
        file.write(data);
        file.close();
    }
}

void SaveDataHelper::refreshLowLVDSFileName()
{
    //低速LVDS的二进制数据以及低速LVDS的CSV数据
    lvdsLowFileName.clear();
    lvdsLowCSVFileName.clear();
}

void SaveDataHelper::refreshLVDSRawFileName()
{
    lvdsRawFileName.clear();
}

void SaveDataHelper::saveSendRawData(QByteArray saveData)
{
    //保存原始数据到 /rawData/sendData/
    //保存原始数据到 /rawData/recvData/

    //软件第一次运行的时候 根据时间创建一个文件并打开
    if(sendRawDataFileName.isEmpty())
    {
        reOpenFile();
    }
    //判断文件大小
    else
    {
        if(rawFile.isOpen())
        {
            if(rawFile.size()>=FILE_SIZE)
            {
                reOpenFile();
            }
        }
    }

    if(rawFile.isOpen())
    {
        QTextStream stream(&rawFile);
        stream.setCodec("UTF-8");
        QString timeStamp = QDateTime::currentDateTime().toString("[yyyy-MM-dd HH:mm:ss]");
        stream<<timeStamp+":"+saveData.toHex(' ').toUpper()+"\n";
    }
    else
    {
        qDebug()<<"文件未打开,不能保存发送数据!"<<rawFile.fileName();
    }
}

void SaveDataHelper::reOpenFile()
{
    //如果正在关闭则不进行写入了
    if(isClosing)return;

    QDateTime dt = QDateTime::currentDateTime();
    QString saveDir = App::saveRawPath+"/sendRawData/";
    //如果目录不存在则创建
    QDir dir;
    if(!dir.exists(saveDir))
    {
        dir.mkpath(saveDir);
    }


    sendRawDataFileName = saveDir+dt.toString("yyyy年MM月dd日_")+QString::number(dt.toTime_t())+".txt";
    if(rawFile.isOpen())
    {
        rawFile.close();
    }
    rawFile.setFileName(sendRawDataFileName);
    if(!rawFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        qDebug()<<"重新打开文件失败!"<<sendRawDataFileName<<rawFile.errorString();
    }
}

void SaveDataHelper::close()
{
    isClosing = true;
    if(rawFile.isOpen())rawFile.close();
}
