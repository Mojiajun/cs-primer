# CMake 教程：如何写 CMakeLists.txt 文件
## Step1 基础
### 基本操作
```
|-- CMakeLists.txt
|-- tutorial.cc
```
CMakeLists.txt 由下面三行组成
```
cmake_minimum_required(VERSION 2.6)
project(Tutorial)
add_executable(Tutorial tutorial.cc)
```
cmake_minimum_required 是 CMake 的脚本命令，一般用于指定编译本项目所需的 CMake 的最低版本，当 CMake 的版本号小于指定的最低版本时，CMake 将停止处理并报告错误。一般格式如下
```
cmake_minimum_required(VERSION major.minor[.patch[.tweak]])
```
project 是 CMake 的工程命令，用于指定项目的名字。变量 PROJECT_NAME 将保存设置的名字，顶层 CMakeLists.txt 中的 CMAKE_PROJECT_NAME 也会保存项目名称。此外，project 也可以指定支持的语言。一般的格式如下
```
project(<PROJECT-NAME> [<language-name>...])
```
project 命令也会设置如下变量
- PROJECT_SOURCE_DIR, \<PROJECT-NAME>_SOURCE_DIR
- PROJECT_BINARY_DIR, \<PROJECT-NAME>_BINARY_DIR

add_executable 是 CMake 的工程命令，通过源文件的方式将可执行文件添加到项目。一般格式如下
```
add_executable(<name> [WIN32] [MACOSX_BUNDLE]
               [EXCLUDE_FROM_ALL]
               [source1] [source2 ...])
```
添加一个名为 \<name> 的可执行目标，该目标将从指定的源文件 [source1] [source2 ...]（如果使用 target_sources() 添加源文件，add_executable() 可以不指定源文件）创建。默认情况下，在 MakeFile 文件同级目录创建可执行文件

需要说明的是，CMakeLists.txt 中的命令可以是大写、小写或者大小写混用

### 添加版本信息
```
|-- CMakeLists.txt
|-- tutorial.cc
|-- TutorialConfig.h.in
```
有的时候，源代码需要编译时指定的版本号。虽然可以在源代码中专门进行此操作，但是在 CMakeLists.txt 中执行此操作可以提供更大的灵活性。

CMake 的 configure_file 脚本命令可以将某个文件复制到另一个位置并修改其内容，一般格式如下
```
configure_file(<intput> <output>
               [COPYONLY] [ESCAPE_QUOTES] [@ONLY]
               [NEWLINE_STYLE [UNIX|DOS|WIN32|LF|CRLF] ])
```
将 \<input> 复制到 \<output> 文件，并在输入文件中 @VAR@ 或 ${VAR} 内容替换为相应变量的值，如果未定义该变量，则为空字符串。此外，一些其他的输入形式可以是
```
#cmakedefine VAR ...
```
将被替换成（VAR 在 CMake 中被设置且用作条件判断不为假）
```
#define VAR ...
```
或者
```
/* #undef VAR */
```
... 内容可以省略，比如
```
#cmakedefine VAR
#cmakedefine01 VAR
```
如果 VAR 在 CMake 定义，则被替换为
```
#define VAR
#define VAR 1
```
否则，被替换为
```
/* #undef VAR */
#define VAR 0
```

CMakeLists.txt 可以用 set() 脚本命令指定普通变量、缓存变量或者环境变量的值，一般格式如下
```
set(<variable> <value>... [PARENT_SCOPE])
set(<veriable> <value>... CACHE <type> <docstring> [FORCE])
set(ENV{<variable>} [<value>])
```

此外 option() 脚本命令提供选择命令，一般格式如下
```
option(<variable> "<help_text>" [value])
```
[value] 的值一般是 ON 或 OFF，如果没有指定 [value] 的值，默认使用 OFF。在终端执行 camke 命令时可以提供选项值，比如 CMakeLists.txt 提供选型 FOO_ENABLE
```
option(FOO_ENABLE "Enable Foo" ON)
```
在终端可以指定 OFF 或者 ON
```
cmake -D FOO_ENABLE=OFF .
```

### 指定 C++ 标准（比如支持 C++ 11 新特性）
```
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
```

## Step2 添加函数库
将某些函数以库的方式提供，通过选项控制是否调用，工程如下
```
|-- math_functions
|   |-- CMakeLists.txt
|   |-- math_functions.h
|   |-- mysqrt.cc
|-- CMakeLists.txt
|-- TutorialConfig.h.in
|-- tutorial.cc
```
math_functions 目录下的 CMakeLists.txt 的内容只有一句
```
add_library(MathFunctiosns mysqrt.cc)
```
上面 add_library() 命令通过源码的方式将一个函数库添加到一共工程。一般格式如下
```
add_library(<name> [STATIC | SHARED | MODULE]
            [EXCLUDE_FROM_ALL]
            [source1] [source2 ...])
```
从源码 [source1] [source2 ...] 编译出一个名为 \<name> 的函数库，然后添加到工程，如果使用 target_sources() 命令可以省略源码列表。[STATIC | SHARED | MODULE] 用于指定库的类型，默认是 SHARED。

顶层 CMakeLists.txt 中需要将函数库的头文件目录包含进来，并且链接 math_functions 函数库。可以用 option() 命令供用户选择
```
option(USE_MYMATH "Use tutorial provided math implementation" ON)
```
添加和链接的操作如下
```
if(USE_MYMATH)
  add_subdirectory(math_functions)
  list(APPEND EXTRA_LIBS math_functions)
  list(APPEND EXTRA_INCLUDES ${PROJECT_SOURCE_DIR}/math_functions)
endif()

add_executable(tutorial-app tutorial.cc)
target_link_libraries(tutorial-app ${EXTRA_LIBS})
target_include_directories(tutorial-app PUBLIC
                           ${PROJECT_BINARY_DIR}
                           ${EXTRA_INCLUDES})
```
add_subdirectory() 命令用将一个子目录添加到工程中，只有添加到顶层 CMakeLists.txt，在编译工程的时候才会纳入编译。一般格式如下
```
add_subdirectory(source_dir [binary_dir] [EXCLUDE_FROM_ALL])
```
source_dir 指定源代码以及 CMakeLists.txt 所在的目录，可以相对路径（相对于当前目录）和绝对路径。binary_dir 用于指定放置输出文件的目录。

list() 可以操作当前 CMake 变量范围内的列表，类似于set() 命令，list() 命令在当前范围内创建新的变量值。一般格式如下
```
# Reading
list(LENGTH <list> <out-var>)
list(GET <list> <index> [<index> ...] <out-var>)
list(JOIN <list> <glue> <out-var>)
list(SUBLIST <list> <begin> <length> <out-var>)

# Search
list(FIND <list> <value> <out-var>)

# Modification
list(APPEND <list> [<element> ...])
list(FILTER <list> {INCUDE | EXCLUDE} REGEX <regex>)
list(INSERT <list> <index> [<element> ...])
list(POP_BACK <list> [<out-var> ...])
list(POP_FRONT <list> [<out-var> ...])
list(PREPEND <list> [<element> ...])
list(REMOVE_ITEM <list> <value>)
list(REMOVE_AT <list> <index> ...)
list(REMOVE_DUPLICATES <list>)
list(TRANSFROM <list> <ACTION> [...])

# Ordering
list(REVERSE <list>)
list(SORT <list> [...])
```

target_link_libraries() 命令用于链接或添加依赖，通用格式如下
```
target_link_libraries(<target> ... <item>... ...)
```
\<target> 必须是 add_executable() 或者 add_library() 命令创建的，\<item> 可以是
- add_library() 创建的库名
- 库的全路径，比如 `/usr/lib/libfoo.so`
- 等等

target_include_directories() 用于指定头文件目录，一般格式如下
```
target_include_directories(<target> [SYSTEM] [BEFORE]
    <INTERFACE|PUBLIC|PRIVATE> [items1 ...]
    [<INTERFACE|PUBLIC|PRIVATE> [items2 ...] ...])
```
\<target> 必须是 add_executable() 或者 add_library() 创建的名字

## Step3 安装和测试
```
|-- math_functions
|   |-- CMakeLists.txt
|   |-- math_functions.h
|   |-- mysqrt.cc
|-- CMakeLists.txt
|-- TutorialConfig.h.in
|-- tutorial.cc
```
### 安装
#### install targets
```
install(TARGETS targets... [EXPORT <export-name>]
        [[ARCHIVE|LIBRARY|RUNTIME|OBJECTS|FRAMEWORK|BUNDLE|
          PRIVATE_HEADER|PUBLIC_HEADER|RESOURCE]
         [DESTINATION <dir>]
         [PERMISSIONS permissions...]
         [COMPONENT <component>]
         [NAMELINK_COMPONENT <component>]
         [OPTIONAL] [EXCLUDE_FROM_ALL]
         [NAMELINK_ONLY|NAMELINK_SKIP]
        ] [...]
        [INCLUDE DESTINATION [<dir> ...]])
```
- DESTINATION：指定安装路径，如果是绝对路径，直接使用。如果是相对路径，相对的是 CMAKE_INSTALL_PREFIX 变量的值。CMAKE_INSTALL_PREFIX 默认值是 /usr/local，可以在编译的时通过 DESTDIR 指定值，比如
```
make DESTDIR=/home/john install
```
- PERMISSIONS：指定安装文件的权限，合法的权限值有 OWNER_READ、OWNER_WRITE、OWNER_EXECUTE、GROUP_READ、GROUP_WRITE、GROUP_EXECUTE、WORLD_READ、WORLD_WRITE、WORLD_EXECUTE、SETUID、SETGID
- CONFIGURATIONS：指定 DEBUG 还是 RELEASE，比如
```
install(TARGETS target
        CONFIGURATIONS Debug
        RUNTIME DESTINATION Debug/bin)
install(TARGETS target
        CONFIGURATIONS Release
        RUNTIME DESTINATION Release/bin)
```

#### install files
```
install(<FILES|PROGRAMS> files...
         TYPE <type> | DESTINATION <dir>
         [PERMISSIONS permissions...]
         [CONFIGURATIONS [Debug|Release|...]]
         [COMPONENT <component>]
         [RENAME <name>] [OPTIONAL] [EXECLUDE_FROM_ALL])
```

### 测试
首先打开测试功能，enable_testing() 命令是支持测试功能
```
enable_testing()
```
接下来是添加测试用例，add_test() 命令用于添加一个测试用例，一般格式如下
```
add_test(NAME <name> COMMAND <command> [<arg> ...]
         [CONFIGURATIONS <config> ...]
         [WORKING_DIRECTORY <dir>])
```
然后指定测试期望的输出，set_tests_properities() 命令可以指定一个或多个测试的期望输出，一般格式如下
```
set_tests_properities(test1 [test2 ...] PROPERTIES prop1 value1 prop2 value2)
```
CMake 支持很多 prop 选项，[详情](https://cmake.org/cmake/help/v3.15/manual/cmake-properties.7.html#test-properties)。现在介绍 PASS_REGULAR_EXPRESSION，表示正则表达式匹配

CMake 的 function() 命令支持定义函数，可以将常用的命令组合定义为一个函数。一般格式如下
```
function(<name> [<arg1> ...])
  <commands>
endfunction()
```
比如说，我们可以将定义一个 do_test() 函数，包装 add_test() 和 set_tests_properities() 命令
```
function(do_test target arg result)
  add_test(NAME Comp${arg} COMMAND ${target} ${arg})
  set_tests_properties(Comp${arg}
      PROPERTIES PASS_REGULAR_EXPRESSION ${result})
endfunction(do_test)
```
然后就可以用 do_test() 添加测试
```
do_test(Tutorial 4 "4 is 2")
```