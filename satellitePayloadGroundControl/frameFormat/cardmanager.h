#ifndef CARDMANAGER_H
#define CARDMANAGER_H

#include <QObject>
#include "lvdscardhelper.h"
class CardManager : public QObject
{
    Q_OBJECT
public:
    explicit CardManager(QObject *parent = nullptr);
public:
    static CardManager *Instance();
    void initManager();
    void setLVDSRunMode(bool isHighMode);
    //打开板卡
    void openCard();
    void closeCard();
    void startCard();
    void stopCard();

    //发送数据
    void sendLVDSData(QByteArray data);
    void sendRS422Data(QByteArray data);
    void sendRSCANData(QByteArray data);
private:
    static CardManager *self;
    LVDSCardHelper* lvdsCard = nullptr;
    LVDSCardHelper* rs422Card = nullptr;
    LVDSCardHelper* canCard = nullptr;
    QList<LVDSCardHelper*> cardList;

signals:
    void onLVDSDataReady(quint8 channel,QByteArray data);
    void onRS422DataReady(quint8 channel,QByteArray data);
    void onCanDataReady(quint8 channel,QByteArray data);
    void errorMsg(LVDSCardHelper::CardType type,QString msg);
public slots:
    void RunErrorMsg(QString msg);
};

#endif // CARDMANAGER_H
