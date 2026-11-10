#include "DisplayEmployeeInfo.h"
#include "ui_DisplayEmployeeInfo.h"

#include "../dbPack/DatabaseService.h"
#include <QString>

DisplayEmployeeInfo::DisplayEmployeeInfo(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DisplayEmployeeInfo)
{
    ui->setupUi(this);

    // 初始化 模型视图
    InitModelView();

    // 读取数据
    FetchAll();

    // 显示数据
    connect(ui->pushButton, &QPushButton::clicked, this, &DisplayEmployeeInfo::DisplayInfoToTable);
}

DisplayEmployeeInfo::~DisplayEmployeeInfo()
{
    delete ui;
}

// 初始化 模型视图
void DisplayEmployeeInfo::InitModelView()
{
    // 初始化数据模型
    m_employeeInfoMode = new QStandardItemModel(0, 4, this);
    const QStringList& labels = {
        QStringLiteral("序号"),QStringLiteral("编号"),
        QStringLiteral("姓名"), QStringLiteral("部门")
    };
    m_employeeInfoMode->setHorizontalHeaderLabels(labels);

    // QTableView 不拥有 Model；Model 的父对象是当前窗口 this，
    // 因而会随窗口一起销毁，并覆盖 TableView 的整个使用期。
    ui->tableView->setModel(m_employeeInfoMode);
    ui->tableView->verticalHeader()->hide();
}

// 获取数据
bool DisplayEmployeeInfo::FetchAll()
{
    qtmysql::QueryResult results;
    auto& db = DatabaseService::instance().db();

    QString error;

    if (!db.select(
        QStringLiteral("SELECT code, name, department FROM employer_info1 ORDER BY id"),
        &results,{}, &error
    )){
        return false;
    }

    m_buffer.clear();
    for (auto& row : results.rows())
    {
        EmployeeInfo employeeInfo;
        employeeInfo.code = row.value("code").toString();
        employeeInfo.name = row.value(QStringLiteral("name")).toString();
        employeeInfo.department = row.value(QStringLiteral("department")).toString();
        m_buffer.push_back(employeeInfo);
    }

    return true;
}

// 显示数据
void DisplayEmployeeInfo::DisplayInfoToTable()
{
    if (m_buffer.isEmpty())
    {
        return;
    }

    // 清空旧行
    m_employeeInfoMode->removeRows(0, m_employeeInfoMode->rowCount());

    for (int i = 0; i < m_buffer.count(); i++)
    {
        m_employeeInfoMode->insertRow(i);

        EmployeeInfo employeeInfo = m_buffer.at(i);
        m_employeeInfoMode->setItem(i, 0, new QStandardItem(QString::number(i + 1)));
        m_employeeInfoMode->setItem(i, 1, new QStandardItem(employeeInfo.code));
        m_employeeInfoMode->setItem(i, 2, new QStandardItem(employeeInfo.name));
        m_employeeInfoMode->setItem(i, 3, new QStandardItem(employeeInfo.department));
    }
}
