#include "EmployeeTableModel.h"
#include "CodeFormat.h"
#include <utility>

EmployeeTableModel::EmployeeTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int EmployeeTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_employees.size();
}

int EmployeeTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(Column::Count);
}

QVariant EmployeeTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() ||
        index.row() < 0 || index.row() >= m_employees.size() ||
        index.column() < 0 || index.column() >= static_cast<int>(Column::Count))
    {
        return {};
    }


    const EmployeeInfo2& employee = m_employees.at(index.row());
    switch (role)
    {
        case Qt::DisplayRole:
        case Qt::EditRole:
        {
            switch (static_cast<Column>(index.column()))
            {
                case Column::Sequence: return index.row() + 1;
                case Column::Code: return employee.code;
                case Column::Name: return employee.name;
                case Column::Department: return employee.department;
                default: break;
            }
            break;
        }
        case Qt::TextAlignmentRole:
        {
            return int(Qt::AlignLeft | Qt::AlignVCenter);
        }
        case Qt::ToolTipRole:
        {
            if (index.column() == static_cast<int>(Column::Code))
            {
                return QStringLiteral("界面显示编号: ") + employee.code;
            }
            break;
        }
        case static_cast<int>(EmployeeDataRole2::EmployeeKeyRole2):
        {
            if (index.column() == static_cast<int>(Column::Code))
            {
                return employee.code;
            }
            break;
        }
    }
    return {};
}

QVariant EmployeeTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return {};

    if (section < 0 || section >= static_cast<int>(Column::Count))
        return {};

    const Column column = static_cast<Column>(section);
    switch (role)
    {
        case Qt::DisplayRole:
        {
            switch (column)
            {
            case Column::Sequence:   return QStringLiteral("序号");
            case Column::Code:       return QStringLiteral("编号");
            case Column::Name:       return QStringLiteral("姓名");
            case Column::Department: return QStringLiteral("部门");
            default: break;
            }
            break;
        }
        case Qt::ToolTipRole:
        {
            if (column == Column::Code)
            {
                return QStringLiteral("员工唯一编号，本表中不可编辑");
            }
            break;
        }
        default:
            break;
    }
    return {};
}

void EmployeeTableModel::setEmployees(QVector<EmployeeInfo2> employees)
{
    beginResetModel();
    m_employees = std::move(employees);
    endResetModel();
}
