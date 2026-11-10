#ifndef QTKNOWLES_DISPLAYEMPLOYEEINFO_H
#define QTKNOWLES_DISPLAYEMPLOYEEINFO_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE

namespace Ui
{
    class DisplayEmployeeInfo;
}

QT_END_NAMESPACE

struct EmployeeInfo
{
    QString code;
    QString name;
    QString department;
};

class DisplayEmployeeInfo : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayEmployeeInfo(QWidget* parent = nullptr);
    ~DisplayEmployeeInfo() override;

private:
    // 初始化 模型视图
    void InitModelView();
    // 获取数据
    bool FetchAll ();
    // 显示数据
    void DisplayInfoToTable();

    QVector<EmployeeInfo> m_buffer{};
    QStandardItemModel* m_employeeInfoMode{};
    Ui::DisplayEmployeeInfo* ui;
};


#endif //QTKNOWLES_DISPLAYEMPLOYEEINFO_H