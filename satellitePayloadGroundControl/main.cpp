#include "mainwindow.h"
#include <QFile>
#include <QDir>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<CentroidFrameAttributes>("CentroidFrameAttributes");
    qRegisterMetaType<TemperatureFrameAttributes>("TemperatureFrameAttributes");
    qRegisterMetaType<ImageAttributes>("ImageAttributes"); // 注册类型
    qRegisterMetaType<SettingsUiToSub>("SettingsUiToSub");

    qRegisterMetaType<CommandAttribute>("CommandAttribute");
    qRegisterMetaType<Command>("Command");
    qRegisterMetaType<CommandInfo>("CommandInfo");
    qRegisterMetaType<CommandQueue>("CommandQueue");
    qRegisterMetaType<TMitemQueue>("TMitemQueue");
    qRegisterMetaType<TMparaQueue>("TMparaQueue");

    qRegisterMetaType<ParaListUi>("ParaListUi");
    // 注册 QVector 类型 -
    qRegisterMetaType<QVector<ParaListUi>>("QVector<ParaListUi>"); // 告诉QT元系统如何序列化和反序列化这种自定义类型才能在信号槽队列连接中使用。

    qRegisterMetaType<FrameTM>("FrameTM");
    qRegisterMetaType<canFrameConfig>("canFrameConfig");


//    QString qssFilePath = ":/QssFiles/mainQss.qss";
//    qDebug() << "QSS File Path:" << qssFilePath;
//    QApplication::setStyle("Fusion"); // 启用更接近win11的主题
//    QFile qssFile(qssFilePath);
//    if (qssFile.open(QFile::ReadOnly)) { // 只需要打开一次
//        qApp->setStyleSheet(qssFile.readAll());
//        qssFile.close();
//    } else {
//        qDebug() << "Failed to open QSS file:" << qssFilePath;
//    }

    MainWindow w;
    w.init();
    w.show();

    QString qssFilePath = ":/QssFiles/mainQss.qss";
    qDebug() << "QSS File Path:" << qssFilePath;
    QApplication::setStyle("Fusion"); // 启用更接近win11的主题
    QFile qssFile(qssFilePath);
    if (qssFile.open(QFile::ReadOnly)) { // 只需要打开一次
        qApp->setStyleSheet(qssFile.readAll());
        qssFile.close();
    } else {
        qDebug() << "Failed to open QSS file:" << qssFilePath;
    }

    return a.exec();
}

