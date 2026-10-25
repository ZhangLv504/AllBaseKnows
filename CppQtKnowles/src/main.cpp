#include <iostream>


enum class LogLevel {
    Debug = 1,
    Info,
    Warning,
    Error = 10
};

enum class AuditLevel {
    Info,
    Warning
};

const char* logLevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Debug:
        return "Debug";
    case LogLevel::Info:
        return "Info";
    case LogLevel::Warning:
        return "Warning";
    case LogLevel::Error:
        return "Error";
    default:
        return "Unknown";
    }
}

int main()
{
    LogLevel log = LogLevel::Warning;
    std::cout << "Name: " << logLevelToString(log) << '\n';
    std::cout << "Number: " << static_cast<int>(log) << '\n';
    if (log == LogLevel::Error)
    {
        std::cout << "log == Error\n";
    }
    else
    {
        std::cout << "log != Error\n";
    }

    // LogLevel level = Warning;    // 没有写类型限定符
    // int number = level;          // 不能隐式转换成整数
    // bool same = (level == AuditLevel::Warning);  // 不同枚举类型不能比较

    return 0;
}