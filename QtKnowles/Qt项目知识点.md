# Qt Model/View 角色（Role）数据存储机制总结

在 Qt 的 Model/View 架构中，表格的一个单元格（Item）不仅仅能存储单一的文本，而是可以挂载多种不同类型的数据。这些数据通过角色（Role）进行隔离和分类存储。

## 1. 核心存储结构

`QStandardItem` 的底层使用类似于字典或哈希表的数据结构（通常映射为 `QMap<int, QVariant>`）来管理数据：

- **键（Key）**：角色（Role），本质是一个 `int` 类型的枚举值。
- **值（Value）**：数据（Data），被封装为通用的 `QVariant` 类型。

## 2. 数据的分类与存入

对同一个 Item 赋值时，不同的角色代表不同的“数据标签”，它们互不干扰、独立存储。

- **默认显示角色（`Qt::DisplayRole` / `Qt::EditRole`）**
    - **场景**：通过 `new QStandardItem(text)` 构造时默认触发。
    - **作用**：`DisplayRole` 决定界面上直接显示的文本；`EditRole` 决定双击进入编辑模式时输入框内的默认文本。
- **自定义角色（`UserRole` 系列，如 `EmployeeKeyRole`）**
    - **场景**：通过 `setData(value, EmployeeKeyRole)` 存入。
    - **作用**：用于在后台“隐蔽”地存储业务核心数据（如数据库主键、唯一编号等）。这些数据对用户不可见，但程序随时可以提取。
- **工具提示角色（`Qt::ToolTipRole`）**
    - **场景**：通过 `setData(text, Qt::ToolTipRole)` 存入。
    - **作用**：当鼠标悬停在该单元格上时，系统底层自动提取此角色的数据并作为弹窗提示显示。

## 3. 数据的精确读取

在信号槽（如 `doubleClicked`）中读取数据时，遵循以下逻辑：

1. **定位 Item**：通过传入的 `QModelIndex`（包含行号和列号）在模型中找到对应的 `QStandardItem`。
2. **按图索骥**：通过 `m_model->data(index, role)` 方法，拿着特定的“角色”去该 Item 的内部字典中进行精确的键值查找。
3. **数据还原**：获取到 `QVariant` 类型的值后，通过 `.toString()` 或其他转换方法还原为实际的数据类型。

## 4. 核心结论

**角色（Role）的本质是“数据标签”。** 界面显示文本、背景颜色、悬停提示以及业务主键等，都在同一个 Item 里面**按角色分类存放，互不覆盖，随用随取**。

---

# 头文件循环依赖问题总结

## 1. 现象

`src/demo2` 中 `EmployeeTableModel.h` 与 `DisplayEmployeeInfo2.h` 互相包含后，编译出现一批看似不相关的错误：

```text
DisplayEmployeeInfo2.h(49): error C2143: 语法错误: 缺少“;”(在“*”的前面)
DisplayEmployeeInfo2.h(49): error C4430: 缺少类型说明符 - 假定为 int

EmployeeTableModel.h(21): error C2065: “EmployeeInfo2”: 未声明的标识符
EmployeeTableModel.h(24): error C2065: “EmployeeInfo2”: 未声明的标识符
EmployeeTableModel.h(21): error C2923: "QVector": "EmployeeInfo2" 不是参数 "T" 的有效 模板 类型参数

DisplayEmployeeInfo2.cpp(92): error C2664:
无法将参数 1 从“QVector<EmployeeInfo2>”转换为“QVector”
```

典型特征是：报错行本身语法没有问题，错误内容在“另一个头文件里明明定义过”，而且换一个编译单元报错位置又会变化。

## 2. 根因：两个头文件互相包含

```cpp
// EmployeeTableModel.h
#include "DisplayEmployeeInfo2.h"   // 为了拿 EmployeeInfo2

// DisplayEmployeeInfo2.h
#include "EmployeeTableModel.h"     // 为了拿 EmployeeTableModel
```

依赖形成环：

```text
EmployeeTableModel.h  ->  DisplayEmployeeInfo2.h
        ^                          |
        |__________________________|
```

`#include` 是文本展开，编译器必须按某个顺序处理。两种顺序都会失败：

**顺序 A：先展开 `EmployeeTableModel.h`**

1. 进入 `EmployeeTableModel.h`
2. 包含 `DisplayEmployeeInfo2.h`
3. `DisplayEmployeeInfo2.h` 再包含 `EmployeeTableModel.h`，被 include guard 跳过
4. 此时 `EmployeeTableModel` 类尚未定义，但 `DisplayEmployeeInfo2` 已经声明了成员 `EmployeeTableModel* m_employeeInfoMode`
5. 编译器不认识 `EmployeeTableModel` → `C2143` / `C4430`

**顺序 B：先展开 `DisplayEmployeeInfo2.h`**

1. 进入 `DisplayEmployeeInfo2.h`
2. 包含 `EmployeeTableModel.h`
3. `EmployeeTableModel.h` 再包含 `DisplayEmployeeInfo2.h`，被 include guard 跳过
4. 此时 `EmployeeInfo2` 尚未定义，但 `EmployeeTableModel.h` 已经使用 `QVector<EmployeeInfo2>`
5. 模板参数类型无效 → `C2065` / `C2923` / `C2955`

`C2664` 只是连带结果：`QVector<EmployeeInfo2>` 已经因类型无效而退化，`setEmployees(m_buffer)` 自然匹配不上。

## 3. 为什么 include guard 解决不了

include guard 或 `#pragma once` 只保证**同一个文件不会被重复展开**，它不负责调整定义顺序。

被跳过的那次包含不会“稍后再展开”，而是直接消失。于是“后需要的那个类型”在当前位置就是不存在，guard 反而让错误以“未声明标识符”的形式出现。

## 4. 修复原则：依赖必须单向

业务数据类型属于模型，不应该由模型反向依赖窗口头文件。

```cpp
// EmployeeTableModel.h
#include <QAbstractTableModel>
#include <QString>
#include <QVector>

struct EmployeeInfo2
{
    QString code;
    QString name;
    QString department;
};

class EmployeeTableModel : public QAbstractTableModel
{
    // ...
};
```

```cpp
// DisplayEmployeeInfo2.h
#include <QWidget>
#include <QTableView>

#include "EmployeeTableModel.h"   // 单向依赖，提供 EmployeeInfo2 和 EmployeeTableModel

class DisplayEmployeeInfo2 : public QWidget
{
    // ...
    QVector<EmployeeInfo2> m_buffer{};
    EmployeeTableModel* m_employeeInfoMode{};
};
```

同时删除 `DisplayEmployeeInfo2.h` 中重复定义的 `EmployeeInfo2`。

修复后的依赖方向：

```text
DisplayEmployeeInfo2.h  ->  EmployeeTableModel.h
```

## 5. 前置声明为什么不够

当前场景不能只写：

```cpp
struct EmployeeInfo2;   // 前置声明
```

原因：

- `QVector<EmployeeInfo2>` 作为**按值成员**，类定义时就需要完整类型，因为它要确定大小并生成析构；
- `setEmployees(QVector<EmployeeInfo2>)` 按值传参并在函数体内移动/保存，同样需要完整类型。

所以业务结构体必须放在模型头文件或单独的公共头文件中，由使用方包含。

如果只是**指针成员**，例如 `EmployeeTableModel*`，则可以前置声明 `class EmployeeTableModel;` 并在 `.cpp` 中再包含对应头文件。但这只适用于指针，不能解决 `QVector<T>` 的完整类型要求。

## 6. 预防规则

1. 分层依赖只能从上层指向下层：窗口/View 依赖 Model，Model 不依赖窗口。
2. 共享的业务数据结构放在底层头文件（本例是 `EmployeeTableModel.h`），不要放在窗口头文件里。
3. 头文件只包含自己直接使用的类型；能用前置声明 + `.cpp` 包含解决的，不要放进头文件。
4. 出现 `C2143`、`C4430`、`C2065`，且错误类型在“另一个头文件里已定义”时，优先检查是否存在循环包含。
5. 循环包含不会因为换用 `#pragma once` 而消失，include guard 不是依赖管理工具。

## 7. 本例结论

`EmployeeInfo2` 是模型的数据契约，归属 `EmployeeTableModel.h`；`DisplayEmployeeInfo2.h` 单向包含模型头文件。这样既消除循环依赖，也避免模型层反向依赖 UI 层。