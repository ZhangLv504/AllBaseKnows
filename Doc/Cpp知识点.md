# C++ 知识点

## 1. 传统 `enum` 的定义、使用与隐式转换

- 技术标准：C++17
- 练习工程：`CppQtKnowles`
- 通关时间：2026-10-25 14:30

### 适用场景

`enum` 适合表示取值范围有限且含义明确的状态集合，例如设备状态、订单状态、日志级别和错误码。相比直接使用整数，它能让变量及判断条件具有清晰的业务含义。

### 定义与编号规则

```cpp
enum LogLevel {
    Debug = 1,
    Info,
    Warning,
    Error = 10
};
```

- 未显式指定值的第一个枚举成员默认从 `0` 开始。
- 后续未指定值的成员等于前一个成员的值加 `1`。
- 上例中 `Debug == 1`、`Info == 2`、`Warning == 3`、`Error == 10`。

### 基本使用

```cpp
LogLevel level = Warning;

if (level == Error) {
    std::cout << "error\n";
}

std::cout << level << '\n'; // 传统 enum 可隐式转换为整数，输出 3
```

### 转换为可读文本

```cpp
const char* logLevelToString(LogLevel level)
{
    switch (level) {
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
```

有返回值的枚举转换函数必须覆盖未知值，否则可能出现“并非所有控制路径都返回值”的编译警告，并在运行时产生未定义行为。

### 传统 `enum` 的局限

1. 枚举值会进入外围作用域，不同枚举中的同名成员可能发生名称冲突。
2. 枚举值能够隐式转换成整数，类型约束较弱。
3. 可以通过 `static_cast` 把任意整数转换为枚举，但结果不一定对应已定义成员，使用前需要校验。

### 通关验证

练习完成了枚举定义、自动编号、名称转换、整数输出和相等比较。CMake Debug 构建成功且无警告，程序输出如下：

```text
Name: Warning
Number: 3
log != Error
```

## 2. `enum class` 的作用域与类型安全

- 技术标准：C++17
- 练习工程：`CppQtKnowles`
- 通关时间：2026-10-25 19:52

### 定义与使用

```cpp
enum class LogLevel {
    Debug = 1,
    Info,
    Warning,
    Error = 10
};

LogLevel level = LogLevel::Warning;
```

`enum class` 的成员位于枚举类型自己的作用域中，使用时必须通过 `枚举类型::成员` 进行限定。

### 避免名称冲突

```cpp
enum class LogLevel {
    Info,
    Warning
};

enum class AuditLevel {
    Info,
    Warning
};
```

两个枚举都可以定义 `Info` 和 `Warning`，因为完整名称分别是 `LogLevel::Info` 和 `AuditLevel::Info`。

### 强类型约束

`enum class` 不会隐式转换成整数，必须显式转换：

```cpp
LogLevel level = LogLevel::Warning;
int number = static_cast<int>(level);
```

不同的枚举类型不能直接比较，即使成员名称和底层整数值相同：

```cpp
// 编译错误：LogLevel 和 AuditLevel 是不同类型
// bool same = (level == AuditLevel::Warning);
```

这些限制可以防止把不同业务概念的状态误用或混合比较。

### 常见错误

```cpp
// LogLevel level = Warning;   // 缺少 LogLevel:: 作用域限定
// int number = level;         // 不能隐式转换成整数
// level == AuditLevel::Warning; // 不同枚举类型不能比较
```

在 `switch` 中也需要使用完整成员名称，并为未知值提供兜底处理：

```cpp
switch (level) {
case LogLevel::Warning:
    return "Warning";
default:
    return "Unknown";
}
```

### 通关验证

练习验证了限定作用域、同名成员、显式整数转换、同类型比较以及未知值处理。CMake Debug 构建成功且无警告，程序输出如下：

```text
Name: Warning
Number: 3
log != Error
```

