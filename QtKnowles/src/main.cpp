#include "CodeFormat.h"
#include <iostream>
#include <QStringList>
#include <QDebug>
#include <QSqlDatabase>
#include <QApplication>

#if (WIN32)
#include <windows.h>
#endif

#include "dbPack/DatabaseService.h"
#include "MySqlDatabase.h"
#include "demo1/DisplayEmployeeInfo.h"
#include "demo2/DisplayEmployeeInfo2.h"

int main(int argc, char * argv[])
{
    // 设置输出代码页为 UTF-8
    // SetConsoleOutputCP(CP_UTF8);

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

    //// demo1
    // DisplayEmployeeInfo employeeInfo;
    // employeeInfo.setMinimumSize(800, 600);
    // employeeInfo.show();

    // demo2
    DisplayEmployeeInfo2 employeeInfo;
    employeeInfo.setMinimumSize(800, 600);
    employeeInfo.show();


    const int code = app.exec();
    DatabaseService::instance().shutdown();

    return code;
}
