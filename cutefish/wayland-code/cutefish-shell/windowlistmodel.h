#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "shellcoreclient.h"

// 窗口列表模型：Dock/任务栏/Alt-Tab 共用数据源。
class WindowListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        AppIdRole,
        TitleRole,
        StateRole,
        ActivatedRole,
    };

    explicit WindowListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setWindows(const QVector<ShellCoreClient::WindowInfo> &windows);

private:
    QVector<ShellCoreClient::WindowInfo> m_windows;
};
