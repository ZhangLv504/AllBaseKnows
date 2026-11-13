#ifndef QTKNOWLES_DISPLAYEMPLOYEEINFO2_H
#define QTKNOWLES_DISPLAYEMPLOYEEINFO2_H

#include <QWidget>
#include <QTableView>

#include "EmployeeTableModel.h"

QT_BEGIN_NAMESPACE

namespace Ui
{
    class DisplayEmployeeInfo2;
}


QT_END_NAMESPACE

class DisplayEmployeeInfo2 : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayEmployeeInfo2(QWidget* parent = nullptr);
    ~DisplayEmployeeInfo2() override;

private:
    // 初始化 模型视图
    void InitModelView();
    // 获取数据
    bool FetchAll ();
    // 显示数据
    void DisplayInfoToTable();

    QVector<EmployeeInfo2> m_buffer{};
    EmployeeTableModel* m_employeeInfoMode{};
    Ui::DisplayEmployeeInfo2* ui;
};


#endif //QTKNOWLES_DISPLAYEMPLOYEEINFO2_H