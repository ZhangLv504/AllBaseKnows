/**
* @file QtMySQLGlobal.h
 * @brief QtMySQL 的导出宏。
 */

#pragma once

#include <QtCore/qglobal.h>

/**
 * @def QTMYSQL_EXPORT
 * @brief 标记公开符号：构建 QtMySQL 时导出，使用库时导入。
 */
#if defined(QTMYSQL_LIBRARY)
#  define QTMYSQL_EXPORT Q_DECL_EXPORT
#else
#  define QTMYSQL_EXPORT Q_DECL_IMPORT
#endif
