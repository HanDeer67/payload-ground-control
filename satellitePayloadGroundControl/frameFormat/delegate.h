#ifndef DELEGATE_H
#define DELEGATE_H

#include <QStyledItemDelegate>
#include <QItemDelegate>
#include <QModelIndex>
#include <QPainter>
#include <QWidget>
#include "head.h"

class Delegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit Delegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                    const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const;
    void destroyEditor(QWidget *editor, const QModelIndex &index) const;
    void setCommand(CommandInfo cmd);
signals:

public slots:
private:
    //DelegateInfo delegateInfo;
    //添加一个Command
    CommandInfo cmdInfo;
};

#endif // DELEGATE_H
