#include "cardmanager.h"
#include "app.h"

#include <QMutex>
CardManager *CardManager::self = nullptr;
CardManager *CardManager::Instance()
{
    if (!self) {
        QMutex mutex;
        QMutexLocker locker(&mutex);
        if (!self) {
            self = new CardManager;
        }
    }

    return self;
}


CardManager::CardManager(QObject *parent) : QObject(parent)
{
//    this->initManager();
}
//void CardManager::initManager()
//{
//    for (int i = 0; i < cardList.length();i++) {

//        LVDSCardHelper * card = cardList.at(i);
//        if(card->isRunning())
//        {
//            card->stopLVDS();
//            card->uninitCard();
//        }
//        card->deleteLater();
//    }
//    cardList.clear();

//    //第一步 初始化
//    lvdsCard = new LVDSCardHelper;
//    cardList << lvdsCard;
//    //设置板卡编号
//    lvdsCard->setCurrentCardNo(static_cast<quint8>(App::lvds_card_num));
//    qDebug()<<"LVDS channel num:"<<App::lvds_card_num;
//    //设置板卡类型
//    lvdsCard->setCardType(LVDSCardHelper::LVDS_CARD);
//    //设置通道和波特率
//    QList<QPair<quint8,quint32>> channelInfoList;

////    if(App::lvds_run_high_mode)
////        channelInfoList.append(QPair<quint8,int>(App::lvds_channel_num,App::high_lvds_baud_rate));
////    else
////        channelInfoList.append(QPair<quint8,int>(App::lvds_channel_num,App::low_lvds_baud_rate));

//    channelInfoList.append(QPair<quint8,quint32>(App::lvds_channel_num,App::low_lvds_baud_rate));
//    lvdsCard->setChannelAndBaudRate(channelInfoList);
//    lvdsCard->setCurrentUseChannel(App::lvds_channel_num);
//    lvdsCard->setEnable(true);
//    //建立数据连接
//    connect(lvdsCard,SIGNAL(onDataReady(quint8 ,QByteArray )),this,SIGNAL(onLVDSDataReady(quint8,QByteArray)));
//    connect(lvdsCard,SIGNAL(RunError(QString)),this,SLOT(RunErrorMsg(QString)));


//    rs422Card = new LVDSCardHelper;
//    cardList << rs422Card;
//    rs422Card->setCurrentCardNo(static_cast<quint8>(App::rs422_card_num));
//    rs422Card->setCurrentUseChannel(App::rs422_channel_num);
//    qDebug()<<"RS422 channel num:"<<App::rs422_card_num;
//    rs422Card->setCardType(LVDSCardHelper::RS422_CARD);
//    channelInfoList.clear();
//    channelInfoList.append(QPair<quint8,quint32>(App::rs422_channel_num,App::rs422_baud_rate));
//    rs422Card->setChannelAndBaudRate(channelInfoList);
//    rs422Card->setEnable(true);
//    connect(rs422Card,SIGNAL(onDataReady(quint8,QByteArray)),this,SIGNAL(onRS422DataReady(quint8,QByteArray)));
//    connect(rs422Card,SIGNAL(RunError(QString)),this,SLOT(RunErrorMsg(QString)));

//    canCard = new LVDSCardHelper;
//    cardList << canCard;
//    canCard->setCurrentCardNo(static_cast<quint8>(App::can_card_num));
//    canCard->setCurrentUseChannel(App::can_channel_num);
//    qDebug()<<"CAN channel num:"<<App::can_card_num;
//    canCard->setCardType(LVDSCardHelper::CAN_CARD);
//    channelInfoList.clear();
//    channelInfoList.append(QPair<int,int>(App::can_channel_num,App::can_baud_rate));
//    canCard->setChannelAndBaudRate(channelInfoList);
//    canCard->setEnable(true);
//    connect(canCard,SIGNAL(onDataReady(quint8,QByteArray)),this,SIGNAL(onCanDataReady(quint8,QByteArray)));
//    connect(canCard,SIGNAL(RunError(QString)),this,SLOT(RunErrorMsg(QString)));

//}

//void CardManager::setLVDSRunMode(bool isHighMode)
//{

//}

//void CardManager::openCard()
//{
//    for (int i = 0; i < cardList.length();i++)
//    {

//        LVDSCardHelper * card = cardList.at(i);
//        if(card->isRunning())
//        {
//            card->stopLVDS();
//            card->uninitCard();
//        }
//        card->initCard();
//    }
//}

//void CardManager::closeCard()
//{
//    for (int i = 0; i < cardList.length();i++)
//    {

//        LVDSCardHelper * card = cardList.at(i);
//        if(card->isRunning())
//        {
//            card->stopLVDS();
//        }
//        card->uninitCard();
//    }
//}

//void CardManager::startCard()
//{
//    for (int i = 0; i < cardList.length();i++)
//    {

//        LVDSCardHelper * card = cardList.at(i);
//        if(!card->isRunning())
//        {
//            card->startLVDS();
//        }
//    }
//}

//void CardManager::stopCard()
//{
//    for (int i = 0; i < cardList.length();i++)
//    {
//        LVDSCardHelper * card = cardList.at(i);
//        if(card->isRunning())
//        {
//            card->stopLVDS();
//        }
//    }
//}

//void CardManager::sendLVDSData(QByteArray data)
//{
//    for (int i = 0; i < cardList.length();i++)
//    {
//        LVDSCardHelper * card = cardList.at(i);
//        if(card->getCardType() == LVDSCardHelper::LVDS_CARD)
//        {
//            card->sendData(data);
//            break;
//        }
//    }
//}

//void CardManager::sendRS422Data(QByteArray data)
//{
//    for (int i = 0; i < cardList.length();i++)
//    {
//        LVDSCardHelper * card = cardList.at(i);
//        if(card->getCardType() == LVDSCardHelper::RS422_CARD)
//        {
//            card->sendData(data);
//            break;
//        }
//    }
//}

//void CardManager::sendRSCANData(QByteArray data)
//{
//    for (int i = 0; i < cardList.length();i++)
//    {
//        LVDSCardHelper * card = cardList.at(i);
//        if(card->getCardType() == LVDSCardHelper::CAN_CARD)
//        {
//            card->sendData(data);
//            break;
//        }
//    }
//}

void CardManager::RunErrorMsg(QString msg)
{
    LVDSCardHelper* card = static_cast<LVDSCardHelper*>(sender());
//    emit errorMsg(card->getCardType(),msg);
    qDebug()<<"RunErrorMsg:"<<msg;
}
