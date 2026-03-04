#ifndef SAVEDATAHELPER_H
#define SAVEDATAHELPER_H

#include <QObject>
#include <QFile>
class SaveDataHelper : public QObject
{
    Q_OBJECT
public:
    explicit SaveDataHelper(QObject *parent = nullptr);
    //保存遥测原始数据
    void saveTMRawData(const QByteArray &data);
    //保存遥测文本数据
    void saveTMData(const QByteArray &data);
    //保存RS422所有原始数据
    void saveRs422RawData(const QByteArray &data);
    //保存低速LVDS数据
    void saveLowLVDSRawData(const QByteArray &data);
    //保存低速LVDS数据到CSV中
    void saveLowLVDSData(const QString &data);
    //保存所有LVDS数据
    void saveLVDSRawData(const QByteArray &data);

    void refreshLowLVDSFileName();

    void refreshLVDSRawFileName();
    QString lvdsLowFileName;
    QString lvdsRawFileName;
    QString lvdsLowCSVFileName;
    //头部信息
    QString lvdsLowCSVFileHead;


    //保存原始数据
    void saveSendRawData(QByteArray saveData);
    void reOpenFile();
    void close();
    //保存原始数据的名称
    QString sendRawDataFileName;
    QFile rawFile;

    bool isClosing = false;
public:
    static SaveDataHelper *Instance();
private:
    static SaveDataHelper *self;


signals:

public slots:
};

#endif // SAVEDATAHELPER_H
