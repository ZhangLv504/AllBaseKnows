# QtMySQL 用法

`QtMySQL` 是对 QtSql + QMYSQL 驱动的轻量封装，交付形式为动态库（DLL）+ 头文件。
它是位于 `otherbase/QtMySQL` 的独立工程，与 `QtKnowles` 并列，二者不共享构建目标。

工程内头文件位于 `QtMySQL/include`：

- `QtMySQL/include/MySqlDatabase.h`
- `QtMySQL/include/QtMySQLGlobal.h`

构建 QtMySQL 后会自动复制一份交付物到 `QtKnowles`：

| 源（QtMySQL） | 副本（QtKnowles） |
|---|---|
| `bin/QtMySQL.dll` | `bin/QtMySQL.dll` |
| `lib/QtMySQL.lib` | `lib/QtMySQL.lib` |
| `include/MySqlDatabase.h`、`include/QtMySQLGlobal.h` | `publics/include/` |
| `bin/sqldrivers/qsqlmysql.dll` | `bin/sqldrivers/qsqlmysql.dll` |

## 接入

在消费方工程中按 IMPORTED target 接入副本：

```cmake
add_library(QtMySQL SHARED IMPORTED)
set_target_properties(QtMySQL PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/bin/QtMySQL.dll"
    IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/QtMySQL.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/publics/include"
)

target_link_libraries(你的目标 PRIVATE QtMySQL)
```

不通过 CMake target 时，把 `publics/include` 加入头文件搜索路径，并链接 `lib/QtMySQL.lib`。
构建顺序固定：先构建 QtMySQL 生成副本，再构建消费方工程。

代码中引入：

```cpp
#include "MySqlDatabase.h"

using qtmysql::ConnectOptions;
using qtmysql::MySqlDatabase;
using qtmysql::QueryResult;
```

## 构建产物与运行依赖

| 路径 | 说明 |
|---|---|
| `QtMySQL/bin/QtMySQL.dll` | 封装库（QtMySQL 自身产物） |
| `QtMySQL/lib/QtMySQL.lib` | MSVC 导入库，链接用 |
| `QtMySQL/include/` | 公开头文件 |
| `QtMySQL/bin/sqldrivers/qsqlmysql.dll` | 从 Qt 源码编译的 QMYSQL 驱动插件 |
| `QtKnowles/bin/`、`QtKnowles/lib/`、`QtKnowles/publics/include/` | 构建 QtMySQL 时自动复制的交付副本 |
| `QtKnowles/bin/libmysql.dll` | MySQL 客户端库（部署时手动放置） |
| `QtKnowles/bin/jemalloc.dll`、`libssl-1_1-x64.dll`、`libcrypto-1_1-x64.dll` | libmysql 的运行时依赖 |

注意：

- `QtKnowles/bin/QtKnowles.exe` 与上述 DLL 同目录，插件在 `QtKnowles/bin/sqldrivers`。
- 单独使用 `QtMySQL.dll` 时，运行目录同样需要 `libmysql.dll` 及其依赖。
- 插件必须与当前 Qt 版本完全一致；QtMySQL 通过 `QT_SQLDRIVERS_SRC_DIR` 指向 Qt 安装的 `Src` 组件源码现场编译。
- `libmysql` 使用 release CRT，请使用 `RelWithDebInfo` / `Release` 构建，不要用 Debug。
- 部署时 `bin` 下的 DLL、`sqldrivers/qsqlmysql.dll` 要一起带走；`windeployqt` 不认识 `libmysql` 及其依赖。

## 快速开始

```cpp
int main()
{
    qtmysql::MySqlDatabase db;
    QString error;

    if (!db.connect("127.0.0.1", 3306, "zl_study_db", "your_password", &error)) {
        qCritical().noquote() << "连接失败:" << error;
        return 1;
    }

    qtmysql::QueryResult result;
    if (!db.select("SELECT VERSION() AS version", &result, {}, &error)) {
        qCritical().noquote() << "查询失败:" << error;
        return 1;
    }
    qInfo() << result.value(0, "version").toString();
    return 0;
}
```

## 连接

### 便捷连接

参数依次为 IP、端口、数据库名、密码；内部默认使用 `root` 用户、`utf8mb4`、5 秒连接超时。

```cpp
bool connect(const QString &ip, quint16 port, const QString &database,
             const QString &password, QString *error = nullptr);
```

```cpp
db.connect("127.0.0.1", 3306, "zl_study_db", "your_password", &error);
```

### 完整配置

需要非 root 用户、自定义字符集或超时时用 `open()`：

```cpp
qtmysql::ConnectOptions options;
options.host = "127.0.0.1";
options.port = 3306;
options.user = "app_user";
options.password = "your_password";
options.database = "zl_study_db";
options.charset = "utf8mb4";
options.connectTimeoutSeconds = 5;

if (!db.open(options, &error)) {
    // error 为失败原因
}
```

`open()` 连接成功后会执行 `SET NAMES <charset>`；字符集名必须是字母、数字、下划线。

### 关闭与状态

```cpp
db.isOpen();                 // 是否已连接
db.close();                  // 断开并移除连接
db.connectionName();         // 本次连接名，如 qtmysql_0
```

析构时会自动 `close()`。

## 执行 SQL：execute

`execute()` 用于 INSERT、UPDATE、DELETE、DDL 等非查询语句，返回是否成功。

```cpp
bool execute(const QString &sql,
             const QVariantList &params = QVariantList(),
             QString *error = nullptr);
```

```cpp
db.execute("DELETE FROM qtmysql_demo WHERE id = ?", {2}, &error);
```

参数统一使用 `?` 位置占位符，下标从 0 开始；不支持 `:name` 命名占位符。
一次只能执行一条语句，不支持多结果集。

## 查询：select

`select()` 执行 SELECT 并把结果一次性取回 `QueryResult`。

```cpp
bool select(const QString &sql, QueryResult *result,
            const QVariantList &params = QVariantList(),
            QString *error = nullptr);
```

```cpp
qtmysql::QueryResult result;
db.select("SELECT id, name, note FROM qtmysql_demo WHERE id > ? ORDER BY id",
          &result, {0}, &error);
```

### QueryResult

```cpp
result.isValid();        // select 成功为 true
result.isEmpty();        // 没有数据行
result.rowCount();       // 行数
result.columnCount();    // 列数
result.columnNames();    // 列名列表

result.value(row, col);           // 按列序号取值
result.value(row, "column");      // 按列名取值
result.row(index);                // 一行转 QVariantMap
result.rows();                    // 全部行转 QVector<QVariantMap>
```

```cpp
for (const QVariantMap &row : result.rows()) {
    const quint64 id = row.value("id").toULongLong();
    const QString name = row.value("name").toString();
    const QDateTime createdAt = row.value("created_at").toDateTime();
}
```

结果集是完整缓冲的，`rowCount()` 始终可用；大表请用 `LIMIT` 分页。

## 示例表

以下增删改查示例基于：

```sql
CREATE TABLE IF NOT EXISTS qtmysql_demo (
    id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    note VARCHAR(128) NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

用本库建表：

```cpp
db.execute(
    "CREATE TABLE IF NOT EXISTS qtmysql_demo ("
    " id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,"
    " name VARCHAR(64) NOT NULL,"
    " note VARCHAR(128) NULL,"
    " created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
    ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4",
    {}, &error);
```

## INSERT

```cpp
// 全部字面量
db.execute("INSERT INTO qtmysql_demo (name, note) VALUES ('张三', '备注')", {}, &error);

// 位置参数（推荐，自动转义）
db.execute("INSERT INTO qtmysql_demo (name, note) VALUES (?, ?)",
           {QStringLiteral("李四"), QStringLiteral("参数绑定")}, &error);

// 多行插入；QVariant() 表示 NULL
db.execute("INSERT INTO qtmysql_demo (name, note) VALUES (?, ?), (?, ?)",
           {QStringLiteral("A"), QStringLiteral("a"),
            QStringLiteral("B"), QVariant()}, &error);

// 绑定时间类型
db.execute("INSERT INTO qtmysql_demo (name, note, created_at) VALUES (?, ?, ?)",
           {QStringLiteral("王五"), QVariant(), QDateTime::currentDateTime()}, &error);

// 循环批量插入（建议放在事务里）
db.beginTransaction(&error);
for (int i = 0; i < 100; ++i) {
    if (!db.execute("INSERT INTO qtmysql_demo (name, note) VALUES (?, ?)",
                    {QStringLiteral("批量%1").arg(i), QStringLiteral("loop")}, &error)) {
        break;
    }
}
db.commitTransaction(&error);

// 主键冲突时更新（MySQL 8.0.19+ 行别名写法）
db.execute("INSERT INTO qtmysql_demo (id, name, note) VALUES (?, ?, ?) AS new "
           "ON DUPLICATE KEY UPDATE name = new.name, note = new.note",
           {1, QStringLiteral("测试用户"), QStringLiteral("覆盖备注")}, &error);

// 覆盖插入
db.execute("REPLACE INTO qtmysql_demo (id, name, note) VALUES (?, ?, ?)",
           {1, QStringLiteral("测试用户"), QStringLiteral("整行替换")}, &error);
```

## UPDATE

```cpp
// 单列更新
db.execute("UPDATE qtmysql_demo SET note = ? WHERE id = ?",
           {QStringLiteral("新备注"), 1}, &error);

// 多列 + 表达式
db.execute("UPDATE qtmysql_demo SET name = ?, note = CONCAT(note, ?) WHERE id = ?",
           {QStringLiteral("新名字"), QStringLiteral(" [追加]"), 1}, &error);

// CASE 条件批量更新
db.execute("UPDATE qtmysql_demo SET note = CASE "
           "WHEN id = ? THEN ? WHEN id = ? THEN ? ELSE note END "
           "WHERE id IN (?, ?)",
           {1, QStringLiteral("备注1"), 2, QStringLiteral("备注2"), 1, 2}, &error);

// 带 LIMIT（MySQL 支持）
db.execute("UPDATE qtmysql_demo SET note = ? ORDER BY id LIMIT ?",
           {QStringLiteral("只改前两行"), 2}, &error);
```

## DELETE

```cpp
// 条件删除
db.execute("DELETE FROM qtmysql_demo WHERE id = ?", {2}, &error);

// 多条件
db.execute("DELETE FROM qtmysql_demo WHERE name = ? AND note IS NULL",
           {QStringLiteral("无备注")}, &error);

// 带 LIMIT
db.execute("DELETE FROM qtmysql_demo WHERE name = ? ORDER BY id LIMIT ?",
           {QStringLiteral("批量0"), 1}, &error);

// 清空（危险）
db.execute("DELETE FROM qtmysql_demo", {}, &error);
```

`TRUNCATE TABLE` 也可用 `execute()`，但它是 DDL，会隐式提交，不能回滚。

## SELECT 常见写法

```cpp
qtmysql::QueryResult result;

// 无参数
db.select("SELECT id, name, note FROM qtmysql_demo ORDER BY id", &result, {}, &error);

// WHERE / LIKE / LIMIT 都可以用 ?
db.select("SELECT id, name FROM qtmysql_demo "
          "WHERE name LIKE ? ORDER BY id DESC LIMIT ?",
          &result, {QStringLiteral("%测试%"), 10}, &error);

// 单值
qtmysql::QueryResult countResult;
db.select("SELECT COUNT(*) AS total FROM qtmysql_demo", &countResult);
const qlonglong total = countResult.value(0, "total").toLongLong();

// 聚合 + 分组
db.select("SELECT name, COUNT(*) AS cnt FROM qtmysql_demo "
          "GROUP BY name HAVING cnt > ? ORDER BY cnt DESC",
          &result, {1}, &error);

// NULL 判断
db.select("SELECT id, note FROM qtmysql_demo WHERE note IS NULL", &result);
for (const QVariantMap &row : result.rows()) {
    if (row.value("note").isNull()) {
        // ...
    }
}
```

常用类型映射：

| MySQL 类型 | QVariant 类型 |
|---|---|
| 整数 | `int` / `uint` / `qlonglong` / `qulonglong` |
| `VARCHAR` / `TEXT` | `QString` |
| `TIMESTAMP` / `DATETIME` | `QDateTime` |
| `BLOB` | `QByteArray` |
| `NULL` | `QVariant::isNull() == true` |

## 事务

```cpp
if (!db.beginTransaction(&error)) {
    // 开启失败
}

const bool ok =
    db.execute("INSERT INTO qtmysql_demo (name) VALUES (?)", {QStringLiteral("事务A")}, &error) &&
    db.execute("UPDATE qtmysql_demo SET note = ? WHERE name = ?",
               {QStringLiteral("已处理"), QStringLiteral("事务A")}, &error) &&
    db.execute("DELETE FROM qtmysql_demo WHERE name = ?", {QStringLiteral("批量2")}, &error);

if (ok && db.commitTransaction(&error)) {
    // 提交成功
} else {
    db.rollbackTransaction(&error);
}
```

## 自增 ID 与影响行数

当前接口没有 `lastInsertId()` / `numRowsAffected()`，用 SQL 函数取：

```cpp
// INSERT 之后取自增 ID
qtmysql::QueryResult idResult;
if (db.select("SELECT LAST_INSERT_ID() AS id", &idResult) && !idResult.isEmpty()) {
    const quint64 newId = idResult.value(0, "id").toULongLong();
}

// UPDATE / DELETE 之后取影响行数
qtmysql::QueryResult affected;
db.select("SELECT ROW_COUNT() AS affected_rows", &affected);
const qlonglong changed = affected.value(0, 0).toLongLong();
```

`ROW_COUNT()` 返回的是实际改变的行数，值没变化时不计入。

## 错误处理

每个执行接口都通过 `QString *error` 返回错误；也可以稍后读 `lastError()`：

```cpp
if (!db.execute(sql, params, &error)) {
    qCritical().noquote() << "SQL 失败:" << error;
    qCritical().noquote() << "最后错误:" << db.lastError();
}
```

错误文本包含驱动错误与数据库错误，常见如：

- `QMYSQL: Unable to connect: Can't connect to MySQL server ...`
- `Duplicate entry ... for key ...`
- `Table ... doesn't exist`

## 限制

- 仅支持 `?` 位置占位符，不支持命名占位符。
- 每次调用重新 prepare，没有可复用的语句句柄。
- 一次只能执行一条 SQL，不支持多结果集。
- 没有 `lastInsertId()` / `numRowsAffected()`，使用 SQL 函数代替。
- SELECT 结果全量缓冲，不适合一次拉取超大结果集。
- 一个 `MySqlDatabase` 对应一个连接，不要跨线程共享；多线程请各自创建实例。
