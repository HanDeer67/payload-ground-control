#include "apptools.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QDir>

#include <QMutex>
QScopedPointer<AppTools> AppTools::self;
AppTools *AppTools::Instance()
{
    if (self.isNull()) {
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        if (self.isNull()) {
            self.reset(new AppTools);
        }
    }

    return self.data();
}

AppTools::AppTools(QObject *parent) : QObject(parent)
{

}

int AppTools::deskWidth()
{
    //没有必要每次都获取,只有当变量为空时才去获取一次
    static int width = 0;
    if (width == 0) {
        width = qApp->desktop()->availableGeometry().width();
    }

    return width;

    //QGuiApplication::primaryScreen()->geometry().size().width();
}

int AppTools::deskHeight()
{
    //没有必要每次都获取,只有当变量为空时才去获取一次
    static int height = 0;
    if (height == 0) {
        height = qApp->desktop()->availableGeometry().height();
    }

    return height;
    //QGuiApplication::primaryScreen()->geometry().size().height();
}

int AppTools::screenNums()
{
    QList<QScreen*> screens = QGuiApplication::screens();
    return screens.count();
}

QString AppTools::appName()
{
    //没有必要每次都获取,只有当变量为空时才去获取一次
    static QString name;
    if (name.isEmpty()) {
        name = qApp->applicationFilePath();
        //下面的方法主要为了过滤安卓的路径 lib程序名_armeabi-v7a
        QStringList list = name.split("/");
        name = list.at(list.count() - 1).split(".").at(0);
    }

    return name;
}

QString AppTools::appPath()
{
#ifdef Q_OS_ANDROID
    //return QString("/sdcard/Android/%1").arg(appName());
    return QString("/storage/emulated/0/%1").arg(appName());
#else
    return qApp->applicationDirPath();
#endif
}

void AppTools::setFormInCenter(QWidget *frm)
{
    int frmX = frm->width();
    int frmY = frm->height();
    QDesktopWidget w;
    int deskWidth = w.availableGeometry().width();
    int deskHeight = w.availableGeometry().height();
    QPoint movePoint(deskWidth / 2 - frmX / 2, deskHeight / 2 - frmY / 2);
    frm->move(movePoint);
}

void AppTools::newDir(const QString &dirName)
{
    QString strDir = dirName;

    //如果路径中包含斜杠字符则说明是绝对路径
    //linux系统路径字符带有 /  windows系统 路径字符带有 :/
    if (!strDir.startsWith("/") && !strDir.contains(":/")) {
        strDir = QString("%1/%2").arg(appPath()).arg(strDir);
    }

    QDir dir(strDir);
    if (!dir.exists()) {
        dir.mkpath(strDir);
    }
}
