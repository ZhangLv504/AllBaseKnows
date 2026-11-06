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

> 状态：学习中 / 待通关
>
> 开始时间：2026-10-26

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
