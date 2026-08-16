#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "shellcoreclient.h"

// 输出列表模型：供 QML 显示与设置页消费。
class OutputListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        WidthRole,
        HeightRole,
        ScaleRole,
        TransformRole,
        ConnectedRole,
    };

    explicit OutputListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setOutputs(const QVector<ShellCoreClient::Output> &outputs);

private:
    QVector<ShellCoreClient::Output> m_outputs;
};
