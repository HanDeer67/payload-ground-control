#ifndef COMMANDCAMERA_H
#define COMMANDCAMERA_H

#include <QObject>

class commandcamera : public QObject
{
    Q_OBJECT
public:
    explicit commandcamera(QObject *parent = nullptr);

public:

    void readXML();
    void reloadXML(QString xmlName);

signals:

public slots:
};

#endif // COMMANDCAMERA_H
