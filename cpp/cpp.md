# C++基础

## ch1 开始
- 函数定义
  - 四个部分
    - 返回类型
    - 函数名
    - 参数列表
    - 函数体
  - 函数原型： 函数名、参数类型、参数个数、顺序以及它所在的类和命名空间
- `endl`作用： 结束当前行，并将于设备关联的缓冲区中的内容刷新到设备中。

## ch2 变量和基本类型

- 整数的尺寸  
  一个int至少和一个short一样大，一个long至少和一个int一样大，一个long long至少和一个long一样大。
- 变量  
  变量提供一个具名的、可供程序操作的、具有某种数据类型的存储空间。数据类型决定着变量所占内存空间的大小和布局方式，该空间能存储的值的范围，以及变量能参与的运算。
- 初始化和赋值的差异  
  初始化不是赋值，初始化的含义是创建变量时赋予其一个初始值，而赋值的含义是把对象的当前值擦除，而以一个新值来替代。
- 声明语句  
  基本数据类型 + 声明符列表  `int &val = b;`
- 声明和定义的关系  
  声明规定变量的类型和名字，使得名字为程序所知；而定义还申请存储空间，负责创建与名字关联的实体，也可能会为变量赋予一个初始值。
- 指针和引用的区别
  - 指针保存的是指向对象的地址，引用相当于变量的别名  
  - 引用在定义的时候必须初始化，指针没有这个要求  
  - 指针可以改变地址，引用必须从一而终  
  - 不存在空应引用，但是存在空指针NULL，相对而言引用更加安全  
  - 引用的创建不会调用类的拷贝构造函数
- 符合类型的解读  
  **从右往左**阅读。离变量最近的符号对变量的类型有最直接的影响。声明符的其余部分用以确定其他含义。
- const对象的作用域  
  默认情况下，const对象尽在文件类有效。添加`extern`可使在其他文件也有效。
- 顶层const和底层const  
  底层const表示指针本身是个常量；底层const表示指针所指的对象是个常量。  
  更一般地，顶层const可以表示任意的对象是常量，底层const则与指针和引用等符合类型有关。
- constexpr  
  - 常量表达式：在编译过程就能得到计算结果的表达式
  - constexpr声明中如果定义了一个指针，限定符constexpr仅对指针有效  
  `constexpr int i = 42;` // i为整型常量  
  `constexpr const int *p = &i;` // p是常量指针，指向整型常量i  
  `int j = 1; constexpr int *pl = &j;` // p1是常量指针，指向整型j  
- auto  
  让编译器通过初始值来推算变量的类型  
  一般忽略底层const
- decltype类型指示符
  - 选择并返回操作数的类型
  - 如果表达式的内容是解引用操作，返回引用类型
  - 和表达式的形式密切相关，双层括号的结果永远是应用
- 理解复杂数组的声明  
  由内向外阅读  
  `int *(&array)[10] = pts`：按照由内向外的顺序，首先知道array是个引用，然后观察右边知道array引用的对象是一个大小为10的数组，最后左边知道，数组的类型是指向int的指针。

## ch4 表达式

- 优先级
  - 后置递增（递减）的优先级高于解引用的优先级 `*iter++`含义为`*(iter++)`
  - 解引用的优先级小于点运算的优先级 `(*p).size()`
- 类型转换
  - 隐式转换
    - 算术转换
    - 数组转换成指针
    - 算术或指针转换成布尔类型
    - 转换成常量（非const转换成const）
  - 显示转换  
    `cast-name<type>(expression)`
    - static_cast：任何具有明确定义的类型转换，只要不包含底层cosnt。
    - dynamic_cast：支持运行时类型识别。`if(Derived *dp = dynamic_cast<Derived*>(base)) {...}`
    - const_cast：只能改变运算对象的底层const。
    - reinterpret_cast：为运算对象的位模式提供底层次上的重新解释。
  
## ch5 语句

- 异常处理
  - throw表达式
  ```
  if(condition) {
    throw runtime_error("output info");
  }
  ```
  - try语句块
  ```
  try {
    program-statements;
  } catch(exception-declaration) {
    handler-statements;
  } catch(exception-decalarttion) {
    handler-statements;
  } // ...
  ```
  - 异常类
- 标准异常
  - `include<exception>`
  - `include<stdexcept>`
    - exception
    - runtime_error
    - range_error
    - overflow_error
    - underflow_error
    - logic_error
    - domain_error
    - invalid_argument
    - length_error
    - out_of_range
  - `include<type_info>`
    - bad_alloc

## ch6 函数

- 函数调用的工作：（1）用实参初始化函数对应的形参；（2）将控制权转移给被调函数。
- return的工作：（1）返回return语句中的值（如果有的话）；（2）将控制权从被调函数转移回主调函数。
- 函数三要素：返回类型、函数名、形参类型；（组成函数原型）
- const形参和实参
  - 当用实参初始化形参时会忽略顶层const（不能构成重载）
  - 可以用使用非常量初始化一个底层const形参，反之不行