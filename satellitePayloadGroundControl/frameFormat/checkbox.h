#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <QCheckBox>

class CheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit CheckBox(QWidget *parent = nullptr);
    explicit CheckBox(QString text,QWidget *parent = nullptr);
    int row = -1;
    int col = -1;
signals:

public slots:
};

#endif // CHECKBOX_H
