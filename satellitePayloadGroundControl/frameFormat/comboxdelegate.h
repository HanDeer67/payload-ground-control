#ifndef COMBOXDELEGATE_H
#define COMBOXDELEGATE_H

#include <QStyledItemDelegate>

class ComBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ComBoxDelegate(QObject *parent = nullptr);
protected:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
signals:

public slots:
private:
    QStringList items;
};

#endif // COMBOXDELEGATE_H
