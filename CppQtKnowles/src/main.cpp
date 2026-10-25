#include <iostream>

enum DeviceState {
    Offline,
    Starting,
    Running,
    Fault = 10
};

const char* stateToString(DeviceState state)
{
    switch (state) {
    case Offline:
        return "Offline";
    case Starting:
        return "Starting";
    case Running:
        return "Running";
    case Fault:
        return "Fault";
    default:
        return "Unknown";
    }
}

enum LogLevel {
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 10
};

const char* logLevelToString(LogLevel level)
{
    switch (level)
    {
    case Debug:
        return "Debug";
    case Info:
        return "Info";
    case Warning:
        return "Warning";
    case Error:
        return "Error";
    default:
        return "Unknown";
    }
}

int main()
{
    // DeviceState state = Starting;
    //
    // if (state == Starting) {
    //     std::cout << "Device is starting\n";
    // }
    //
    // std::cout << "Name: " << stateToString(state) << '\n';
    // std::cout << "Number: " << state << '\n';

    LogLevel log = Warning;
    std::cout << "Name: " << logLevelToString(log) << '\n';
    std::cout << "Number: " << log << '\n';
    if (log == Error)
    {
        std::cout << "log == Error\n";
    }
    else
    {
        std::cout << "log != Error\n";
    }

    return 0;
}