#ifndef QTKNOWLES_EMPLOYEETABLEMODEL_H
#define QTKNOWLES_EMPLOYEETABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>

struct EmployeeInfo2
{
    QString code;
    QString name;
    QString department;
};

enum class EmployeeDataRole2
{
    EmployeeKeyRole2 = Qt::UserRole + 1
};

enum class Column
{
    Sequence,       // 序号
    Code,           // 编号
    Name,           // 名称
    Department,     // 部门
    Count           // 列数量
};

class EmployeeTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit EmployeeTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setEmployees(QVector<EmployeeInfo2> employees);

private:
    QVector<EmployeeInfo2> m_employees;
};


#endif //QTKNOWLES_EMPLOYEETABLEMODEL_H