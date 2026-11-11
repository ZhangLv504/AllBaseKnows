# Qt 知识点

## 专题：Qt Model/View 表格体系

> Qt 版本：5.15.2
>
> 练习工程：`QtKnowles`

### 专题路线

本专题按“能显示 → 能建模 → 能变化 → 能处理大量数据 → 能分页 → 能应对生产环境极端情况”的顺序推进：

1. Model/View 基础分工与最小表格：`QTableView + QStandardItemModel`
2. `QModelIndex`、数据角色、表头、可编辑性与选择行为
3. 使用 `QAbstractTableModel` 为业务对象编写自定义表格模型
4. 行插入、删除、修改与模型通知协议
5. 使用 `QSortFilterProxyModel` 排序、过滤和映射索引
6. 大量数据下的内存、刷新与绘制性能优化
7. `canFetchMore/fetchMore` 分批加载和异步数据到达
8. 内存分页、数据库分页与服务端分页的边界和实现
9. 高频更新、后台线程、超大行数、异常数据源等极端情况
10. Delegate、自定义显示与编辑器，以及综合表格实战

以上是路线图，不代表这些知识点已经通关。每轮只展开和验证一个知识点。

---

## 知识点 01：Model/View 基础分工与最小表格

> 状态：已通关
>
> 开始时间：2026-10-26
>
> 通关时间：2026-11-11 00:20

### 本轮目标

1. 说清楚 Model、View 各自负责什么，以及二者如何连接。
2. 区分 `QTableView` 与 `QTableWidget` 的基本定位。
3. 在 `QtKnowles` 中使用 `QTableView + QStandardItemModel` 显示一个普通的小型表格。
4. 理解 `setModel()` 不会转移模型所有权，避免模型提前销毁。

### 一、核心原理

Qt 的 Model/View 不是把数据直接塞进表格控件，而是把“数据”和“显示数据的界面”分开：

- **Model（模型）**负责提供数据。它回答“有多少行、多少列”“某个单元格显示什么”“该单元格能否编辑”等问题。
- **View（视图）**负责把模型提供的数据绘制出来，并处理滚动、选择、键盘和鼠标交互。表格对应 `QTableView`。
- **Delegate（委托）**负责单元格如何绘制、如何编辑。本轮只知道它存在，后续单独学习。
- **Selection Model（选择模型）**负责记录当前项和选中项。本轮不展开。

最基本的数据流是：

```text
业务数据/表格数据
       ↓
     Model  ← View 通过索引询问数据
       ↓
      View  → 把数据绘制成表格
```

关键点是：**View 不保存业务数据，它只向 Model 请求当前需要显示的数据。** 因此同一个模型可以被不同视图观察；数据发生变化时，模型按协议通知视图刷新。

#### `QTableWidget` 与 `QTableView` 的区别

- `QTableWidget` 是便利控件，内部已经带有一个基于 item 的模型，适合非常简单、规模不大的临时表格。
- `QTableView` 是纯视图，必须通过 `setModel()` 连接一个模型，但可以连接 `QStandardItemModel`、自定义 `QAbstractTableModel`、数据库模型或代理模型。
- 本专题选择 `QTableView`，因为大数据、分页、延迟加载和业务对象映射都需要清晰的模型边界。

本轮使用 `QStandardItemModel`。它能快速搭建普通小表格，但每个单元格通常对应一个 `QStandardItem`，数据量很大时会产生明显的对象和内存开销。它是入门工具，不是百万行表格的最终方案。

### 二、最小示例

练习文件：`QtKnowles/src/main.cpp`

```cpp
#include <QApplication>
#include <QStandardItemModel>
#include <QTableView>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QStandardItemModel model(3, 3);
    model.setHorizontalHeaderLabels(
        {QStringLiteral("编号"), QStringLiteral("姓名"), QStringLiteral("部门")});

    model.setItem(0, 0, new QStandardItem(QStringLiteral("1001")));
    model.setItem(0, 1, new QStandardItem(QStringLiteral("张三")));
    model.setItem(0, 2, new QStandardItem(QStringLiteral("研发部")));

    model.setItem(1, 0, new QStandardItem(QStringLiteral("1002")));
    model.setItem(1, 1, new QStandardItem(QStringLiteral("李四")));
    model.setItem(1, 2, new QStandardItem(QStringLiteral("测试部")));

    model.setItem(2, 0, new QStandardItem(QStringLiteral("1003")));
    model.setItem(2, 1, new QStandardItem(QStringLiteral("王五")));
    model.setItem(2, 2, new QStandardItem(QStringLiteral("运维部")));

    QTableView view;
    view.setModel(&model);
    view.setWindowTitle(QStringLiteral("员工表 - Model/View 入门"));
    view.resize(600, 300);
    view.show();

    return app.exec();
}
```

代码中的核心只有三步：

1. 创建模型，并准备行、列和单元格数据。
2. 创建 `QTableView`。
3. 调用 `view.setModel(&model)` 建立连接。

`model` 与 `view` 都是 `main()` 中的局部对象。它们的析构顺序与创建顺序相反，因此退出事件循环后先销毁 `view`，再销毁 `model`，模型在视图使用期间一直有效。

`QStandardItemModel::setItem()` 接管传入的 `QStandardItem` 所有权，因此示例中不应手动 `delete` 这些 item。但是，`QTableView::setModel()` **不会**接管 `model` 的所有权；模型必须比使用它的视图活得更久。

### 三、企业场景示例

假设要实现一个“员工查询结果”窗口：

- 数据层取得员工编号、姓名和部门。
- Model 把这些字段组织成行列数据。
- `QTableView` 只负责显示、选择和滚动。
- 后续若切换到数据库分页或远程接口分页，View 可以继续使用，主要替换或扩展 Model。

这种分离让界面不会绑定某一种数据来源，也是本专题后面处理大量数据和分页的基础。

### 四、反例与常见坑

#### 1. 把 `QTableView` 当作数据容器

错误思路是不断尝试向 `QTableView` 本身“添加单元格”。`QTableView` 不保存数据，应修改模型。

#### 2. 模型是短生命周期局部变量

```cpp
void attachModel(QTableView *view)
{
    QStandardItemModel model(3, 3);
    view->setModel(&model);
} // model 在这里销毁，view 留下悬空指针
```

模型离开函数就被销毁，之后视图再访问会产生未定义行为。解决方式是让模型成为窗口成员、由合适的 `QObject` 父对象管理，或像最小示例一样保证作用域覆盖视图的整个使用期。

#### 3. 大数据仍为每个格子创建 `QStandardItem`

例如一百万行、二十列会产生两千万个 item，对象数量、内存占用和初始化时间都会非常糟糕。后续会改用自定义 `QAbstractTableModel`，只在 View 请求某个索引时返回数据。

#### 4. 为每个单元格放一个 `QWidget`

少量按钮列可以谨慎使用，但给大表的每个格子创建控件会导致对象数量和布局成本爆炸。常规显示应交给 View 和 Delegate 绘制。

#### 5. 忽略模型所有权

`setModel()` 只是建立引用关系，不意味着 View 会替你销毁模型。必须明确模型由栈对象、父子对象系统还是智能指针管理，并保证生命周期正确。

### 五、本轮练习

在 `QtKnowles/src/main.cpp` 完成以下练习：

1. 使用 `QApplication`、`QTableView`、`QStandardItemModel` 显示表格。
2. 表格至少包含 3 列：编号、姓名、部门。
3. 至少显示 5 行员工数据，不要只照抄示例中的 3 行。
4. 给窗口设置标题和合适的初始大小。
5. 在 `setModel()` 附近写一句注释，说明 View 是否拥有 Model，以及本程序如何保证 Model 生命周期安全。
6. 构建并运行，确认表头、5 行数据、中文内容均正常显示。

完成后请提交 `QtKnowles/src/main.cpp` 的实现以及构建/运行结果，再进行本知识点的通关验证。

---

## 知识点 02：QModelIndex、数据角色、表头、可编辑性与选择行为

> 状态：已通关
>
> 开始时间：2026-11-11
>
> 通关时间：2026-11-11 23:49

### 本轮目标

1. 理解 `QModelIndex` 是 View 和 Model 之间定位数据的通用坐标，而不是业务数据本身。
2. 能使用不同的 `Qt::ItemDataRole` 为同一个索引提供显示值、编辑值、提示信息和业务 ID。
3. 能通过模型设置表头，并区分表头数据与普通单元格数据。
4. 能控制哪些单元格允许编辑，以及设置整行单选行为。
5. 能从用户点击的索引找到整行及对应稳定业务键，而不依赖当前行号作为身份。

### 一、QModelIndex：模型中的通用位置

`QModelIndex` 可以理解为模型发出的一个轻量位置描述，主要包含：

- 行号 `row()`；
- 列号 `column()`；
- 所属模型 `model()`；
- 树形模型中的父索引 `parent()`；表格的顶层索引通常没有有效父索引。

它不拥有数据，也不复制单元格内容。View 拿着索引向 Model 询问数据：

```cpp
const QModelIndex index = m_employeeInfoMode->index(0, 2);
if (index.isValid()) {
    const QString name = m_employeeInfoMode
        ->data(index, Qt::DisplayRole)
        .toString();
}
```

这比直接调用 `QStandardItemModel::item()` 更通用。未来换成自定义 `QAbstractTableModel` 后，`index()`、`data()` 和 Role 的用法仍然成立。

从用户点击的某一列切换到同一行的另一列，可以使用：

```cpp
const QModelIndex codeIndex = clickedIndex.sibling(clickedIndex.row(), 1);
```

普通 `QModelIndex` 可能在插入、删除、排序或模型重置后失效。业务逻辑如果需要长期记住一个员工，应保存稳定业务键。本轮编号在数据库中唯一，而且练习要求编号列只读，因此可以用编号；如果以后业务允许改编号，应改用不可变主键。确实需要跨结构变化保留索引时才考虑 `QPersistentModelIndex`。

### 二、数据角色：一个格子不只有一份数据

调用模型的 `data(index, role)` 时，Role 表示“这次想问哪一种数据”。常见 Role 如下：

| Role | 作用 |
| --- | --- |
| `Qt::DisplayRole` | View 正常显示的文本或值 |
| `Qt::EditRole` | 进入编辑器时使用的值 |
| `Qt::ToolTipRole` | 鼠标悬停时的提示 |
| `Qt::DecorationRole` | 图标、图片等装饰 |
| `Qt::TextAlignmentRole` | 文本对齐方式 |
| `Qt::CheckStateRole` | 复选框状态 |
| `Qt::ForegroundRole` / `Qt::BackgroundRole` | 前景色和背景色 |
| `Qt::UserRole` 及更大的值 | 应用自定义数据，例如数据库主键或唯一业务编号 |

企业代码不要到处裸写 `Qt::UserRole + 1`，应给角色命名：

```cpp
enum EmployeeDataRole
{
    EmployeeKeyRole = Qt::UserRole + 1
};
```

显示内容和业务身份可以保存在同一个 item 中：

```cpp
auto* codeItem = new QStandardItem(employeeInfo.code);
codeItem->setData(employeeInfo.code, EmployeeKeyRole);
codeItem->setData(
    QStringLiteral("唯一员工编号：%1").arg(employeeInfo.code),
    Qt::ToolTipRole);
```

读取时只通过模型接口：

```cpp
const QString employeeKey =
    m_employeeInfoMode->data(codeIndex, EmployeeKeyRole).toString();
const QString displayCode =
    m_employeeInfoMode->data(codeIndex, Qt::DisplayRole).toString();
```

当前编号在数据库中唯一，本轮界面又把编号列设为只读，因此它可以作为本次练习的稳定业务键。把它放入自定义 Role 的意义是让代码明确区分“用于显示的值”和“用于定位记录的业务身份”；以后即使显示格式变为 `员工-20260001`，Role 中仍可保留原始编号。若未来允许修改编号，则应使用不可变主键作为 Role 值。

### 三、表头也是模型数据

表头不是 `QTableView` 自己保存的文本，而是模型通过 `headerData()` 提供的数据。`QStandardItemModel` 的便利写法是：

```cpp
m_employeeInfoMode->setHorizontalHeaderLabels({
    QStringLiteral("序号"),
    QStringLiteral("编号"),
    QStringLiteral("姓名"),
    QStringLiteral("部门")
});
```

更通用的写法是按方向、序号和 Role 设置：

```cpp
m_employeeInfoMode->setHeaderData(
    1, Qt::Horizontal, QStringLiteral("编号"), Qt::DisplayRole);
m_employeeInfoMode->setHeaderData(
    1, Qt::Horizontal, QStringLiteral("员工唯一编号，只读"), Qt::ToolTipRole);
```

横向表头的 section 对应列号；纵向表头的 section 对应行号。

### 四、可编辑性由 Model/Item 的 flags 决定

`QStandardItem` 默认可编辑。只读列可以直接设置：

```cpp
sequenceItem->setEditable(false);
codeItem->setEditable(false);
nameItem->setEditable(true);
departmentItem->setEditable(true);
```

底层含义是调整 `Qt::ItemIsEditable` 标志。View 的 `editTriggers` 决定“用户通过什么动作尝试编辑”，Model/Item 的 flags 决定“这个索引是否允许编辑”，二者职责不同。

```cpp
ui->tableView->setEditTriggers(
    QAbstractItemView::DoubleClicked |
    QAbstractItemView::SelectedClicked);
```

本轮修改 `QStandardItemModel` 只会改变内存中的 item，不会自动执行 SQL 更新。数据库回写、校验和失败回滚以后再学习，不能把“界面可编辑”误认为“数据库已更新”。

### 五、选择行为与 Selection Model

员工表通常希望点击任意单元格都选中整行，并且一次只选一名员工：

```cpp
ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
```

`currentIndex()` 表示当前索引，`selectedRows()` 可以取得选中的行索引：

```cpp
const QModelIndex current = ui->tableView->currentIndex();
if (current.isValid()) {
    const QModelIndex codeIndex = current.sibling(current.row(), 1);
    const QString employeeKey =
        m_employeeInfoMode->data(codeIndex, EmployeeKeyRole).toString();
}
```

需要注意：调用 `setModel()` 会使 View 建立新的 Selection Model。如果要连接 `selectionChanged` 等选择信号，应在 `setModel()` 之后连接 `ui->tableView->selectionModel()`。

### 六、结合当前员工表的最小增量示例

练习位置：

- `QtKnowles/src/demo1/DisplayEmployeeInfo.h`
- `QtKnowles/src/demo1/DisplayEmployeeInfo.cpp`

先定义当前表格使用的稳定业务键 Role：

```cpp
enum EmployeeDataRole
{
    EmployeeKeyRole = Qt::UserRole + 1
};
```

当前工程已经确认 `code` 在数据库中唯一，而且本轮要求编号列只读，因此 SQL 继续查询 `code` 即可，不强制额外读取自增 `id`：

```cpp
SELECT code, name, department
FROM employer_info1
ORDER BY id
```

填充编号列时把原始编号同时写入 `EmployeeKeyRole`。双击任意单元格时，通过传入的 `QModelIndex` 读取同一行编号列上的稳定业务键：

```cpp
connect(ui->tableView, &QTableView::doubleClicked,
        this, [this](const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }

    const QModelIndex codeIndex = index.sibling(index.row(), 1);
    const QString employeeKey =
        m_employeeInfoMode->data(codeIndex, EmployeeKeyRole).toString();
    const QString displayCode =
        m_employeeInfoMode->data(codeIndex, Qt::DisplayRole).toString();

    qInfo().noquote()
        << "当前员工:" << employeeKey
        << "显示编号:" << displayCode;
});
```

### 七、企业场景示例

员工表经过排序后，“第 5 行”可能不再是原来的员工。删除、编辑和打开详情页时，应从当前行的 `EmployeeKeyRole` 读取唯一员工编号，再调用业务接口，而不能拿 `row()` 当记录身份。

自增 `id` 是代理键，`code` 是自然业务键。两者都可能唯一，但只有在编号不允许修改的使用范围内，`code` 才能长期作为稳定身份。本轮编号列只读，因此练习选择 `code`；如果将来业务允许修改编号，或者只是在代码中“约定唯一”而数据库没有唯一约束，则应改用数据库主键。

同一个格子可以同时拥有：

- `DisplayRole`：界面显示的员工编号；
- `EditRole`：编辑器使用的值；
- `ToolTipRole`：字段说明；
- `EmployeeKeyRole`：原始唯一员工编号。

View 只根据自己需要的 Role 取值，业务代码也能读取稳定身份信息。虽然当前 `DisplayRole` 和 `EmployeeKeyRole` 初始值相同，但两者的职责不同，显示格式变化不应改变业务键。

### 八、反例与常见坑

#### 1. 把行号当业务 ID

排序、过滤、插入和删除都会改变行号。行号只表示当前模型位置，业务操作必须使用稳定业务键。

#### 2. 为了保存 ID 解析显示文本

不要从“员工 1001”字符串中截取 ID。显示格式一改，业务逻辑就会失效。应使用自定义 Role。

#### 3. 长期保存普通 QModelIndex

模型结构变化后旧索引可能失效。短期信号处理可以使用它；长期业务状态应保存稳定业务键，必要时才使用 `QPersistentModelIndex`。

#### 4. 只在代码里假设编号唯一

使用编号作为长期业务键的前提应由数据库唯一约束和不可修改规则共同保证，而不能只靠界面代码约定。本轮只在编号列只读的练习范围内使用它；如果编号允许修改，就应使用不可变主键定位记录。

#### 5. 只设置 editTriggers 就认为列只读

`editTriggers` 是 View 级别的触发方式。某个索引能否编辑应由 item/model flags 控制。

#### 6. 认为编辑 QStandardItem 会自动更新数据库

当前模型是内存模型，编辑只改变内存数据。数据库写回需要单独设计校验、SQL、失败提示和回滚。

#### 7. 在 setModel() 之前保存 selectionModel()

`setModel()` 会替换选择模型。需要监听选择变化时，先设置 Model，再获取并连接 Selection Model。

### 九、本轮练习

在现有 `QtKnowles/src/demo1/DisplayEmployeeInfo.*` 上完成：

1. 以“编号在数据库中唯一，并且本轮编号列只读”为本轮身份契约，不要求给 `EmployeeInfo` 增加自增 `id`；若未来允许改编号，再切换为不可变主键。
2. 定义命名的自定义角色 `EmployeeKeyRole = Qt::UserRole + 1`。
3. 把原始员工编号保存到编号列 item 的 `EmployeeKeyRole`，并为编号列设置 `ToolTipRole`。
4. 序号、编号两列只读；姓名、部门两列允许编辑。
5. 表格设置为整行单选，并保留双击可编辑行为。
6. 使用 `setHeaderData()` 至少给“编号”表头设置一个 ToolTip。
7. 双击任意单元格时，使用 `QModelIndex::sibling()` 和 `model->data()` 分别读取并输出该行 `EmployeeKeyRole`、`DisplayRole` 以及点击行列；不要用 `itemFromIndex()` 完成读取。
8. 构建并运行，验证整行单选、只读列、可编辑列、表头提示和双击输出均符合预期。

练习中的编辑只验证内存模型行为，不要求把修改写回数据库。
