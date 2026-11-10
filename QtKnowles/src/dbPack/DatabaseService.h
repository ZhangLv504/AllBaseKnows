/**
 * @file DatabaseService.h
 * @brief 应用级 MySQL 连接的单例封装。
 */

#ifndef QTKNOWLES_DATABASESERVICE_H
#define QTKNOWLES_DATABASESERVICE_H

#include "../../publics/include/MySqlDatabase.h"

#include <QString>
#include <memory>

/**
 * @brief 应用级唯一的数据库连接。
 *
 * 进程内只建立一个连接，供各仓储层复用；通过 instance() 获取，不可复制。
 */
class DatabaseService
{
public:
    /**
     * @brief 获取进程内唯一的实例。
     * @return 单例引用。
     * @note 首次调用时构造；C++11 起静态局部变量初始化是线程安全的。
     */
    static DatabaseService& instance();

    /**
     * @brief 初始化数据库连接。
     *
     * 当前连接已打开时直接返回 true，不会重复建连；连接已关闭或尚未建立时
     * 重新连接。失败时不会替换已有的 m_db。
     * @param host 服务器地址。
     * @param port 端口。
     * @param database 默认数据库。
     * @param password 密码。
     * @param error 可选；仅失败时写入原因。
     * @return 连接可用返回 true。
     * @note 连接名固定为 app_main，便于调试识别，进程内不要再创建同名连接。
     */
    bool init(const QString& host, quint16 port, const QString& database,
              const QString& password, QString* error = nullptr);

    /**
     * @brief 在 main 结束前显式关闭连接。
     *
     * 避免 Qt 先退出后连接才析构，导致访问已销毁的全局状态。
     * 调用后之前通过 db() 取得的引用全部失效。
     */
    void shutdown();

    /**
     * @brief 连接是否已经初始化并可用。
     * @return 已成功初始化返回 true。
     */
    bool isReady() const;

    /**
     * @brief 获取底层数据库对象。
     * @return 由 init() 建立的连接。
     * @pre 应在 isReady() 为 true 时调用。
     * @note 仅限 GUI 线程使用。
     */
    qtmysql::MySqlDatabase& db();

    DatabaseService(const DatabaseService&) = delete;
    DatabaseService& operator=(const DatabaseService&) = delete;

private:
    /// 单例：构造与析构只允许本类调用。
    DatabaseService() = default;
    ~DatabaseService() = default;

    std::unique_ptr<qtmysql::MySqlDatabase> m_db; ///< 底层连接实例。
};

#endif //QTKNOWLES_DATABASESERVICE_H
