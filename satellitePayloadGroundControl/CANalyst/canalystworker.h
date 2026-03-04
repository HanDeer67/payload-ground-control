#ifndef CANALYSTWORKER_H
#define CANALYSTWORKER_H

#include <QObject>

class CANalystWorker : public QObject
{
    Q_OBJECT
public:
    explicit CANalystWorker(QObject *parent = nullptr);

signals:

public slots:
};

#endif // CANALYSTWORKER_H
