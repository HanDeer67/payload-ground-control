#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QDomDocument>
#include <QTableWidget>
#include <QListWidget>
#include <QTableWidgetItem>
#include <QListWidgetItem>

#include "head.h"

#include "delegate.h"

class Command : public QObject
{
    Q_OBJECT

public:
    static Command *Instance();
    explicit Command(QObject *parent = nullptr);
    //定义两个转xml的函数
    void writeXML(QString xmlName,QVector<CommandInfo> cmdList);
    QVector<CommandInfo> readXML(QString xmlName);
    void initXML(QString xmlName);
    //修改命令
    void changeCommand(int index);

    void sendCMD();
    void sendCMD(CommandInfo cmdInfo);
    void realSendCMD(QByteArray cmd);
    QString getCMDString(CommandInfo cmdInfo);
    QByteArray getCMDByteArray(CommandInfo cmdInfo);
    void sendCMDList(QVector<CommandInfo> cmdInfoList);
    int commandIndex = -1;
    //初始化
    void registerTabelWidget(QTableWidget* tableWidget,QListWidget* listWidget);
private:
    static Command *self;
    QTableWidget *tableWidget = nullptr;
    QListWidget *listWidget = nullptr;
    Delegate* delegate = nullptr;
private:
    void readAttribute(QDomElement element,QString attNodeName);
    QString readAttributeWithString(QDomElement element,QString attNodeName);
        void initTable();
public:

    //解析SYSTEM部分
    QString systemName;
    //解析XML类型说明
    QString xmlType;
    //参数名称列表
    QVector<CommandInfo> CommandList;

signals:

public slots:
};

#endif // COMMAND_H
