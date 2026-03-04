#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>

class ComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit ComboBox(QWidget *parent = nullptr);
    int row = -1;
    int col = -1;
signals:

public slots:
};

#endif // COMBOBOX_H
