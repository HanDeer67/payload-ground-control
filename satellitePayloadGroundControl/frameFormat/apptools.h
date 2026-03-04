#ifndef APPTOOLS_H
#define APPTOOLS_H

#include <QObject>

class AppTools : public QObject
{
    Q_OBJECT
public:
    static AppTools *Instance();
private:
    static QScopedPointer<AppTools> self;
    explicit AppTools(QObject *parent = nullptr);

public:
    //桌面宽度高度
    static int deskWidth();
    static int deskHeight();
    static int screenNums();

    //程序文件名称+当前所在路径
    static QString appName();
    static QString appPath();

    //设置窗体居中显示
    static void setFormInCenter(QWidget *frm);

    //新建目录
    static void newDir(const QString &dirName);
signals:

public slots:
};

#endif // APPTOOLS_H
