#include <iostream>
#include <QStringList>
#include <QDebug>
#include <QSqlDatabase>
#include <QApplication>

#include "dbPack/DatabaseService.h"
#include "MySqlDatabase.h"
#include "demo1/DisplayEmployeeInfo.h"

int main(int argc, char * argv[])
{
    QApplication app (argc, argv) ;

    // 默认初始化数据库连接
    QString error;
    if (!DatabaseService::instance().init(
        QStringLiteral ( "127.0.0.1" ),
        3306 ,
        QStringLiteral ( "zl_study_db" ),
        QStringLiteral ( "504504" ), &error))
    {
        qCritical().noquote () << "启动失败，数据库不可用:" << error;
        return 1 ;
    }

    DisplayEmployeeInfo employeeInfo;
    employeeInfo.setMinimumSize(800, 600);
    employeeInfo.show();

    const int code = app.exec();
    DatabaseService::instance().shutdown();

    return code;
}
