# CMake 知识点

## 专题：CMake 构建基础

> 练习工程：复用现有工程的 `CMakeLists.txt`（优先 `QtKnowles`）
>
> 参考文件：`QtKnowles/CMakeLists.txt`、`CppKnowles/CMakeLists.txt`

---

## 知识点 01：`file()` 与 `target_include_directories()` 的区别与作用

> 状态：学习中 / 待通关
>
> 开始时间：2026-11-12

### 本轮目标

1. 分清“哪些文件参与构建”和“编译器去哪里找头文件”是两件不同的事。
2. 掌握 `file(GLOB/GLOB_RECURSE)` 的作用、求值时机和局限。
3. 掌握 `target_include_directories()` 的作用域，以及 `PRIVATE` / `PUBLIC` / `INTERFACE` 的含义。
4. 能解释 `QtKnowles/CMakeLists.txt` 中两条命令为什么都不可少。

### 一、核心区别：两类命令回答两个不同问题

| 命令 | 命令类别 | 回答的问题 | 产生的结果 | 生效阶段 |
| --- | --- | --- | --- | --- |
| `file(GLOB ...)` | 文件系统类命令 | 磁盘上有哪些文件要交给构建系统 | 一个保存路径列表的变量（如 `APP_SOURCES`） | configure 阶段求值，结果写入变量 |
| `target_include_directories(...)` | 目标属性类命令 | 编译这个 target 时 `#include` 去哪些目录找 | target 的包含目录属性，最终变成编译器的 `-I` / `/I` 参数 | configure 阶段设置，构建时进入每条编译命令 |

关键结论：

- `file()` 只负责“收集文件路径”，不会添加任何编译选项，也不会编译它收集到的文件。
- `target_include_directories()` 只负责“设置头文件搜索路径”，不会把任何 `.cpp` 加入构建。
- 两者不能互相替代。文件在构建列表里，不代表它的目录会自动进入包含路径；目录在包含路径里，也不代表其中的源文件会被编译。

`QtKnowles/CMakeLists.txt` 中的注释已经点明这一点：

```cmake
# `GLOB_RECURSE` 只回答「哪些文件参与构建」，
# `target_include_directories` 才回答「`#include xxx` 时到哪些目录下找 xxx」。
```

### 二、`file()` 的作用与边界

常用形式：

```cmake
file(GLOB <变量名> [CONFIGURE_DEPENDS] <匹配模式>...)
file(GLOB_RECURSE <变量名> [CONFIGURE_DEPENDS] <匹配模式>...)
```

- `GLOB`：只匹配指定目录当前层的文件。
- `GLOB_RECURSE`：递归匹配子目录。
- 结果保存在变量中，必须再交给 `add_executable()`、`add_library()` 或 `target_sources()` 使用：

```cmake
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.qrc"
)

add_executable(QtKnowles ${APP_SOURCES})
```

需要理解的行为：

1. **求值时机**：glob 在 configure 阶段执行。不加 `CONFIGURE_DEPENDS` 时，configure 之后新增的文件不会自动进入变量，必须手动重新运行 CMake。
2. **`CONFIGURE_DEPENDS`**：让构建系统在构建时检查匹配结果是否变化，必要时自动重新 configure。它会带来额外检查开销，也不是所有生成器在任何场景下都能保证完全可靠；大型项目更常见的做法是显式列出源文件。
3. **头文件不会被编译**：把 `*.h` 放进 `add_executable()` 的源文件列表，只是让 IDE 显示它们，并让 `AUTOMOC` 等工具扫描到它们；真正编译的仍然是 `.cpp`。
4. **构建目录可能被误收**：`GLOB_RECURSE` 的范围写得过宽时，可能把生成目录或临时文件收进来。
5. **configure 阶段读不到构建期才生成的文件**：如果某个源文件要由构建过程生成，`file()` 在 configure 阶段无法匹配到它，应改用 `configure_file()`、`add_custom_command()` 配合 `target_sources()`。

### 三、`target_include_directories()` 的作用与边界

常用形式：

```cmake
target_include_directories(<target> [SYSTEM] [BEFORE|AFTER]
    <PRIVATE|PUBLIC|INTERFACE> <目录>...
)
```

前提与结果：

- `<target>` 必须已经由 `add_executable()` 或 `add_library()` 创建，顺序不能颠倒。
- 这些目录最终变成编译参数：GCC/Clang 为 `-I<目录>`，MSVC 为 `/I<目录>`。
- 相对路径按当前 `CMakeLists.txt` 所在目录（`CMAKE_CURRENT_SOURCE_DIR`）解释。跨 `add_subdirectory()` 时容易与直觉不符，建议用 `${CMAKE_CURRENT_SOURCE_DIR}` 显式写绝对路径。

三种可见性：

| 关键字 | 当前 target 能否使用 | 链接本 target 的使用者能否继承 | 典型场景 |
| --- | --- | --- | --- |
| `PRIVATE` | 能 | 不能 | 可执行文件、只在本模块内部使用的头文件目录 |
| `PUBLIC` | 能 | 能 | 库的公开头文件目录，且库自身实现也要用 |
| `INTERFACE` | 不能 | 能 | header-only 库、只给使用者传递要求的接口 target |

`QtKnowles` 的用法：

```cmake
target_include_directories(QtKnowles PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dbPack"
    "${CMAKE_SOURCE_DIR}/publics/include"
)
```

它带来的直接效果：

- [main.cpp](file:///D:/AiProjs/TwoProjs/AllBaseKnows/QtKnowles/src/main.cpp#L7) 的 `#include "dbPack/DatabaseService.h"` 依赖 `src`。
- [main.cpp](file:///D:/AiProjs/TwoProjs/AllBaseKnows/QtKnowles/src/main.cpp#L8) 的 `#include "MySqlDatabase.h"` 依赖 `publics/include`。
- [DisplayEmployeeInfo.cpp](file:///D:/AiProjs/TwoProjs/AllBaseKnows/QtKnowles/src/demo1/DisplayEmployeeInfo.cpp#L4) 的 `#include "DatabaseService.h"` 依赖 `src/dbPack`。

可执行文件没有“使用者”可继承，因此只用 `PRIVATE`。`PUBLIC` / `INTERFACE` 主要出现在库上。

补充：项目还开启了：

```cmake
set(CMAKE_INCLUDE_CURRENT_DIR ON)
```

它会把定义 target 的当前源码目录和构建目录自动加入包含路径，所以 `src` 有时看似可以省略；但它不会加入 `src/dbPack` 这类子目录，跨目录 include 仍然必须显式设置。显式写出 `src` 的好处是不依赖这个全局变量，阅读时一目了然。

### 四、两者如何配合：一次完整的包含关系

以 `QtKnowles` 为例：

1. `file(GLOB_RECURSE ...)` 找到 `src/dbPack/DatabaseService.cpp`，它因此进入编译列表。
2. `DatabaseService.cpp` 同目录 `#include "DatabaseService.h"`。MSVC/GCC 对引号形式会先搜索“当前源文件所在目录”，所以这一条不依赖 `-I` 也能成功。
3. `DisplayEmployeeInfo.cpp` 在 `src/demo1` 目录，却要 `#include "DatabaseService.h"`。它们不在同一目录，必须靠 `src/dbPack` 进入包含路径。
4. `main.cpp` 要 `#include "MySqlDatabase.h"`。该文件在 `publics/include`，必须靠包含目录；只有 `DatabaseService.cpp` 参与编译并不会自动提供这个搜索路径。

因此，一个能编译的项目需要同时满足：

```text
源文件进入 target 的源列表  +  头文件所在目录进入 target 的包含路径
        file()                         target_include_directories()
```

另外，`DatabaseService.h` 目前用的是：

```cpp
#include "../../publics/include/MySqlDatabase.h"
```

这是用相对路径绕过包含目录的写法。它能编译，但一旦头文件或源文件的目录层级调整就会断，而且把工程目录结构写死进了头文件。更稳妥的做法是让 `publics/include` 进入包含路径，然后写：

```cpp
#include "MySqlDatabase.h"
```

### 五、企业场景示例

- **多模块项目**：每个模块一个 target。模块自己的 `.cpp` 通过 `file()` 或显式列表收集；模块的公开头文件目录用 `PUBLIC`，私有实现目录用 `PRIVATE`，依赖方链接后自动获得公开包含路径，不需要每个使用方重复写 `-I`。
- **第三方库**：第三方头文件目录通过 `target_include_directories(<target> SYSTEM PRIVATE ...)` 引入，可抑制第三方头文件产生的编译警告。
- **header-only 库**：建立 `INTERFACE` 库，只用 `target_include_directories(lib INTERFACE include)` 传递包含路径，没有源文件需要编译。
- **可追踪的构建清单**：大型项目更倾向显式列出源文件，新增文件在代码评审中可见；`file(GLOB)` 适合目录结构稳定、文件增删频繁的原型或内部工具。
- **导出与安装**：库的 `PUBLIC` / `INTERFACE` 包含目录会写入导出配置，安装后使用方 `find_package()` 也能得到正确的头文件路径；这是全局 `include_directories()` 做不到的。

### 六、反例与常见坑

1. **以为 glob 了头文件就会编译或安装头文件**：头文件只在目标源列表中用于 IDE 展示和工具扫描；安装头文件需要 `install(FILES ...)`。
2. **以为源文件所在目录自动进入包含路径**：CMake 不会为源文件列表中的每个目录自动加 `-I`。同目录引号 include 能成功，是编译器的搜索规则，不是 CMake 的保证。
3. **用了 `GLOB` 却不加 `CONFIGURE_DEPENDS`**：新增 `.cpp` 后没有重新 configure，表现为类找不到、符号未定义，而源文件明明存在。
4. **在 configure 阶段 glob 构建期生成的文件**：生成文件此时还不存在，必然漏掉；应使用 `add_custom_command()` 生成后再 `target_sources()`。
5. **给可执行文件写 `PUBLIC` / `INTERFACE`**：可执行文件通常不会被其他 target 链接，这些可见性没有实际意义，还会误导阅读者。用 `PRIVATE`。
6. **用全局 `include_directories()` 代替 target 级命令**：它会影响当前目录及其子目录中之后创建的所有 target，造成路径污染，也无法作为库的使用要求传递出去。
7. **在 `add_executable()` 之前调用 `target_include_directories()`**：target 尚未存在，CMake 直接报错。
8. **包含目录拼写错误**：configure 阶段通常不报错，直到编译时出现“找不到头文件”才暴露；跨平台还可能是大小写问题。
9. **在头文件里写跨目录相对路径**：如 `../../publics/include/MySqlDatabase.h`，目录层级一变就断；应通过包含目录使用逻辑路径。
10. **把 `file(GLOB_RECURSE)` 的范围写成整个工程根目录**：容易把 `build/`、生成文件、备份文件收进构建列表，导致重复符号或意外编译。

### 七、练习题

练习在 `QtKnowles` 工程中进行，本轮只做只读分析和本地构建验证，不修改现有 `CMakeLists.txt` 的交付配置，也不改动 Qt 业务代码。

1. 指出 [CMakeLists.txt](file:///D:/AiProjs/TwoProjs/AllBaseKnows/QtKnowles/CMakeLists.txt#L36) 中 `file(GLOB_RECURSE ...)` 和 [CMakeLists.txt](file:///D:/AiProjs/TwoProjs/AllBaseKnows/QtKnowles/CMakeLists.txt#L58) 中 `target_include_directories(...)` 各自决定什么，并说明为什么删掉任何一条都会出问题。
2. 假设从包含目录中删除 `src/dbPack`，第一个编译失败的文件是哪个？失败原因是什么？
3. 假设从包含目录中删除 `publics/include`，哪些文件会失败？为什么 `DatabaseService.h` 里的 `../../publics/include/MySqlDatabase.h` 不能救回 `main.cpp`？
4. 在 `src` 下新建 `demo2/EmployeeReport.cpp` 和 `demo2/EmployeeReport.h`：源文件是否会自动参与构建？如果 `main.cpp` 要 `#include "EmployeeReport.h"`，还需要改哪里？如果改成 `#include "demo2/EmployeeReport.h"` 呢？
5. 如果把 `PRIVATE` 改成 `PUBLIC`，这个可执行文件工程会有什么实际变化？为什么库工程才更需要 `PUBLIC` / `INTERFACE`？
6. 用一句话分别概括：`file()` 解决什么问题，`target_include_directories()` 解决什么问题。
