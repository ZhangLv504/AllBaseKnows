#include "DisplayEmployeeInfo.h"
#include "ui_DisplayEmployeeInfo.h"

#include "DatabaseService.h"
#include <QString>
#include <QDebug>

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
    connect(ui->tableView,&QTableView::doubleClicked,this,[this](const QModelIndex& index) {
        if (index.isValid())
        {
            const auto& codeIndex = index.sibling(index.row(), 1);
            qDebug() << "员工唯一编号: " << m_employeeInfoMode->data(codeIndex,
                static_cast<int>(EmployeeDataRole::EmployeeKeyRole)).toString();
            qDebug() << "界面显示编号: " << m_employeeInfoMode->data(codeIndex,
                Qt::DisplayRole).toString();
            qDebug() << "点击行号： " << index.row();
            qDebug() << "点击列号： " << index.column();
        }
    });
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
    m_employeeInfoMode->setHeaderData(
        1,
        Qt::Horizontal,
        QStringLiteral("员工唯一编号，本表中不可编辑"),
        Qt::ToolTipRole);

    // QTableView 不拥有 Model；Model 的父对象是当前窗口 this，
    // 因而会随窗口一起销毁，并覆盖 TableView 的整个使用期。
    ui->tableView->setModel(m_employeeInfoMode);
    ui->tableView->verticalHeader()->hide();


    ui->tableView->setSelectionBehavior(QAbstractItemView ::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(
        QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
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

        auto* sequenceItem = new QStandardItem(QString::number(i + 1));
        m_employeeInfoMode->setItem(i, 0, sequenceItem);

        auto* codeItem = new QStandardItem(employeeInfo.code);
        codeItem->setData(employeeInfo.code, static_cast<int>(EmployeeDataRole::EmployeeKeyRole));
        codeItem->setData(QString("唯一员工编号：%1").arg(employeeInfo.code), Qt::ToolTipRole);
        m_employeeInfoMode->setItem(i, 1, codeItem);

        auto* nameItem = new QStandardItem(employeeInfo.name);
        m_employeeInfoMode->setItem(i, 2, nameItem);

        auto* departmentItem = new QStandardItem(employeeInfo.department);
        m_employeeInfoMode->setItem(i, 3, departmentItem);

        sequenceItem->setEditable(false);
        codeItem->setEditable(false);
        nameItem->setEditable(true);
        departmentItem->setEditable(true);
    }
}
