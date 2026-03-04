#ifndef PARASETELEMETRYXML_H
#define PARASETELEMETRYXML_H

#include <QObject>
#include <QTableView>
#include <QTableWidget>
#include <QDomElement>
//该类为解析遥测数据的XML文件类并传入一个QTableWidget 或者QTabelView进行初始化
//该类为解析器

class QStandardItemModel;
class ParaseTelemetryXml : public QObject
{
    typedef struct S_TM{
       QString TMItemName;          //参数序号
       QString TMItemSpecification; //参数描述
       short TmItemByteLength;    //参数占有字节数
       QString TmItemType;          //类型
       QString TmItemHexCode;       //源码
       QString TmItemFun;           //计算函数 例如x*360.0/65536-180.0
       QMap<QString,QString> listMap;   //对于多种类型展示
    }TM;
    Q_OBJECT
public:
    static ParaseTelemetryXml *Instance();
    void paraseXml(QString xmlName);
    void refreashXml(QString xmlName);
    void initTable(QTableWidget * widget);
    void initTable(QTableView * widget,QTableView *widget2);
    //解析接收到的Qbytearray数据
    void paraseData(QByteArray data);
    void saveData();
    QString saveFileName;
    //是否初始化
    bool isParaserInit = false;
    int allByteLength = 0;
    QString xmlType;
    QString systemName;
    static ParaseTelemetryXml *self;
    explicit ParaseTelemetryXml(QObject *parent = nullptr);
    void readAttribute(QDomElement element, QString attNodeName);
    QString readAttributeWithString(QDomElement element, QString attNodeName);
signals:
    //发送接收到遥测数据的信号
    void recvTMData();
    void recvTMDataCrCCheck(bool isRight);
public slots:
public slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());

private:
    QList<TM> itemList;
    QColor selectColor = QColor("#20B2AA");
    QColor unSelectColor = QColor("#FF8000");
    QStandardItemModel* model = nullptr;
};

#endif // PARASETELEMETRYXML_H
