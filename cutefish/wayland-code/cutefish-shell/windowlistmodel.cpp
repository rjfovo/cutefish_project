#include "windowlistmodel.h"

WindowListModel::WindowListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int WindowListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_windows.size();
}

QVariant WindowListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_windows.size())
        return {};
    const ShellCoreClient::WindowInfo &window = m_windows.at(index.row());
    switch (role) {
    case IdRole: return window.id;
    case AppIdRole: return window.appId;
    case TitleRole: return window.title;
    case StateRole: return window.state;
    case ActivatedRole: return window.activated;
    default: return {};
    }
}

QHash<int, QByteArray> WindowListModel::roleNames() const
{
    return {
        {IdRole, "windowId"},
        {AppIdRole, "appId"},
        {TitleRole, "title"},
        {StateRole, "state"},
        {ActivatedRole, "activated"},
    };
}

void WindowListModel::setWindows(const QVector<ShellCoreClient::WindowInfo> &windows)
{
    beginResetModel();
    m_windows = windows;
    endResetModel();
}
