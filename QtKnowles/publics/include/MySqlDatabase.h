/**
 * @file MySqlDatabase.h
 * @brief QtSql + QMYSQL 的轻量封装：连接、执行、查询与事务。
 */

#pragma once

#include "QtMySQLGlobal.h"

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include <memory>

/**
 * @namespace qtmysql
 * @brief 本库的全部公开接口。
 */
namespace qtmysql {

/**
 * @brief 建立连接所需的参数。
 *
 * 未显式赋值的字段使用下面的默认值；密码默认为空字符串。
 */
struct QTMYSQL_EXPORT ConnectOptions
{
    QString host = QStringLiteral("127.0.0.1");  ///< 服务器地址，默认本机。
    quint16 port = 3306;                         ///< 端口，默认 3306。
    QString user;                                ///< 用户名，open() 要求非空。
    QString password;                            ///< 密码，可以为空。
    QString database;                            ///< 默认数据库；为空时连接后不切换库。
    QString charset = QStringLiteral("utf8mb4"); ///< 连接字符集，必须非空且只含字母、数字、下划线。
    int connectTimeoutSeconds = 5;               ///< 连接超时秒数；小于等于 0 时使用驱动默认值。
};

/**
 * @brief 一次性缓冲的查询结果集。
 *
 * select() 成功后由库填充。数据在 select() 返回时已全部读入内存，
 * 之后与连接状态无关；默认构造的对象 isValid() 为 false。
 */
class QTMYSQL_EXPORT QueryResult
{
public:
    QueryResult() = default;

    /**
     * @brief select() 是否成功执行并填充了结果。
     * @return 成功执行过 select() 返回 true（即使结果为零行）。
     */
    bool isValid() const;

    /**
     * @brief 是否没有数据行。
     * @return 零行返回 true；未成功执行过 select() 的对象也返回 true。
     */
    bool isEmpty() const;

    /**
     * @brief 数据行数。
     * @return 结果集中的行数。
     */
    int rowCount() const;

    /**
     * @brief 列数。
     * @return 结果集的列数。
     */
    int columnCount() const;

    /**
     * @brief 按查询顺序返回列名。
     * @return 与列号一一对应的列名列表。
     */
    QStringList columnNames() const;

    /**
     * @brief 按行号、列号取值。
     * @param row 行号，从 0 开始。
     * @param column 列号，从 0 开始。
     * @return 单元格值；行号或列号越界时返回无效 QVariant。
     */
    QVariant value(int row, int column) const;

    /**
     * @brief 按行号、列名取值。
     * @param row 行号，从 0 开始。
     * @param column 列名，匹配区分大小写。
     * @return 单元格值；行号越界或列名不存在时返回无效 QVariant。
     */
    QVariant value(int row, const QString &column) const;

    /**
     * @brief 把一行转换为“列名 -> 值”的映射。
     * @param index 行号，从 0 开始。
     * @return 该行数据；行号越界时返回空 QVariantMap，NULL 值保留为无效 QVariant。
     */
    QVariantMap row(int index) const;

    /**
     * @brief 把所有行转换为映射列表。
     * @return 每行一个 QVariantMap，顺序与结果集一致。
     * @note 每行都会分配一个 QVariantMap，大结果集请用 LIMIT 控制行数。
     */
    QVector<QVariantMap> rows() const;

private:
    friend class MySqlDatabase;

    QStringList m_columns;             ///< 列名。
    QVector<QVector<QVariant>> m_rows; ///< 行优先存储的单元格值。
    bool m_valid = false;              ///< select() 是否成功填充过结果。
};

/**
 * @brief 一个 MySQL 连接及其上的 SQL 操作。
 *
 * 每个实例独占一个 QSqlDatabase 连接，不可复制，也不要跨线程共享。
 * 析构时会自动关闭并移除连接。
 */
class QTMYSQL_EXPORT MySqlDatabase
{
public:
    /**
     * @brief 构造实例，此时尚未连接。
     * @param connectionName Qt 连接名；为空时自动生成 qtmysql_N。
     * @warning 连接名在进程内必须唯一；两个实例使用同一名称会互相顶掉已建立的连接。
     */
    explicit MySqlDatabase(const QString &connectionName = QString());
    ~MySqlDatabase();

    MySqlDatabase(const MySqlDatabase &) = delete;
    MySqlDatabase &operator=(const MySqlDatabase &) = delete;

    /**
     * @brief 便捷连接：使用 root 用户、utf8mb4 和 5 秒连接超时。
     * @param ip 服务器地址。
     * @param port 端口。
     * @param database 默认数据库。
     * @param password 密码。
     * @param error 可选；失败时写入原因，成功时清空。
     * @return 连接并设置字符集成功返回 true。
     * @note 等价于填好 ConnectOptions 后调用 open()，其中 user 固定为 root。
     */
    bool connect(const QString &ip, quint16 port, const QString &database,
                 const QString &password, QString *error = nullptr);

    /**
     * @brief 按完整参数建立连接。
     *
     * 会先关闭当前连接。host/user 为空、字符集名非法、驱动打开失败或
     * SET NAMES 失败都会返回 false，并把原因写入 error 和 lastError()。
     * @param options 连接参数。
     * @param error 可选；失败时写入原因，成功时清空。
     * @return 连接成功返回 true。
     */
    bool open(const ConnectOptions &options, QString *error = nullptr);

    /// @brief 关闭并移除连接；未连接时什么也不做。可重复调用。
    void close();

    /// @brief 连接是否处于打开状态。
    bool isOpen() const;

    /**
     * @brief 执行 INSERT、UPDATE、DELETE、DDL 等非查询语句。
     * @param sql 单条 SQL，占位符使用 ?。
     * @param params 按位置绑定的参数，从下标 0 开始。
     * @param error 可选；失败时写入原因，成功时清空。
     * @return 执行成功返回 true。
     * @note 一次只能执行一条语句，不取回结果集。
     */
    bool execute(const QString &sql, const QVariantList &params = QVariantList(),
                 QString *error = nullptr);

    /**
     * @brief 执行 SELECT 并把全部结果读入 QueryResult。
     * @param sql 单条 SQL，占位符使用 ?。
     * @param result 结果输出；传 nullptr 时查询仍会执行并读取数据，但不回传结果。
     * @param params 按位置绑定的参数，从下标 0 开始。
     * @param error 可选；失败时写入原因，成功时清空。
     * @return 执行并读取成功返回 true；此时 result->isValid() 为 true。
     * @note 结果全量缓冲，大表请用 LIMIT 分页；失败时不会改写 *result。
     */
    bool select(const QString &sql, QueryResult *result,
                const QVariantList &params = QVariantList(), QString *error = nullptr);

    /**
     * @brief 开启事务。
     * @param error 可选；失败时写入原因，成功时清空。
     * @return 成功返回 true。
     */
    bool beginTransaction(QString *error = nullptr);

    /// @brief 提交事务；参数与返回值同 beginTransaction()。
    bool commitTransaction(QString *error = nullptr);

    /// @brief 回滚事务；参数与返回值同 beginTransaction()。
    bool rollbackTransaction(QString *error = nullptr);

    /**
     * @brief 最近一次操作的错误文本。
     * @return open/execute/select 和事务操作开始时清空；失败时写入原因，成功时为空。
     */
    QString lastError() const;

    /// @brief 本次连接使用的 Qt 连接名；自动生成时形如 qtmysql_0。
    QString connectionName() const;

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace qtmysql
