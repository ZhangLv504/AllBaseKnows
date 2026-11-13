#include "DisplayEmployeeInfo2.h"
#include "ui_DisplayEmployeeInfo2.h"
#include "CodeFormat.h"
#include "DatabaseService.h"
#include <QString>
#include <QDebug>
#include <QPushButton>

DisplayEmployeeInfo2::DisplayEmployeeInfo2(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DisplayEmployeeInfo2)
{
    ui->setupUi(this);

    // 初始化 模型视图
    InitModelView();

    // 读取数据
    FetchAll();

    // 显示数据
    connect(ui->pushButton, &QPushButton::clicked, this, &DisplayEmployeeInfo2::DisplayInfoToTable);
    connect(ui->tableView,&QTableView::doubleClicked,this,[this](const QModelIndex& index) {
        if (index.isValid())
        {
            const auto& codeIndex = index.sibling(index.row(), static_cast <int>(Column::Code));
            qDebug() << "行号:" << index.row()
                     << "列号:" << index.column()
                     << "显示值:" << m_employeeInfoMode->data(index, Qt::DisplayRole).toString()
                     << "业务键:" << m_employeeInfoMode->data(codeIndex,
                            static_cast<int>(EmployeeDataRole2::EmployeeKeyRole2)).toString();
        }
    });
}

DisplayEmployeeInfo2::~DisplayEmployeeInfo2()
{
    delete ui;
}

// 初始化 模型视图
void DisplayEmployeeInfo2::InitModelView()
{
    // 初始化数据模型
    m_employeeInfoMode = new EmployeeTableModel(this);

    // QTableView 不拥有 Model；Model 的父对象是当前窗口 this，
    // 因而会随窗口一起销毁，并覆盖 TableView 的整个使用期。
    ui->tableView->setModel(m_employeeInfoMode);
    ui->tableView->verticalHeader()->hide();


    ui->tableView->setSelectionBehavior(QAbstractItemView ::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // 本轮模型只读；后续实现 flags()/setData() 后再放开编辑触发器
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

// 获取数据
bool DisplayEmployeeInfo2::FetchAll()
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
        EmployeeInfo2 employeeInfo;
        employeeInfo.code = row.value("code").toString();
        employeeInfo.name = row.value(QStringLiteral("name")).toString();
        employeeInfo.department = row.value(QStringLiteral("department")).toString();
        m_buffer.push_back(employeeInfo);
    }

    return true;
}

// 显示数据
void DisplayEmployeeInfo2::DisplayInfoToTable()
{
    m_employeeInfoMode->setEmployees(m_buffer);
}