#include <iostream>
#include <QStringList>
#include <QDebug>
#include <QSqlDatabase>

#include "MySqlDatabase.h"

namespace
{
    bool connectToDB()
    {
        // const QStringList drivers = QSqlDatabase::drivers();
        // qInfo().noquote() << "可用数据库驱动:" << drivers.join(QStringLiteral(", "));
        //
        // if (!drivers.contains(QStringLiteral("QMYSQL"))) {
        //     qCritical() << "QMYSQL 插件未加载，请检查 sqldrivers/qsqlmysql.dll 与 libmysql.dll 是否可被搜索到。";
        //     return false;
        // }

        qtmysql::MySqlDatabase database;
        QString error;
        if (!database.connect(QStringLiteral("127.0.0.1"), 3306, QStringLiteral("zl_study_db"),
                              QStringLiteral("504504"), &error)) {
            qCritical().noquote() << "连接失败:" << error;
            return false;
                              }
        qInfo().noquote() << "连接成功:" << database.connectionName();
        return true;
    }
}

int main()
{


    return 0;
}