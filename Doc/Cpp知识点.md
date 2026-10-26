# C++ 知识点

## 1. 传统 `enum` 的定义、使用与隐式转换

- 技术标准：C++17
- 练习工程：`CppQtKnowles`
- 通关时间：2026-10-25 14:30

### 本轮目标

- 理解枚举适合表达什么数据。
- 掌握传统 `enum` 的定义、赋值、比较和自动编号规则。
- 理解传统 `enum` 的隐式整数转换与作用域污染问题。
- 能够使用 `switch` 将枚举转换成可读文本，并覆盖未知值。

### 核心概念

当一个变量只能从有限的一组状态中取值时，枚举比没有语义的整数或字符串更清晰。例如设备只可能处于离线、启动、运行或故障状态：

```cpp
enum DeviceState {
    Offline,
    Starting,
    Running,
    Fault
};
```

`DeviceState` 是枚举类型，`Offline`、`Starting`、`Running` 和 `Fault` 是该类型的枚举成员。

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

### 企业场景示例

不要使用没有语义的整数表示订单状态：

```cpp
int status = 2;
```

调用者无法直接判断 `2` 的含义。可以改成：

```cpp
enum OrderStatus {
    Pending,
    Paid,
    Shipped,
    Cancelled
};

OrderStatus status = Shipped;

if (status == Shipped) {
    std::cout << "准备显示物流信息\n";
}
```

这样变量类型、赋值和判断条件都包含业务语义。

### 反例与常见坑

#### 1. 隐式转换会削弱类型约束

```cpp
DeviceState state = Running;
int number = state; // 合法，传统 enum 会隐式转换成整数
```

虽然使用方便，但调用者可能在并非真正需要整数的地方把业务状态当作普通数值处理。

#### 2. 枚举成员进入外围作用域

```cpp
enum DeviceState {
    Running
};

enum TaskState {
    Running // 编译错误：同一作用域内名称冲突
};
```

传统 `enum` 的成员不是封闭在枚举类型内部的，因此多个枚举容易出现成员重名。

#### 3. 整数不能直接赋给枚举

```cpp
DeviceState state = 1; // 编译错误
```

可以显式转换：

```cpp
DeviceState state = static_cast<DeviceState>(1);
```

但转换成功不代表整数一定对应已经定义的枚举成员，因此外部整数在转换前需要校验。

#### 4. 重复数值会降低可读性

```cpp
enum ResultCode {
    Success = 0,
    Ok = 0
};
```

语法允许多个成员使用同一个值，但日志、分支判断和反向转换会变得含糊，通常应避免。

### 练习题

在 `CppQtKnowles/src/main.cpp` 中完成：

```cpp
enum LogLevel {
    Debug = 1,
    Info,
    Warning,
    Error = 10
};
```

要求：

1. 写出 `Info`、`Warning` 和 `Error` 对应的整数值。
2. 实现 `const char* logLevelToString(LogLevel level)`，并覆盖未知值。
3. 创建值为 `Warning` 的变量，输出成员名称和整数值。
4. 判断该变量是否等于 `Error`，并输出判断结果。

预期输出：

```text
Name: Warning
Number: 3
log != Error
```

### 通关检查清单

- `enum` 适合表达什么类型的数据？
- 未显式指定值时，枚举成员如何编号？
- 为什么传统枚举能够直接赋值给 `int`？
- 为什么两个传统枚举可能发生成员名冲突？
- 为什么整数强制转换成枚举后仍然可能是非法业务值？
- 有返回值的枚举转换函数为什么必须覆盖未知值？

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

### 本轮目标

- 掌握 `enum class` 的定义和成员访问语法。
- 理解限定作用域如何避免成员名称冲突。
- 理解强类型约束如何阻止隐式整数转换和跨枚举比较。
- 能够使用 `switch` 处理 `enum class` 并显式取得整数值。

### 核心概念

`enum class` 是 C++11 引入的作用域枚举。它仍然表达一组有限状态，但成员保存在枚举类型自己的作用域中，并且不会自动退化成普通整数。

它主要解决传统 `enum` 的两个问题：

1. 枚举成员泄漏到外围作用域，容易重名。
2. 枚举值可以隐式转换为整数，类型约束较弱。

### 一、基本语法

```cpp
enum class DeviceState {
    Offline,
    Starting,
    Running,
    Fault
};
```

使用成员时必须写出枚举类型：

```cpp
DeviceState state = DeviceState::Running;
```

下面的写法不能通过编译：

```cpp
DeviceState state = Running; // 错误：缺少 DeviceState:: 限定
```

### 二、限定作用域避免名称冲突

传统 `enum` 无法在同一作用域中重复定义 `Running`：

```cpp
enum DeviceState {
    Running
};

enum TaskState {
    Running // 编译错误
};
```

改成 `enum class` 后可以正常共存：

```cpp
enum class DeviceState {
    Idle,
    Running
};

enum class TaskState {
    Idle,
    Running
};
```

完整名称分别是 `DeviceState::Running` 和 `TaskState::Running`，因此不会冲突。

### 三、禁止隐式转换成整数

`enum class` 不允许把枚举悄悄当作整数：

```cpp
LogLevel level = LogLevel::Warning;
int number = level; // 编译错误
```

需要明确表达转换意图：

```cpp
int number = static_cast<int>(level);
```

这种限制能让编译器发现许多把业务状态误当数值使用的问题。

### 四、不同枚举类型不能比较

```cpp
enum class DeviceState {
    Idle,
    Running
};

enum class TaskState {
    Idle,
    Running
};

DeviceState device = DeviceState::Running;
TaskState task = TaskState::Running;

// 编译错误：DeviceState 与 TaskState 是不同类型
// bool same = (device == task);
```

即使两个成员名称相同、底层整数也可能相同，它们仍然表达不同业务概念。同一种枚举类型可以正常比较：

```cpp
if (device == DeviceState::Running) {
    std::cout << "Device is running\n";
}
```

### 五、与 `switch` 配合

```cpp
const char* stateToString(DeviceState state)
{
    switch (state) {
    case DeviceState::Idle:
        return "Idle";
    case DeviceState::Running:
        return "Running";
    default:
        return "Unknown";
    }
}
```

`case` 后同样需要使用完整的 `枚举类型::成员` 名称。

### 企业场景示例

订单状态和支付状态都可能包含 `Pending`、`Success`、`Failed`：

```cpp
enum class OrderState {
    Pending,
    Success,
    Failed
};

enum class PaymentState {
    Pending,
    Success,
    Failed
};
```

调用代码的含义非常明确：

```cpp
OrderState order = OrderState::Pending;
PaymentState payment = PaymentState::Success;
```

如果误把订单状态与支付状态进行比较，编译器会直接拒绝：

```cpp
// 编译错误：不同的枚举类型
// bool same = (order == payment);
```

### 反例与常见坑

#### 1. 忘记作用域限定符

```cpp
LogLevel level = Warning; // 编译错误
```

应写成：

```cpp
LogLevel level = LogLevel::Warning;
```

#### 2. 直接输出 `enum class`

```cpp
std::cout << level; // 通常无法编译
```

应显式转换为整数，或者先转换成业务文本：

```cpp
std::cout << static_cast<int>(level);
std::cout << logLevelToString(level);
```

#### 3. 比较不同枚举类型

```cpp
// LogLevel 和 AuditLevel 不是同一类型
// bool same = (level == AuditLevel::Warning);
```

成员名称相同不代表类型相同。

#### 4. 忘记处理未知值

枚举转换函数仍应提供 `default` 或函数末尾的兜底返回，避免某些控制路径没有返回值。

### 练习题

在 `CppQtKnowles/src/main.cpp` 中定义：

```cpp
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
```

要求：

1. 实现 `logLevelToString(LogLevel level)` 并处理未知值。
2. 创建值为 `LogLevel::Warning` 的变量。
3. 输出名称，并通过 `static_cast<int>` 输出整数值。
4. 判断它是否等于 `LogLevel::Error`。
5. 保留下列错误代码的注释，并说明错误原因：

```cpp
// LogLevel level = Warning;
// int number = level;
// bool same = (level == AuditLevel::Warning);
```

预期输出：

```text
Name: Warning
Number: 3
log != Error
```

### 通关检查清单

- 为什么必须写 `LogLevel::Warning`？
- 为什么两个 `enum class` 可以拥有同名成员？
- 为什么不能直接执行 `int number = level`？
- 为什么 `LogLevel` 不能与 `AuditLevel` 比较？
- 如何取得 `enum class` 对应的整数？
- 在 `switch` 中为什么也要使用完整成员名称？

### 通关验证

练习验证了限定作用域、同名成员、显式整数转换、同类型比较以及未知值处理。CMake Debug 构建成功且无警告，程序输出如下：

```text
Name: Warning
Number: 3
log != Error
```

## 3. 枚举底层类型、显式转换与工程实践

- 技术标准：C++17
- 练习工程：`CppQtKnowles`
- 讲义记录时间：2026-10-25 20:53
- 通关时间：2026-10-25 21:16
- 当前状态：已通关

### 本轮目标

掌握三个问题：

- 如何指定枚举使用的底层整数类型。
- 如何显式取得枚举的整数值。
- 如何安全地把外部整数转换成枚举。

### 一、枚举的底层类型

枚举成员最终需要使用某种整数类型存储。

没有指定时，`enum class` 默认使用 `int`：

```cpp
enum class DeviceState {
    Offline,
    Running
};
```

大致相当于：

```cpp
enum class DeviceState : int {
    Offline,
    Running
};
```

可以使用冒号明确指定底层类型：

```cpp
#include <cstdint>

enum class PacketType : std::uint8_t {
    Login = 1,
    Data = 2,
    Logout = 10
};
```

这里表示：

- `PacketType` 使用无符号 8 位整数存储。
- 枚举值只能落在 `std::uint8_t` 能表示的范围内。
- 每个 `PacketType` 通常占用一个字节。

下面的定义不能通过编译：

```cpp
enum class PacketType : std::uint8_t {
    Invalid = 300 // 超出 uint8_t 的取值范围
};
```

### 二、为什么要指定底层类型

普通业务代码通常不必关注枚举占几个字节，但这些场景需要明确：

- 网络协议字段
- 二进制文件格式
- 嵌入式设备通信
- 数据库存储字段
- 跨模块或者跨语言接口
- 对象数量很大的内存敏感场景

例如协议规定消息类型占一个字节：

```text
1  = Login
2  = Data
10 = Logout
```

对应的枚举应该写成：

```cpp
enum class PacketType : std::uint8_t {
    Login = 1,
    Data = 2,
    Logout = 10
};
```

这里不仅要固定底层类型，还要固定每个成员的数值，不能依赖自动编号。

### 三、枚举转换成整数

最直接的方法是显式转换：

```cpp
PacketType type = PacketType::Data;

std::uint8_t raw =
    static_cast<std::uint8_t>(type);
```

更通用的 C++17 写法是使用 `std::underlying_type_t`：

```cpp
#include <type_traits>

using PacketValue = std::underlying_type_t<PacketType>;

PacketValue raw =
    static_cast<PacketValue>(PacketType::Data);
```

`PacketValue` 会自动变成枚举真实使用的底层类型。本例中就是 `std::uint8_t`。

还可以在编译期进行确认：

```cpp
static_assert(
    std::is_same_v<
        std::underlying_type_t<PacketType>,
        std::uint8_t
    >
);
```

如果底层类型不是 `std::uint8_t`，编译就会失败。

### 四、`uint8_t` 的输出陷阱

在很多实现中，`std::uint8_t` 实际上是字符类型，因此这样输出：

```cpp
std::uint8_t raw = 2;
std::cout << raw;
```

可能输出一个不可见字符，而不是数字 `2`。

应再次转换成整数：

```cpp
std::cout << static_cast<int>(raw);
```

注意这里有两次不同目的的转换：

```cpp
auto raw = static_cast<std::uint8_t>(PacketType::Data);
std::cout << static_cast<int>(raw);
```

第一步取得枚举的协议值，第二步只是为了以数字形式输出。

### 五、整数转换成枚举

语法上可以这样转换：

```cpp
std::uint8_t raw = 2;
PacketType type = static_cast<PacketType>(raw);
```

但强制转换只改变类型，不会验证它是不是已经定义的枚举成员：

```cpp
std::uint8_t raw = 99;
PacketType type = static_cast<PacketType>(raw);
```

转换仍然可能成功，但 `PacketType` 中没有数值为 `99` 的成员。

因此外部数据必须先校验：

```cpp
bool isValidPacketType(std::uint8_t raw)
{
    switch (raw) {
    case static_cast<std::uint8_t>(PacketType::Login):
    case static_cast<std::uint8_t>(PacketType::Data):
    case static_cast<std::uint8_t>(PacketType::Logout):
        return true;
    default:
        return false;
    }
}
```

使用时：

```cpp
std::uint8_t received = 10;

if (isValidPacketType(received)) {
    PacketType type = static_cast<PacketType>(received);
}
```

正确顺序是：

```text
收到整数 → 验证数值 → 转换成枚举 → 执行业务逻辑
```

### 企业场景示例

假设从网络收到一个字节：

```cpp
std::uint8_t received = receivePacketType();
```

错误做法：

```cpp
PacketType type = static_cast<PacketType>(received);
handlePacket(type);
```

这会把非法值直接传入业务层。

更安全的做法：

```cpp
if (!isValidPacketType(received)) {
    std::cout << "Invalid packet type: "
              << static_cast<int>(received)
              << '\n';
    return;
}

PacketType type = static_cast<PacketType>(received);
handlePacket(type);
```

这样外部脏数据会在系统边界被拦截。

### 常见坑

#### 1. 协议枚举依赖自动编号

```cpp
enum class PacketType : std::uint8_t {
    Login,
    Data,
    Logout
};
```

如果以后在中间添加一个成员，后续编号会变化，可能破坏协议兼容性。

协议值应该显式指定：

```cpp
enum class PacketType : std::uint8_t {
    Login = 1,
    Data = 2,
    Logout = 10
};
```

#### 2. 把强制转换当成校验

```cpp
auto type = static_cast<PacketType>(raw);
```

这只进行了类型转换，没有证明 `raw` 是合法业务值。

#### 3. 直接输出 `uint8_t`

```cpp
std::cout << raw;
```

可能按字符输出。应写成：

```cpp
std::cout << static_cast<int>(raw);
```

#### 4. 底层类型容量不足

```cpp
enum class ErrorCode : std::uint8_t {
    DatabaseError = 1000 // 超出范围
};
```

需要选择能覆盖所有值的类型，例如：

```cpp
enum class ErrorCode : std::uint16_t {
    DatabaseError = 1000
};
```

### 练习题

请修改 `CppQtKnowles/src/main.cpp`。

#### 1. 定义枚举

```cpp
enum class PacketType : std::uint8_t {
    Login = 1,
    Data = 2,
    Logout = 10
};
```

#### 2. 实现两个函数

```cpp
const char* packetTypeToString(PacketType type);

bool isValidPacketType(std::uint8_t raw);
```

要求：

- `packetTypeToString()` 必须处理未知值。
- `isValidPacketType()` 只允许 `1`、`2`、`10`。

#### 3. 在 `main()` 中完成验证

- 创建 `PacketType::Data`。
- 输出 `sizeof(PacketType)`。
- 使用 `std::underlying_type_t` 取得底层值并输出。
- 模拟收到合法值 `10`，校验后转换并输出 `Logout`。
- 模拟收到非法值 `99`，拒绝转换并输出错误信息。
- 添加 `static_assert`，确认底层类型是 `std::uint8_t`。

预期输出可以是：

```text
PacketType bytes: 1
Data raw value: 2
Received type: Logout
Rejected raw value: 99
```

### 通关检查清单

完成后应能回答：

- `enum class PacketType : std::uint8_t` 中冒号后的类型有什么作用？
- 为什么网络协议枚举应显式指定数值？
- `std::underlying_type_t` 能得到什么？
- 为什么输出 `uint8_t` 时要转成 `int`？
- 为什么 `static_cast<PacketType>(raw)` 不等于合法性校验？
- 外部整数转换成枚举的安全顺序是什么？

### 通关验证

练习代码位于 `CppQtKnowles/src/main.cpp`，完成了以下内容：

- 使用 `std::uint8_t` 固定 `PacketType` 的底层类型。
- 为协议成员显式指定 `1`、`2`、`10`。
- 使用 `std::underlying_type_t` 获取底层类型。
- 使用 `static_assert` 在编译期确认底层类型。
- 在整数转换成枚举前验证合法性。
- 正确拒绝未定义的协议值 `99`。
- 显式包含 `<cstdint>` 和 `<type_traits>`，不依赖间接包含。

CMake Debug 构建成功且无警告，程序退出码为 `0`，实际输出如下：

```text
1
2
Logout
接收到非法值
```

## 4. `push_back` 与 `emplace_back` 的选择规则（学习中/待通关）

- 技术标准：C++17
- 练习工程：`CppQtKnowles`
- 讲义记录时间：2026-10-25 22:57
- 当前状态：待完成练习和通关验证

### 本轮目标

- 理解 `push_back` 和 `emplace_back` 的参数语义。
- 能根据“已有对象”还是“构造参数”选择接口。
- 理解 `emplace_back` 并不必然比 `push_back` 更快。
- 掌握花括号初始化、C++17 聚合类型和资源所有权等边界情况。

### 核心结论

1. 已经有一个目标类型对象时，优先使用 `push_back`，语义最直接。
2. 手里只有构造函数参数、希望在容器内部直接构造对象时，使用 `emplace_back`。
3. `emplace_back(existingObject)` 仍然需要复制或移动，不会凭空消除开销。
4. 无类型的 `{...}` 无法供 `emplace_back` 的参数包推导；这类写法经常需要 `push_back({ ... })`。
5. 在 C++17 中，纯聚合类型不能通过多参数圆括号直接构造，`push_back({ ... })` 往往更方便。

### 一、两个接口分别接收什么

对 `std::vector<T>` 来说，`push_back` 接收一个已经形成的 `T` 对象：

```cpp
void push_back(const T& value); // 复制
void push_back(T&& value);      // 移动
```

`emplace_back` 接收创建 `T` 所需的构造参数，并在容器的存储位置直接构造 `T`：

```cpp
template<class... Args>
T& emplace_back(Args&&... args); // C++17 返回新元素的引用
```

### 二、已有对象时优先 `push_back`

```cpp
struct User {
    int id;
    std::string name;

    User(int idValue, std::string nameValue)
        : id(idValue), name(std::move(nameValue))
    {
    }
};

std::vector<User> users;
User alice{1, "Alice"};

users.push_back(alice);            // 复制 alice
users.push_back(std::move(alice)); // 移动 alice
```

下面两种 `emplace_back` 写法并不会避免复制或移动：

```cpp
users.emplace_back(alice);            // 仍然复制
users.emplace_back(std::move(alice)); // 仍然移动
```

因此已有对象时使用 `push_back` 更能直接表达“把这个对象放进去”。

### 三、只有构造参数时使用 `emplace_back`

```cpp
std::vector<User> users;
users.emplace_back(2, "Bob");
```

`vector` 会在自己的存储空间中调用：

```cpp
User(2, "Bob")
```

对应的 `push_back` 写法需要先形成临时对象：

```cpp
users.push_back(User{2, "Bob"});
```

它表达的是“先创建一个临时 `User`，再把临时对象移动进容器”。`emplace_back` 通常可以省掉这次临时对象的移动。

### 四、`explicit` 构造函数

```cpp
struct Token {
    explicit Token(int value)
        : value(value)
    {
    }

    int value;
};

std::vector<Token> tokens;
```

`emplace_back` 可以直接调用显式构造函数：

```cpp
tokens.emplace_back(42);
```

下面的 `push_back` 不能进行隐式转换：

```cpp
// tokens.push_back(42); // 编译错误
```

需要先明确创建对象：

```cpp
tokens.push_back(Token{42});
```

### 五、花括号初始化：常见的 `push_back` 直接可用场景

```cpp
std::vector<std::vector<int>> matrix;

matrix.push_back({1, 2, 3}); // 正确
```

`push_back` 的参数类型已经确定为 `std::vector<int>`，因此编译器知道 `{1, 2, 3}` 应该构造成什么类型。

下面的写法通常无法编译：

```cpp
// matrix.emplace_back({1, 2, 3});
```

原因是裸花括号列表本身没有类型，模板参数包 `Args...` 无法从它推导参数类型。

可以显式给出类型：

```cpp
matrix.emplace_back(std::initializer_list<int>{4, 5, 6});
```

但在这个场景中，`push_back({1, 2, 3})` 往往更简洁、更自然。

### 六、C++17 聚合类型

```cpp
struct Point {
    int x;
    int y;
};

std::vector<Point> points;
points.push_back({1, 2}); // C++17 正确
```

在 C++17 中，下面的多参数写法不能用圆括号初始化纯聚合类型：

```cpp
// points.emplace_back(1, 2); // C++17 编译失败
```

可以写成：

```cpp
points.emplace_back(Point{1, 2});
```

但这又显式创建了临时 `Point`，不再具有“直接传构造参数”的优势。因此 C++17 的简单聚合对象通常直接使用：

```cpp
points.push_back({1, 2});
```

### 七、性能与重新分配

`emplace_back` 只可能省掉“先构造临时对象再移动”的步骤，并不会阻止 `vector` 扩容。

当容量不足时，`vector` 仍可能：

1. 分配更大的连续内存。
2. 将旧元素移动或复制到新内存。
3. 释放旧内存。

如果提前知道元素数量，真正影响更大的优化经常是：

```cpp
std::vector<User> users;
users.reserve(1000);
```

然后再根据参数形态选择 `push_back` 或 `emplace_back`。

### 企业场景示例

批量把数据库查询结果转换成领域对象：

```cpp
std::vector<User> users;
users.reserve(rows.size());

for (const auto& row : rows) {
    users.emplace_back(row.id, row.name);
}
```

这里手里只有 `User` 的构造参数，`emplace_back` 很合适。

如果业务层已经返回完整对象：

```cpp
User user = buildUser(row);
users.push_back(std::move(user));
```

此时使用 `push_back` 更清晰。

### 反例与常见坑

#### 1. 认为 `emplace_back` 永远不会复制或移动

```cpp
User user{1, "Alice"};
users.emplace_back(user); // 复制
```

参数是已有对象时，容器仍需从它构造一个新元素。

#### 2. 认为 `emplace_back` 永远更快

对于已有对象，`push_back(std::move(obj))` 与 `emplace_back(std::move(obj))` 通常执行相同的移动构造。性能差异需要基准测试，不能只根据函数名字判断。

#### 3. 用裸指针原地构造智能指针

```cpp
std::vector<std::unique_ptr<User>> users;
// 不推荐：users.emplace_back(new User{1, "Alice"});
```

更安全、意图更清晰的写法是先建立所有权：

```cpp
users.push_back(std::make_unique<User>(1, "Alice"));
```

#### 4. 忽略对象被移动后的状态

```cpp
User user{1, "Alice"};
users.push_back(std::move(user));
```

移动后 `user` 仍然有效，但内容只保证处于可析构、可重新赋值的有效状态，不应继续假设其 `name` 仍为 `"Alice"`。

### 选择规则

```text
已经有 T 对象
├─ 需要复制：push_back(obj)
└─ 不再需要原内容：push_back(std::move(obj))

只有构造 T 的参数
└─ emplace_back(args...)

使用裸花括号初始化
├─ push_back({ ... }) 能表达目标类型：优先使用
└─ emplace_back 需要显式补出参数类型

C++17 纯聚合类型
└─ 通常使用 push_back({ ... })
```

严格来说，很多“只能 `push_back`”的场景也可以写成 `emplace_back(T{...})`，但这只是把完整临时对象传给 `emplace_back`，没有得到原地构造优势。工程上应选择语义最清晰的接口，而不是机械地全部替换成 `emplace_back`。

### 练习题

在 `CppQtKnowles/src/main.cpp` 中完成以下练习：

1. 定义带有 `id`、`name` 和构造函数的 `User`。
2. 创建已有对象 `alice`，使用 `push_back(alice)` 插入副本。
3. 使用 `emplace_back(2, "Bob")` 原地构造对象。
4. 创建 `std::vector<std::vector<int>>`，使用 `push_back({1, 2, 3})` 插入一行。
5. 保留并注释掉 `matrix.emplace_back({4, 5, 6})`，说明它为什么不能推导。
6. 定义聚合类型 `Point { int x; int y; }`，在 C++17 中使用 `push_back({10, 20})` 插入。
7. 遍历并输出所有数据，证明插入结果正确。

### 通关检查清单

- `push_back` 和 `emplace_back` 分别接收什么？
- 已经有一个目标类型对象时为什么通常选 `push_back`？
- 哪种情况下 `emplace_back` 可能省掉一次临时对象移动？
- 为什么 `emplace_back(existingObject)` 仍可能复制？
- 为什么 `emplace_back({1, 2, 3})` 可能无法编译？
- 为什么 C++17 聚合类型常使用 `push_back({ ... })`？
- `emplace_back` 能否避免 `vector` 扩容搬迁旧元素？
- 为什么智能指针通常推荐 `push_back(std::make_unique<T>(...))`？

> 通关验证将在练习完成并通过编译、运行检查后补充。

