#pragma once

#include <QtCore/qglobal.h>

#if defined(QTMYSQL_LIBRARY)
#  define QTMYSQL_EXPORT Q_DECL_EXPORT
#else
#  define QTMYSQL_EXPORT Q_DECL_IMPORT
#endif
