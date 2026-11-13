/**
 * @file DatabaseService.cpp
 * @brief DatabaseService 的实现。
 */

#include "DatabaseService.h"

#include <QDebug>

/**
 * @brief 获取进程内唯一实例。
 * @return DatabaseService 的引用。
 * @note C++11 起静态局部变量的初始化是线程安全的；实例方法本身不做同步，
 *       应在 GUI 线程使用。
 */
DatabaseService& DatabaseService::instance()
{
    static DatabaseService s_instance;
    return s_instance;
}

/**
 * @brief 建立应用级连接，已连接时直接复用。
 *
 * 当前连接已打开时立即返回 true；否则用固定连接名 app_main 新建
 * MySqlDatabase 并连接。连接失败时不会替换原有的 m_db，直接返回 false。
 * @param host 服务器地址。
 * @param port 端口。
 * @param database 默认数据库。
 * @param password 密码。
 * @param error 可选；仅失败时写入原因。
 * @return 连接可用返回 true。
 * @note 连接名固定为 app_main，便于调试时识别；同一个进程内不要再创建同名连接。
 */
bool DatabaseService::init(const QString& host, quint16 port, const QString& database,
                           const QString& password, QString* error)
{
    if (m_db && m_db->isOpen())
        return true;

    // 显式指定连接名，便于调试识别；单例只会建立这一个连接
    auto db = std::make_unique<qtmysql::MySqlDatabase>(QString("app_main"));

    QString localError;
    if (!db->connect(host, port, database, password, &localError)) {
        if (error)
            *error = localError;
        qCritical().noquote() << "数据库连接失败:" << localError;
        return false;
    }

    m_db = std::move(db);
    qInfo().noquote() << "数据库已连接:" << m_db->connectionName();
    return true;
}

/**
 * @brief 关闭并释放底层连接。
 *
 * 可重复调用；未初始化时什么也不做。调用后之前通过 db()
 * 取得的引用都会失效。
 */
void DatabaseService::shutdown()
{
    if (!m_db)
        return;
    m_db->close();
    m_db.reset();
}

/**
 * @brief 连接是否已初始化且处于打开状态。
 * @return 已建立可用连接返回 true。
 */
bool DatabaseService::isReady() const
{
    return m_db && m_db->isOpen();
}

/**
 * @brief 获取底层数据库对象。
 * @return 由 init() 建立的 MySqlDatabase 引用。
 * @pre 必须先成功调用 init()。
 * @note 仅限 GUI 线程使用；调试构建会断言前置条件，发布构建不检查，
 *       未初始化时调用属于未定义行为。
 */
qtmysql::MySqlDatabase& DatabaseService::db()
{
    Q_ASSERT_X(isReady(), "DatabaseService::db", "未初始化数据库连接");
    return *m_db;
}
