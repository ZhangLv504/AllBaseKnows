#pragma once

#include "QtMySQLGlobal.h"

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include <memory>

namespace qtmysql {

struct QTMYSQL_EXPORT ConnectOptions
{
    QString host = QStringLiteral("127.0.0.1");
    quint16 port = 3306;
    QString user;
    QString password;
    QString database;
    QString charset = QStringLiteral("utf8mb4");
    int connectTimeoutSeconds = 5;
};

class QTMYSQL_EXPORT QueryResult
{
public:
    QueryResult() = default;

    bool isValid() const;
    bool isEmpty() const;
    int rowCount() const;
    int columnCount() const;
    QStringList columnNames() const;

    QVariant value(int row, int column) const;
    QVariant value(int row, const QString &column) const;
    QVariantMap row(int index) const;
    QVector<QVariantMap> rows() const;

private:
    friend class MySqlDatabase;

    QStringList m_columns;
    QVector<QVector<QVariant>> m_rows;
    bool m_valid = false;
};

class QTMYSQL_EXPORT MySqlDatabase
{
public:
    explicit MySqlDatabase(const QString &connectionName = QString());
    ~MySqlDatabase();

    MySqlDatabase(const MySqlDatabase &) = delete;
    MySqlDatabase &operator=(const MySqlDatabase &) = delete;

    // 便捷连接：默认 root 用户、utf8mb4、5 秒连接超时
    bool connect(const QString &ip, quint16 port, const QString &database,
                 const QString &password, QString *error = nullptr);
    bool open(const ConnectOptions &options, QString *error = nullptr);
    void close();
    bool isOpen() const;

    bool execute(const QString &sql, const QVariantList &params = QVariantList(),
                 QString *error = nullptr);
    bool select(const QString &sql, QueryResult *result,
                const QVariantList &params = QVariantList(), QString *error = nullptr);

    bool beginTransaction(QString *error = nullptr);
    bool commitTransaction(QString *error = nullptr);
    bool rollbackTransaction(QString *error = nullptr);

    QString lastError() const;
    QString connectionName() const;

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace qtmysql
