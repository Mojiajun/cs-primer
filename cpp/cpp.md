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
    - const_cast：只能改变（加上或去掉）运算对象的底层const。在函数重载非常有用。
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
  return返回类型和函数返回类型不一致的时候，以函数返回类型为准。
- const形参和实参
  - 当用实参初始化形参时会忽略顶层const（不能构成重载）
  - 可以用使用非常量初始化一个底层const形参，反之不行
- 函数如何返回一个值  
  返回的值用于初始化调用点的一个临时对象，该临时对象就是函数调用的结果。
- 调用一个返回引用的函数得到左值，其他返回类型得到右值
- 函数重载  
  如果**同一作用域**（一个类就是一个作用域，具有继承关系的类是在不用的作用域）的几个函数名字相同但形参列表不同，称之为函数重载
- 隐藏  
  在内层作用域中声明的对象（包括函数），它将隐藏外层作用域中声明的同名实体。
- inline函数
  - 调用点内联展开而非进行函数调用
  - inline不是强制性，具体有编译器决定是否进行内联替换
  - inline表示程序中的函数可能有多个定义，但要求每个定义出现在不同的翻译单元中并且所有定义都相同。
  - 定义在class、struct、union的函数（包括friend函数）都是inline
  - constexpr函数是inline函数、
  - 通常放在头文件中
- constexpr函数  
  - 函数的返回类型及所有的形参类型都必须是字面值类型，而且函数体中有且只有一条return语句。
  - 默认为inline函数
  - 通常放在头文件中
- 函数匹配（函数查找优先于类型检查）
  - 第一步：选定重载函数集（候选函数）
  - 第二步：根据实参选择可行函数
  - 第三步：选择最佳匹配函数
- 实参类型转换（具体排序如下）
  - 精确匹配
  - 通过const转换实现匹配
  - 通过类型提升实现匹配
  - 通过算术类型转换或指针转换实现匹配
  - 通过类类型转换实现匹配

## ch7 类

- 类的基本思想：**数据抽象**和**封装**  
  数据抽象是一种一类与接口和实现分离的编程技术。类的接口包括用户能执行的操作；类的实现包括类的数据成员、负责接口实现的函数体以及定义类所需的各种私有函数。封装实现了类的接口是实现的分离。
- const成员函数
  - 默认情况下，this的类型是指向类类型非常量版本的常量指针（顶层const）
  - 紧跟在参数列表后面的const表示this是一个指向常量的常量指针（底层const+顶层const）
- 可变数据成员（mutable data member）  
  永远不会是const，即使他是const对象的成员
- 类内初始值必须使用=的初始化形式或者花括号括起来的直接初始化形式
- 友元的声明仅仅指定了访问的权限，而非一个通常意义的函数声明。类和非成员函数的声明必须在它们的友元声明之前。
- 类的定义（编译阶段）
  - 首先编译成员的声明
  - 直到类全部可见后才编译函数体
  - 只使用与成员函数中使用的名字。声明中使用的名字，包括返回类型或者参数列表中使用的名字，都必须在使用前确保可见。
- 类型名要特殊处理  
  在类中，如果成员使用了外层作用域中的某个名字，而该名字代表一种类型，则类不能再之后重新定义该名字
- 使用构造函数初始值
  - 如果没有在构造函数的初始值列表中显示地初始化成员，则该成员将**在构造函数体之前**执行默认初始化
  - 如果成员是const或者引用的话，必须将其初始化
  - 成员初始化顺序和它们在类定义中的出现顺序一致
  - 在构造函数体中是对成员赋值（已经初始化完成）
- 委托构造函数：使用它所属类的其他构造函数执行它自己的初始化过程（初始值用列表的形式）
- 类类型转换
  - 只接受一个参数的构造函数（转换构造函数），实际上定义了转换为此类类型的隐式转换机制
  - 只允许一步类类型转换
  - explicit可以阻止隐式类类型转换（只能在类内声明构造函数时使用，在类外定义是不能重复）
  - 用explicit关键字声明构造函数时，它将只能以直接初始化的形式使用，不能用于拷贝
- 聚合类
  - 所有成员都是public
  - 没有定义任何构造函数
  - 没有类内初始值
  - 没有基类，也没有virtual函数
- 字面值常量类
  - 数据成员都是字面值类型的聚合类是字面值常量类
  - 一个字面值常量类必须至少提供一个constexpr构造函数
- 类的静态成员
  - 声明前加static，只出现在类内部声明语句
  - 存在于任何对象之外，对象中不包含任何与静态数据成员有关的数据
  - 静态成员函数也不与任何对象绑定在一起，它们不包含this指针
  - （类外）使用作用域运算符直接访问静态成员
  - 通常不能再类内部初始化静态成员，必须在外部定义和初始化每个静态成员
  - 可以为constexpr类型的静态成员提供const整型类型的初始值

## ch8 IO库

|头文件|类型|
|:-:|:-|
|iostream|istream, wistream<br>ostream, wostream<br>iostream, wiostream|
|fstream|ifstream, wifstream<br>ofstream, wofstream<br>fstream, wfstream|
|sstream|istringstream, wistringstream<br>ostringstream, wostringstream<br>stringstream, wstringstream|

- IO对象无拷贝或赋值
- IO缓冲刷新
  - 程序正常结束
  - 缓冲区满
  - endl、flush、ends
  - 设置unitbuf
  - 关联流：当读写被关联流时，关联到的流缓冲区被刷新
  - cin或cerr都会导致cout缓冲区刷新
  - **程序崩溃，不会刷新**
- 文件IO
  - fstream对象被销毁时，close会自动被调用

## ch9 顺序容器

- 容器操作
  - 类型别名
    - |iterator
    - const_iterator
    - size_type
    - difference_type
    - value_type
    - reference
    - const_reference
  - 构造函数
    - C c;
    - C c1(c2);
    - C c(b, e);
    - C c{a, b, c...};
  - 赋值与swap
    - c1 = c2;
    - c1 = {a, b, c...};
    - a.swap(b);
    - swap(a, b)
  - 大小
    - c.size()
    - c.max_size()
    - c.empty()
  - 添加/删除元素
    - c.insert(args)
    - c.emplace(args)
    - c.erase(args)
    - c.clear()
  - 关系运算
    - ==, !=
    - <, <=, >, >=
  - 迭代器
    - c.begin(), c.end()
    - c.cbegin(), c.cend()
    - c.rbegin(), c.rend()
    - c.crbegin(), c.crend()
    - reverse_iterator, const_reverse_iterator
- 使用swap  
  swap交换两个相同类型容器的内容，除array外，swap不对任何原色进行拷贝、删除或插入操作，因此可以保证在常数时间内完成
- 容器元素是拷贝  
  当我们用一个对象来初始化容器时，或将一个对象插入到容器中时，实际上放入到容器中的是对象值的一个拷贝，而不是对象本身
- vector对象如何增长  
  如果没有空间容纳新元素，必须分配新的内存空间（两倍增长），将已有元素从久位置移动到新空间，然后添加新元素，释放内存空间

## ch10 泛型算法

- 迭代器令算法不依赖与容器，但算法依赖于元素类型的操作  
  泛型算法本身不会执行容器的操作，它们只会运行与迭代器之上，执行迭代器的操作
- lambda表达式
  - lambda表达式表示一个可调用的代码单元
  - 具有一个返回类型、一个参数列表和一个函数体。具体形式  
  `[capture list](parametr list) -> return type {function body }`
  - `capture list`（捕获列表）是一个所在函数中定义的局部变量的列表
  - 可以忽略参数列表和返回类型，但必须永远包含捕获列表和函数体
  - lambda只能使用那些明确指明的变量，可以直接使用定义在当前函数之外的名字
  - 类似参数传递，变量的捕获方式也可以是值或引用  
  ```
  void fcn() {
    size_t v1 = 42;
    auto f = [v1]() { return v1; } // 值捕获
    //...
  }

  void fcn2() {
    size_t v1 = 42;
    auto f = [&v1]() { return v1; } // 引用捕获
    //...
  }
  ```
  - 隐式捕获，在捕获列表中写一个&=。  
  &告诉编译器采用引用捕获方式，=则表示采用值捕获方式
  - 可变lambda  
  默认情况下，对于一个值拷贝的变量，lambda不会改变其值，在参数列表后加mutable可改变捕获变量的值  
  `auto f = [v1]() mutable { return ++v1; }`
  - 指定lambda返回类型  
  默认情况下，如果一个lambda体包含return之外的任何语句，则编译器假定此lambda返回void  
  必须使用尾置返回类型
- bind函数  
  接受一个可调用对象，生成一个新的可调用对象来“适应”原对象的参数列表，一般形式为：  
  `auto newCallable = bind(callable, arg_list);`
  
## ch11关联容器

- insert返回值  
  添加单一元素的insert和emplace版本返回一个pair，告诉我们插入操作是否成功。pair的first成员是一个迭代器，指向具有给定关键字的元素;second成员是一个bool值，指出元素是插入成功还是失败
- map的下表操作  
  有则返回，无则创建并初始化
- 无序容器  
  - 无序容器在存储上组织为一组桶，每个桶保存零个或多个元素
  - 使用一个哈希函数将元素映射到桶
  - 具有一个特定哈希值的所有元素都保存在相同的桶中

## ch12 动态内存

- 智能指针
  - shared_ptr：允许多个指针指向同一个对象
  - unique_ptr：“独占”所指向的对象
  - weak_ptr：伴随类，弱引用，指向shared_ptr所管理的对象
- make_shared  
  - `make_shared<T>(args)`使用arg初始化此对象
  - 在动态内存中分配一个对象并初始化它，返回指向此对象的shared_ptr
- shared_ptr
  - 引用计数：每个shared_ptr都会记录有多少个其他shared_ptr指向相同的对象
  - 当指向一个对象的最后一个shared_ptr被销毁时，shared_ptr类会自动销毁此对象
  - 释放对象后，自动释放相关联的内存
  - 可以用new返回的指针来初始化（构造函数为explicit，必须使用直接初始化形式）智能指针
  - reset可以将一个新的指针赋予一个shared_ptr对象 `shared_ptr<int> p; p.reset(new int(10));`  
  reset会更新引用计数，如果需要的话，会释放指向的对象
  - 可以定义自己的释放操作（初始化时传入）
- 智能指针陷阱
  - 不使用相同的内置指针初始化（或reset）多个智能指针
  - 不delete `get()`返回的指针
  - 不使用`get()`初始化或reset另一个智能指针
  - 如果你使用`get()`返回的指针，记住当最后一个对应的智能指针销毁后，你打指针变为无效了
  - 如果你使用智能指针管理的资源不是new分配的内存，基础传递给它一个删除器
- unique_ptr
  - 某个时刻只能有一个unique_ptr指向一个给定的对象
  - 没有对应的make_shared
  - 绑定到new返回的指针时，必须直接初始化形式
  - 不支持普通的拷贝或赋值操作
  - `release()`或`reset()`将指针的所有权从一个（非const）unique_ptr转移给另一个unique_ptr  
  release成员返回unique_ptr当前保存的指针并将其置为空
- weak_ptr
  - 指向一个shared_ptr管理的对象，不控制所指向对象生存期
  - 不会改变shared_ptr的引用计数
  - `lock()`检查weak_ptr指向的对象是否存在，若存在，返回一个指向共享对象的shared_ptr
  - `use_count()`返回指向共享对象的shared_ptr的数量
  - `expired()`若`use_count()`为0,返回true，否则返回false
- 动态数组
  - 申请：`Tp *p = new Tp[size]`（未初始化）或`Tp *p = new Tp[size]()`（初始化）
  - 释放：`delete [] p;`
  - 智能指针：`unique_ptr<Tp[]> up(new Tp[size])`
- allocator  
  将内存分配和对象构造分离
  - 定义：`allocator<T> a;`
  - 分配：`auto p = a.allocate(n);`
  - 释放：`a.deallocate(p, n);`
  - 构造：`a.construct(p, args);`
  - 析构：`a.destroy(p);`
  - 伴随算法，可以在未初始化内存中创建对象
    - `uninitialized_copy(b, e, b2)`：[b, e)->b2（[b, e)拷贝元素到迭代器b2指定的未构造的原始内存中）
    - `uninitialized_copy_n(b, n, b2)`：[b, b+n)->b2
    - `uninitialized_fill(b, e, t)`：t->[b, e)（拷贝t到迭代器[b, e)指定的未构造的原始内存中）
    - `uninitizlized_fill_n(b, n, t)`：t->[b, b+n)

## ch13 拷贝控制

- 拷贝构造函数
  - 如果一个构造函数的第一个参数是自身类类型的应用，且任何外参数都有默认值，则此构造函数是拷贝构造函数
  - 如果没有定义，编译器会为我们合成一个 