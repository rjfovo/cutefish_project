#include "outputlistmodel.h"

OutputListModel::OutputListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int OutputListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_outputs.size();
}

QVariant OutputListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_outputs.size())
        return {};
    const ShellCoreClient::Output &output = m_outputs.at(index.row());
    switch (role) {
    case NameRole: return output.name;
    case WidthRole: return output.width;
    case HeightRole: return output.height;
    case ScaleRole: return output.scale;
    case TransformRole: return output.transform;
    case ConnectedRole: return output.connected;
    default: return {};
    }
}

QHash<int, QByteArray> OutputListModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {WidthRole, "width"},
        {HeightRole, "height"},
        {ScaleRole, "scale"},
        {TransformRole, "transform"},
        {ConnectedRole, "connected"},
    };
}

void OutputListModel::setOutputs(const QVector<ShellCoreClient::Output> &outputs)
{
    beginResetModel();
    m_outputs = outputs;
    endResetModel();
}
