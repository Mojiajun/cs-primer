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
- 复合类型的解读  
  **从右往左**阅读。离变量最近的符号对变量的类型有最直接的影响。声明符的其余部分用以确定其它含义。
- const对象的作用域  
  默认情况下，const对象仅在文件类有效。添加`extern`可使在其他文件也有效。
- 顶层const和底层const  
  底层const表示指针本身是个常量；底层const表示指针所指的对象是个常量。  
  更一般地，顶层const可以表示任意的对象是常量，底层const则与指针和引用等复合类型有关。
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
  - 和表达式的形式密切相关，双层括号的结果永远是引用
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
  - constexpr函数是inline函数
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
  数据抽象是一种依赖于接口和实现分离的编程技术。类的接口包括用户能执行的操作；类的实现包括类的数据成员、负责接口实现的函数体以及定义类所需的各种私有函数。封装实现了类的接口和实现的分离。
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
  - （类外可以）使用作用域运算符直接访问静态成员，也可以使用对象访问
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
  swap交换两个相同类型容器的内容，除array外，swap不对任何元素进行拷贝、删除或插入操作，因此可以保证在常数时间内完成
- 容器元素是拷贝  
  当我们用一个对象来初始化容器时，或将一个对象插入到容器中时，实际上放入到容器中的是对象值的一个拷贝，而不是对象本身
- vector对象如何增长  
  如果没有空间容纳新元素，必须分配新的内存空间（两倍增长），将已有元素从旧位置移动到新空间，然后添加新元素，释放内存空间

## ch10 泛型算法

- 迭代器令算法不依赖于容器，但算法依赖于元素类型的操作  
  泛型算法本身不会执行容器的操作，它们只会运行于迭代器之上，执行迭代器的操作
- lambda表达式
  - lambda表达式表示一个可调用的代码单元
  - 具有一个返回类型、一个参数列表和一个函数体，一个捕获列表。具体形式  
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
  - 隐式捕获，在捕获列表中写一个&或=。  
  &告诉编译器采用引用捕获方式，=则表示采用值捕获方式
  - 可变lambda  
  默认情况下，对于一个值拷贝的变量，lambda不能改变其值，在参数列表后加mutable可改变捕获变量的值（本身的值不会改变）
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
- map的下标操作  
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
  - 引用计数：每个shared_ptr都会记录有多少个其它shared_ptr指向相同的对象
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
  - 如果你使用`get()`返回的指针，记住当最后一个对应的智能指针销毁后，你的指针变为无效了
  - 如果你使用智能指针管理的资源不是new分配的内存，记住传递给它一个删除器
- unique_ptr
  - 某个时刻只能有一个unique_ptr指向一个给定的对象
  - 没有对应的make_shared函数
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
  - 如果一个构造函数的第一个参数是**自身类类型的引用**，且任何外参数都有默认值，则此构造函数是拷贝构造函数
  - 如果没有定义，编译器会为我们合成一个
  - 编译器可以绕过拷贝构造函数，但拷贝/移动构造函数必须存在且是可访问的
  - 为什么第一个参数是引用？  
  在函数调用过程中，具有非引用类型的形参要进行拷贝初始化。所以拷贝构造函数是用来初始化非引用类类型参数。如果其参数不是引用类型，则会一直递归地调用下去，永远不会成功。
- 拷贝赋值运算符
  - 如果类未定义自己的拷贝赋值运算符，编译器会为它合成一个
  - 通常返回一个指向其左侧运算对象的引用（为了与内置类型的赋值保持一致）
  - 必须能够处理对象赋予它自身
  - 大多数组合类析构和拷贝构造函数的工作
  - 可以使用swap，将左侧运算对象与右侧对象的一个副本进行交换
- 析构函数
  - 释放对象使用的资源，并销毁对象的非static数据成员，按初始化顺序的逆序销毁
  - 没有参数和返回值，不能重载
  - 当一个类未定义析构函数时，编译器会为它定义一个合成析构函数
- =default  
  - 显示地要求编译器生成合成的版本
  - 如果一个类有数据成员不能默认构造、拷贝、复制或销毁，则对应的成员函数将被定义为删除的
- 阻止拷贝
  - 定义删除的函数
    - `NoCopy(const NoCopy&) = delete;`将拷贝构造函数和赋值运算符定义为**删除的函数**来阻止拷贝
    - =delete必须出现在函数第一次声明的时候
  - private拷贝控制
    - 将拷贝构造函数和赋值运算符声明为私有，但不定义它们
- 右值引用
  - 必须绑定到右值的引用。通过`&&`来获取右值引用
  - 只能绑定到一个将要销毁的对象（临时对象，没有其他用户）
  - 不能将一个右值引用直接绑定到左值上，也不能将左值引用绑定到右值（但是可以将一个const的左值引用绑定到右值）
    ```
    int i = 42;            
    int &r = i;             // 正确
    int &&rr = i;           // 错误
    int &r2 = i * 42;       // 错误
    const int &r3 = i * 42; // 正确
    int &&rr2 = i * 42;     // 正确
    ```
  - `std::move`获得绑定到左值上的右值引用（调用move意味着除了对对象赋值或者销毁外，不再使用它）
- 移动构造函数
  - 从给定对象“窃取”资源而不是拷贝资源
  - 第一个参数是一个右值引用
  - 必须确保移后源对象处于这样一个状态--销毁它是无害的
    ```
    StrVec::StrVec(StrVec &&s) noexcept
      : elements(s.elements), first_free(s.first_free), cap(s.cap) {
        s.elements = s.frist_free = s.cap = nullptr; // 销毁它是无害的
    }
    ```
  - 不分配任何新内存，通常不会抛出任何异常
- 移动赋值运算符
  - 必须正确处理自赋值
  - 移后源对象必须是可析构（析构后不会影响到移后的对象）
    ```
    StrVec &StrVec::operator=(StrVec &&rhs) noexcept{
      if(this != &rhs) {
        free();
        elements = rhs.elements;
        first_free = rhs.first_free;
        cap = rhs.cap;
        rhs.elements = rhs.frist_free = rhs.cap = nullptr; // 将rhs置于可析构状态
      }
      return *this;
    }
    ```
  - 只有当一个类没有定义任何自己版本的拷贝控制成员，且它的所有数据成员都能移动构造或移动赋值时，编译器才会为它合成移动构造函数或移动赋值运算符
  - 移动操作永远不会隐式定义为删除的函数。但是=default显示要求但不能移动所有成员，则编译器会将移动操作定义为删除的函数
  - 如果类定义了一个移动构造函数和/或一个移动赋值运算符，则该类的合成拷贝构造函数和拷贝赋值运算符会被定义为删除的
  - 移动右值。拷贝左值；如果没有移动构造函数，右值也被拷贝
- 三/五法则  
  所有五个拷贝控制成员应该看成一个整体：一般来说，如果一个类定义了任何一个拷贝操作，它就应该定义所有五个操作
  - 需要析构函数的类也需要拷贝和赋值操作（基类的虚析构函数除外）
  - 需要拷贝操作的类也需要赋值操作
  - 需要赋值操作的类也需要拷贝操作
- 移动迭代器
  - 通过改变给定迭代器的解引用运算符的行为来适配此迭代器
  - `std::make_move_iterator`函数将一个普通的迭代器转换为一个移动迭代器
- 右值引用和成员函数
  - 区分移动和拷贝的重载函数通常有一个版本接受一个`const T&`，而另一个版本接受一个`T&&`
  - 引用限定符
- 引用限定符
  - 放在参数列表后面
  - 强制左侧运算对象（即，this指向的对象）是一个左值
  - 可以是&和&&，分别指出this可以指向一个左值或右值
  - 只能用于（非static）成员函数，且必须同时出现在函数的声明和定义中
  - 可以同时用const和引用限定，引用限定必须跟随在const限定符后面
  - 引用限定符也可以区分重载版本
  - 如果我们定义两个或两个以上具有相同的名字或相同的参数列表的成员函数，就必须对所有函数都加上引用限定符，或者所有都不加

## ch14 重载运算与类型转换

- 基本概念
  - 重载运算符是具有特殊名字的函数：它们有关键字`operator`和其后要定义的运算符号共同组成
  - 参数数量与该运算符作用的运算对象数量一样多
  - 如果一个运算符函数是成员函数，则它的第一个（左侧）运算对象绑定到隐式的this指针上
  - 优先级和结合率与对应的内置运算符保持一致
  - 运算对象求值顺序的规则无法应用到重载的运算符号上
  - 不能被重载的运算符（`::`, `.*`, `,`, `? :`）
  - 通常情况下，不应该重载的运算符（`，`， `&`， `&&`， `||`）
- 选择作为成员或者非成员
  - `=`、`[]`、`()`、`->`必须是成员
  - 复合运算符一般来说应该是成员，但非必须
  - 改变对象状态的运算符或者与给定类型密切相关的运算符（`++`、`--`，`*`）通常是成员
  - 具有对称性的运算符通常作为普通函数
- 重载输出运算符`<<`
  - 通常第一个形参是`ostream &`，第二个形参是一个常量的引用
  - 返回`ostream &`
  - 尽量减少格式化操作
  - 必须是非成员函数（成员函数第一个参数默认为this）  
    `ostream &operator<<(ostream &os, const Tp &t);`
- 重载输入运算符`>>`
  - 通常第一个形参是将要读取的流的引用，第二个形参是将要读入到的（非常量）对象的引用
  - 返回某个给定流的引用
  - 必须是非成员函数（成员函数第一个参数默认为this）
  - 必须处理输入可能失败的情况
- 算术和关系运算符
  - 通常情况下，我们把算术和关系运算符定义成非成员函数以允许对左侧或右侧的运算对象进行转换
  - 返回局部变量的值
- 赋值运算符
  - 必须为成员函数
  - 复合赋值运算符不非得是类的成员，不过倾向于定义在类内
  - 返回左侧运算对象的引用
- 下标运算符
  - 必须是成员函数
  - 常以所访问元素的引用作为返回值
  - 最后同时定义常量版本和非常量版本
- 递增和递减运算符
  - 建议设定为成员函数
  - 同时定义前置版本和后置版本
  - 区分前置版本和后置版本，后置版本接受一个额外的（不被使用）int类型的形参  
    `StrBlobPtr &operator++();`,`StrBlobPtr operator++(int);`
  - 前置版本对象的引用，后置版本返回副本的值
  - 显式调用后置运算符`p.operator++(0);`
- 成员访问运算符`*`、`->`
  - 箭头运算符必须是类的成员，解引用运算符通常也是类的成员，但不是必须
  - `point->men`的执行过程
    - 第一步：如果point是指针，则引用内置的箭头运算符，表达式等价于`(*point).men`。首先解引用该指针，然后从所得的对象中获取指定的成员。如果point所指的类型没有名为mem的成员，程序会发生错误。
    - 第二布：如果point是定义了`operator->`的类的一个对象，则使用`point.operator->()`的结果获取mem。其中，如果该结果是一个指针，则执行第一步。如果该结果本身含有重载的`operator->()`，则重复调用当前步骤。最终，当这一过程结束时程序或者返回了所需的内容，或者返回一些表示程序错误的信息。
- 函数调用运算符
  - 必须是成员函数
  - 可以重载
- lambda是函数对象
  - 当我们编写了一个lambda后，编译器将该表达式翻译成一个**未命名类的未命名对象**
  - 含有一个重载的函数调用运算符
  - 默认情况下，函数调用运算符是const成员函数
  - 不含默认构造函数、赋值运算符及默认析构函数
- 调用形式
  - 调用形式指明了调用返回类型以及传递给调用的实参类型
- 标准库function类型
  - function的操作
    - `function<T> f;` T为调用形式
    - `function<T> f(nullptr);`空function对象
    - `function<T> f(obj);`
    - `f`作为条件
    - `f(args)`调用f中的对象
  - 可以把所有可调用对象，包括函数指针、lambda或者函数对象赋值给function，只要调用形式一样
    ```
    map<string, function<int(int, int)>> binops = {
      {"+", add},                                // 函数指针
      {"-", std::minus<int>()},                  // 标准库函数对象
      {"/", divide()},                           // 用户定义的函数对象
      {"*", [](int i, int j) { return i * j; }}, // 未命名的lambda对象
      {"%", mod}}                                // 命名的lambda对象
    ```
  - 不能（直接）将重载函数的名字存入function类型的对象（可转为存储函数指针）
- 类类型转换运算符  
  `operator type() const;`type为某种类型
  - 负责将一个类类型的值转换成其他类型
  - 必须是类的成员函数，通常是const
  - 不能声明返回类型
  - 隐式执行，无法传递实参，所以形参列表必须为空
  - 不允许转换成数组或者函数类型，但允许转换成指针（包括数组指针及函数指针）或引用类型
  - 加`explicit`显示转换（`static_cast<>`），编译器不会隐式转换。在下列位置，会被隐式执行
    - if、while及do语句的条件部分
    - for语句头的条件表达式
    - 逻辑非、或、与的运算对象
    - 条件运算符（`? :`）的条件表达式
  - 避免有二义性的类型转换（只存在唯一一种转换方式）
    - 不要令两个类执行相同的类型转换：如果Foo类有个接受Bar类对象的构造函数，则不要在Bar类中在定义转换目标为Foo类的类型转换运算符
    - 避免转换目标是内置算术类型的类型转换。特别是定义类转换成算术类型的转换类型的时候，接下来
      - 不要在定义接受算术类型的重载运算符
      - 不要定义转换到多种算术类型的类型转换
  
## ch15 面向对象程序设计

- OPP概述
  - 数据抽象：接口和实现分离
  - 继承
  - 动态绑定：运行时绑定（只能通过基类指针或引用才会发生）
- 定义基类
  - 析构函数通常为虚函数
  - virtual只能出现在类内部的声明语句之前而不能用于类外部的函数定义
  - 被用作基类的类必须是已经定义而非仅仅声明
  - 在类名后加`final`可以防止被继承
- 定义派生类
  - 使用类派生列表指明继承关系
  - 必须将继承而来的成员函数中需要覆盖的那些重新声明
  - 允许派生类显式地注明它使用某个成员函数覆盖类它继承的虚函数：在形参列表、或者在const成员函数的const关键字后面、或者在引用成员函数的引用限定符后面加`override`
  - 派生类必须使用基类的构造函数来初始化它的基类部分
  - 派生类可以访问基类的公有成员和受保护成员
  - 如果基类定义了一个静态成员，则在整个继承体系中只存在该成员的唯一定义
  - 静态成员遵循通用的访问控制规则
- 类类型转换与继承
  - 可以将基类的指针（智能指针也可以）或引用绑定到派生类对象上
  - 不存在从基类向派生类的隐式类型转换，在对象间不存在类型转换
  - 当用一个派生类对象为一个基类对象初始化或赋值时，只有该派生类对象中的基类部分会被拷贝、移动或赋值，它的派生类部分会被忽略掉
- 虚函数
  - 所有虚函数都必须有定义
  - 对虚函数的调用可能在运行时才被解析
  - 一旦某个函数被声明为虚函数，则在所有派生类中它都是虚函数
  - 派生类中的函数只有与基类虚函数的调用形式一样（有一个例外，返回对象本身的指针或应用）才构成覆盖（重写）
  - `override`关键字显式说明派生类覆盖继承于基类的一个虚函数（编译器会帮助检查是否覆盖**虚函数**，没有的话会报错，不是也会报错）
  - 使用作用域运算符可以回避虚函数的机制`double ans = base->Base::f();`
- 抽象基类
  - 纯虚函数
    - 在虚函数体的位置（声明语句的分号前）书写`=0`就可以将一个虚函数说明为纯虚函数
    - =0只能出现在类内部的虚函数声明语句处
    - 可以为纯虚函数提供定义，不过函数体必须定义在类外部
  - 含有纯虚函数的类是抽象基类
  - 不能实例化一个抽象基类
- 访问控制与继承
  - protected成员
    - 对于类的用户来说是不可访问的
    - 对于派生类和友元来说是可访问的
  - public、private和protected继承
    - 控制派生类用户（包括派生类的派生类在内）对于基类成员的访问权限
  - 友元与继承
    - 友元关系不能继承
  - 通过`using`声明改变个别成员的可访问性
    - 通过在类的内部使用`using`声明语句，可以将该类的直接过间接基类的任何可访问成员（可访问的，如非私有成员）标记出来
    - `using`声明语句中名字的访问权限有该`using`声明语句之前的访问说明符决定
  - 默认的继承保护级别
    - class定义的派生类是私有继承，struct定义的派生类是公有继承
- 继承中的类作用域
  - 每个类定义自己的作用域
  - 派生类的作用与嵌套在其基类的作用域**之内**
  - 派生类也能重用定义在其直接或间接基类的名字，此派生类的成员将隐藏同名（函数亦如此）的基类成员
  - 一如往常，名字查找优先于类型检查
    - 函数（同名即使参数列表不一样）隐藏（注意虚函数，形参列表不一样就会被隐藏）
    - 一旦名字找到，编译器不再继续查找
  - 通过基类调用隐藏的虚函数
- 虚析构函数
  - 在基类中将析构函数定义成虚函数确保多态时执行正确的析构函数版本
  - 虚析构函数将阻止合成移动操作
- 合成拷贝控制与继承
  - 对类本身的成员一次进行初始化、赋值或销毁的操作
  - 使用直接基类对应的操作（是可访问的并且不是一个被删除的函数）对直接基类部分进行初始化、赋值或销毁操作
  - 如果基类中的默认构造函数、拷贝构造函数、拷贝赋值运算符或析构函数是被删除的函数或者不可访问的，则派生类中对应的成员是被删除的
  - 如果在基类中有一个不可访问或删除掉的析构函数，则拍摄了还总合成的默认和拷贝构造函数是被删除的
  - 编译器不会自动合成一个删除掉的移动操作
- 派生类的拷贝控制成员
  - 通常使用对应的基类构造函数初始化对象的基类部分
  - 派生类的赋值运算符也必须**显式地**为基类部分赋值
  - 派生类析构函数只负责销毁有派生类自己分配的资源
  - 在构造函数和析构函数中调用虚函数：执行与构造函数或虚构函数所属类型相对应的虚函数版本
- 继承的构造函数
  - 派生类能够重用其直接基类定义的构造函数
  - using语句作用与构造函数，令编译器产生代码（using作用其他，只是让某个名字在当前作用域可见）
  - 一个构造函数的using声明不会改变该构造函数的访问级别

## ch16 模板与泛型编程

- 函数模板  
  ```
  template <typename T>
  int f(const T&) {}
  ```
  - 模板类型参数，像内置类型或类类型一样使用
    - 类型参数前必须加class或typename
  - 非类型模板参数
    - 表示一个值而非一个类型
    - 通过一个特定的类型名而非class或typename来指定非类型参数
    - 可以是一个整型或是一个指向对象或函数的指针或（左值）引用
    - 绑定早指针或引用非类型参数的实参必须具有静态的生存期
- 实例化函数模板
  - 当编译器遇到一个模板定义时，并不生成代码
  - 在调用是，实例化一个特定版本，编译器才会生成代码
  - 编译器用推断出的模板参数来实例化一个特定版本的函数
  - 大多数编译错误在实例化期间报告
- 类模板
  - 编译器不能为类模板推断模板参数类型
  - 使用是提供显式模板实参列表，编译器使用这些模板实参实例化出特定的类
  - 一个类模板的每个实例（类名和模板实参）都形成一个独立的类
  - 类模板的每个实例都有其自己版本的成员函数
  - 一个类模板的成员函数只有当程序用到它时才进行实例化（没有被使用，就不会实例化）
  - 在类模板自己的作用域，可以直接使用模板名而不提供实参
  - 在类外定义成员函数必须以关键字template开始，后接模板参数列表
    ```
    template<typename T>
    BlobPtr<T> BlobPtr<T>::operator++(int) { // 类名<模板参数>
      // 进入自己的作用域
      BlobPtr ret = *this;  // 类内可以直接使用类名
      ++*this;
      return ret;
    }
    ```
  - 当一个类包含一个友元声明，类与友元各自是否是模板是相互无关的
  - 类模板的static成员
    - 每个实例都有其自己的static成员实例
    - 必须有且只有一个定义
    - 可以通过类类型访问，也可以使用作用域访问
    - 和普通成员函数一样，static成员函数在使用的时候才会实例化
- 模板参数
  - 作用域
    - 可用范围是在其声明之后，至模板声明或定义结束之前
    - 遵循普通的作用域规则
    - 会隐藏外层作用域中声明的相同名字
    - 在模板内不能重用模板参数名
  - 模板声明必须包含模板参数，声明中的模板参数的名字不必与定义中相同
  - 使用类的类型成员
    - 默认情况下，C++语言假定通过作用域运算符访问的名字不是类型
    - 使用typename显式告诉编译器使用的是模板类型参数的类型成员（不能用class）
      ```
      template <typename T>
      typename T::value_type top(const T& c) {
        if(c.empty())
          return c.back();
        else
          return typename T::value_type(); 
      }
      ```
  - 默认模板实参
    - 可以为函数和类模板提供默认实参
    - 所有模板参数都有默认实参，<>也不能省略
- 成员模板
  - 不能是虚函数
  - 成员模板实参由编译器推断
- 控制实例化
  - 通过显式实例化避免模板有多个实例
  - 显式实例化形式  
    ```
    extern template declaration;  // 实例化声明
    template declaration;         // 实例化定义
    // 例如
    extern template class Blob<string>;           // 声明
    template int compare(const int&, const int&); // 定义
    ```
    当编译器遇到extern模板声明时，它不会在本文件中生成实例化代码。extern必须出现在任何使用此实例化的代码之前
  - 实例化定义会实例化所有成员
- 模板实参推断
  - 函数模板，编译器利用调用总的函数实参来确定其模板参数（这个过程称为模板实参推断）
  - 类型转换与模板类型参数
    - 将实参传递给带模板类型的函数形参时，能够自动应用的类型转换只有const转换及数组或函数到指针的转换
    - 使用相同模板参数类型的函数形参必须具有相同类型
    - 正常类型转换应用与普通函数实参，不进行特殊处理，正常转换
  - 函数模板显式实参
    - 在<>中给出，位于函数名后，实参列表之前
    - 从左至右的顺序与对于的模板参数匹配
  - 尾置返回类型与类型转换
    ```
    template<typename It>
    auto fcn(It beg, It end) -> decltype(*beg) { return *beg; }
    ```
  - 函数指针和实参推断
    - 函数模板初始化或赋值函数指针，编译器使用函数指针的类型来推断模板实参
    - 对每个模板参数，能唯一确定其类型或值（对于重载函数指针的情况，无法唯一确定）
    - 可以使用显式模板实参`func(compare<int>);`
  - 模板实参推断和引用
    - 从左值引用函数参数推断类型
      - 模板参数类型`T&`，只能传左值实参
      - 模板参数类型`const T&`，可以传递任何实参（`const &`可以绑定到右值）
    - 从右值引用（`T&&`）函数参数推断类型
      - 传递右值实参（其实可以传递任意实参类型，见引用折叠）
    - 引用折叠和右值引用参数
      - 引用折叠：间接创建（只能间接，如类型别名或模板参数，语法不支持直接创建）应用的引用，这些引用会形成“折叠”
        - `X& &`、`X& &&`、`X&& &`都会折叠成`X&`
        - `X&& &&`折叠成`X&&`
      - 如果一个函数参数是一个指向模板类型参数的右值引用（`T&&`），则它可以被绑定到一个左值
      - 如果（模板类型`X&&`）实参是一个左值，则推断出的模板实参类型将是一个左值引用，且函数参数将被实例化一个（普通）的左值引用参数（`X&`)
- 理解`std::move`  
  获得一个绑定到左值上的右值引用（本质上接受任何类型的实参）
  ```
  template <typename T>
  typename remove_reference<T>::type&& move(T&& t) {
    return static_cast<typename remove_reference<T>::type&&>(t);
  }
  ```
  - 传入右值，如`auto s2 = std::move(string("bye!"));`
    - 推断出T的类型为string
    - `remove_reference`用`string`实例化
    - `remove_reference<string>`的`type`成员是`string`
    - move的返回类型是`string&&`
    - move的函数参数t的类型为s`tring&&`
  - 传入左值，如`s1 = string("bye"); auto s2 = std::move(s1);`
    - 推断出T的类型为`string&`
    - `remove_reference`用`string&`实例化
    - `remove_reference<string&>`的`type`成员是`string`
    - move的返回类型是string&&
    - move的函数参数t的类型为`string& &&`，折叠成`string&`
    - `static_cast`将`string&`类型转换为`string&&`
- ***从一个左值`static_cast`到一个右值引用是允许的***
- 转发
  - 某些函数需要将某一个或多个实参连同类型（是否为const、是左值还是右值）不变地转发给其他函数
  - 如果一个函数参数是指向模板类型参数的右值引用，它对应的实参的const属性和左值/右值属性将得到保持
  - 在调用中使用`std::forward`保持类型信息
    - 必须通过显式模板实参来使用，返回该显式实参类型的右值引用
- 重载与模板
  - 名字相同的函数必须具有不同数量或类型的参数
- 可变参数模板
  - 就是一个接受可变数目参数的模板函数或模板类。可变数目的参数称为参数包
    - 模板参数包：零个或多个模板参数
    - 函数参数包：零个或多个函数参数
  - 用一个省略号指出一个模板参数或函数参数表示一个包
    ```
    // Args 是一个模板参数包;rest 是一个函数参数包
    // Args表示零个或多个模板类型参数
    // rest表示零个或多个函数参数
    template <typename T, typename... Args>
    void foo(cosnt T &t, const Args& ... rest);
    ```
  - `sizeof...`运算符可以知道包中有多少个元素
    ```
    template <typename ... Args>
    void g(Args ... args) {
      cout << sizeof...(Args) << endl;
      cout << sizeof...(args) << endl;
    }
    ```
  - 编写可变西安数函数模板
    ```
    template <typename T, typename... Args>
    ostream &print(ostream &os, const T &t, const Args&... rest) {  // 扩展Args
      os << t << ", ";           // 打印第一个实参
      return print(os, rest...); // 递归调用，打印其他实参; ...触发扩展rest
    }
    ```
  - 包扩展（...）
- 模板特例化  
  模板的一个独立的定义，在其中一个或多个模板参数被指定为特定的类型
  - 定义函数模板特例化
    - 必须为原模板中的每个模板参数都提供实参
    - `template <>`指出正在实例化一个模板
    - 本质是实例化一个模板，而非重载。特例化不影响函数匹配
    - 当一个非模板函数提供与模板函数同样好的匹配时，编译器会选择非模板版本
  - 类模板特例化
    - 不必为所有模板参数提供实参
    - 全特例化：`template<>`开始
    - 部分特例化（partial spicialization）
      - 本身是一个模板，首先定义模板参数
      - 部分特例化版本的名字与原版本的名字相同
      - 在类名后`<>`内，为要特例化的模板参数指定实参
      ```
      // 通用版本
      template <class T> struct remove_reference { typedef T type; };
      // 左值引用版本
      template <class T> struct remove_reference<T&> { typedef T type; };
      // 右值引用版本
      template <class T> struct remove_reference<T&&> { typedef T type; };
      ```
    - 特例化成员而不是类
      ```
      template <typename T> struct Foo {
        Foo(const T &t = T()): mem(t){}
        void Bar() { /*...*/}
        T mem;
      };
      template<>
      void Foo<int>::Bar() { /*...*/}
      ```
      用int之外的任何类型使用Foo，成员像往常一样实例化。用int使用Foo，Bar之外的成员向往常一样实例化，Bar会用特例化版本
      
## ch17 标准库特殊设施

- tuple
  - 成员类型可以不相同，任意数量（一旦确定，数目不再改变）
  - 定义和初始化
    ```
    // 没有初始化列表方式
    #include <tuple>
    tuple<T1, T2, ..., Tn> t;
    tuple<T1, T2, ..., Tn> t(v1, v2, ..., vn);
    auto t = make_tuple(v1, v2, ..., vn);
    ```
  - 访问tuple的成员
    ```
    get<i>(t);                         // 第i个值的引用（左值->左值引用，右值->右值引用）
    tuple_size<tupleType>::value;      // tuple的成员数量， tupleType可有`decltype`获取
    tuple_element<i, tupleType>::type; // tuple指定成员的类型
    ```
  - 关系运算符`==`、`!=`
- bitset类型  
  类似array，具有固定大小，定义时需要指定包含多少个二进制位
  - 定义和初始化
    ```
    #include <bitset>
    bitset<n> b;                        // n为全0
    bitset<n> b(u);                     // unsigned long long值u的低n位的拷贝（高位丢弃或高位补零）
    bitset<n> b(s, pos, m, zero, one);  // string s（只有‘1’或‘0’）从位置pos开始m个字符的拷贝
    bitset<n> b(cp, pos, m, zero, one); // cp指向字符数组
    ```
  - bitset的操作
- 随机数
  - 随机数库(#include <random>)
    - 随机数引擎类：生成随机数unsigned整数序列
      - 接受种子，构造时指定（未指定使用默认种子值）或`seed(int)`指定
      - 默认引擎（`default_random_engine`）
    - 随机数分类类：使用引擎返回服从特定概率分布的随机数
  - 分布类型和引擎
    - 某个范围的平均分布  
      ```
      uniform_int_distribution<unsigned> ui(0, 9); // 0～9 unisigned int
      uniform_real_distribution<double> ud(0, 1);  // 0～1 double
      default_random_engine e1;
      default_random_engine e2(SEED);  // 设种子
      e1.seed(SEED);                   // 设种子
      cout << u1(e1) << u2(e2);
      ```
    - 正态分布
      ```
      default_random_engin e;
      normal_distribution<> n(mean, std);  // 默认生成double值
      ```
    - bernoulli_distribution
      ```
      default_random_engine e;
      bernoulli_distribution b; // 没有参数
      cout << b(e);
      ```
- IO库再探
  - 格式化输入与输出
    - 控制布尔值的格式 `boolalpha`、`noboolalpha`
    - 指定整型的进制 `hex`、`oct`、`dec`将其改为十六进制、八进制或改回十进制
      - 十六进制默认小写，`uppercase`可改成大些，`nouppercase`改回
    - 在输出中指出进制 `showbase`、`noshowbase`
    - 打印精度 `precision`、`setprecision(int)`
    - 科学计数法 `scientific`
    - 打印小数点 `showpoint`、`noshowpoint`
    - 输出补白 `setw`、`left`、`right`、`setfill`
  - 未格式化的输入/输出操作
    - 单字节操作 `get`、`put`
    - 将字符放回输入流 `peek`、`unget`、`putback`
  - 流随机访问（g输入流，p输出流）
    - `tellg()`、`tellp()`                     // 当前位置
    - `seekg(pos)`、`seekp(pos)`               // 重定位，pos是绝对地址，tell返回
    - `seekp(off, from)`、`seekg(off, from)`   // 重定位，相对于from（`beg`、`cur`、`end`）

## ch18 用于大型程序的工具

- 当执行一个throw时，跟在throw后面的语句将不再执行。相反，程序的控制权从throw转移到与之匹配的catch模块
- 栈展开  
  如果对抛出异常的函数的调用位于一个try语句块内，则检查与该try块关联的catch子句。如果找到了匹配的catch，就使用该catch处理异常。否则，如果该try语句嵌套在其他try语句块中，则继续检查与外层try匹配的catch子句。如果仍然没有找到匹配的catch，则推出当前这个主调函数，继续在调用刚刚退出的这个函数的其他函数中寻找名，以此类推（沿着嵌套函数的调用链不断查找，知道找到了与异常匹配的catch子句为止，如果一直没找到匹配的catch，则退出主函数后查找过程终止）。
- 栈展开过程中的对象被自动销毁
  - 如果异常发生在构造函数中，要确保已构造的成员能被正确地销毁
- 析构函数和异常
  - 析构函数总是被执行，但是函数中负责释放资源的代码却可能被跳过
  - 析构函数在栈展开的过程中执行（不应该抛出异常，一旦抛出异常且析构函数自身没能捕获到该异常，则程序终止）
  - 析构函数中可能抛出异常的操作应该放在try语句块中，并且在自己内部得到处理
- 查找匹配的处理代码
  - 按照catch语句的出现顺序逐一进行匹配，选择第一个与异常匹配的catch语句（异常类继承链最底端放在前面）
  - 绝大多数类型转换都不被允许，要求精确匹配
    - 允许从非常量向常量的类型转换
    - 允许从派生类向基类的类型转换
    - 数组被转换成指针，函数被转换成指针
- 重新抛出
  - catch语句通过重新抛出将异常传递给另外一个catch语句
  - 用`throw;`，不包含任何表达式
  - 只有当catch异常声明是引用类型时，对参数所做的改变才会被保留并继续传播
- 捕获所有异常`catch(...) {/*....*/}`
- 函数try语句块
  ```
  template <typename T>
  Blob<T>::Blob(std::initializer_list<T> il) try :
      data(std::make_shared<std::vector<T>>(il)) {
    /*constructor*/
  } catch(const std::bad_alloc &e) { hand_out_of_memeory(e); }

  template <typename T>
  Blob<T>::~Blob() try  {
    / *deconstructor*/
  } catch(exception &e) { /*...*/}
  ```
  - 既能处理构造函数体（或析构函数体），也能处理构造函数的初始化过程（或析构函数的析构过程）的异常
  - 关键字出现在构造函数初始值列表的冒号以及表示构造函数体的化括号之前
- noexcept异常说明
  - 指定某个函数不会抛出异常
  - 关键词`noexcept`紧跟在函数的参数列表后面；**在成员函数中，`noexcept`需要跟在`const`以及引用限定符之后，而在`final`、`override`或虚函数的`=0`之前**
    ```
    void recoup(int) noexcept; // 不抛出异常
    void recoup(int) throw();  // 等价的声明（兼容性）
    ```
  - 违反异常说明
    - 一旦一个noexcept函数抛出了异常，程序就会调用terminate结束程序以确保遵守不在运行时抛出异常的承诺
  - 异常说明的实参
    - 可选实参，该实参必须能转换为bool类型，true代表不抛出异常
      ```
      void recoup(int) noexcept(true);   // 不抛出异常
      void alloc(int) noexcept(false);  // 可能爆出异常
      ```
  - noexcept运算符
    - 一元运算符，返回bool，用于表示给定的表达式是(true)否(false)抛出异常`noexcept(recoup(i));`
  - 异常说明与指针、虚函数和拷贝控制
    - 函数指针及该指针所指的函数必须具有一致的异常说明
    - 如果一个虚函数不抛出异常，则后续派生出来的虚函数也不能抛出异常；与之相反，虚函数抛出异常，派生类中不做要求（可以、不可以都行）
    - 合成拷贝控制成员时，如果调用的任意一个函数可能抛出异常，则合成的成员是`noexcept(false)`
- 异常类层次
  - exception仅仅定义了拷贝构造函数、拷贝赋值运算符、一个虚析构函数和一个名为waht的虚成员
  - what负责返回用于初始化异常对象的信息（异常类名字之类的）
- 命名空间
  - 定义
    ```
    namespace name{
      /*
      class, vairable, function, template, other namespace 
      */
    }
    ```
  - 每个命名空间都是一个作用域
  - 可以不是连续的（可以定义在几个不用的部分）  
    `namespace nsp{/*...*/}`：可能定义一个新的命名空间，也可能是为已经存在的命名空间添加一些成员
  - 模板特例化
    - 必须定义在原始模板所属的命名空间中 `namespace std { template <> struct hash<Blob>{/*...*/}; }`
  - 全局命名空间
    - 全局定义中的名字也就是定义在全局命名空间中
    - 所有程序都存在
    - `::member_name`
  - 内联命名空间 `inline namespace snp {/*...*/}`
    - 内联命名空间中的名字可以被外层命名空间直接使用
    - `inline`必须出现在第一次定义的地方
  - 未命名的命名空间
    - 未命名的命名空间中定义的变量拥有静态生命周期
    - 可以在给定的文件中不连续，但不能跨越多个文件（作用范围不会横跨多个不用的文件）
    - 定义在未命名的命名空间中的名字可以直接使用
  - 使用命名空间中的成员
    - 命名空间别名 `namespace other_name = origin_name;`
    - using声明
      - 一条using声明语句一次只能引入命名空间的一个成员`using std::vector;`
      - 声明的是一个名字，而非一个特定的函数
      - 外层作用域的同名实体将被隐藏，参数列表相同发生错误
      - 为引入的名字添加而外的重载实例，并最终扩充候选函数集的规模
    - using指示
      - 所有名字都可见`using namespace std;`
      - 可以出现在全局作用域、局部作用域和命名空间作用域，但不能出现在类的作用域中
      - 为引入的名字添加而外的重载实例，并最终扩充候选函数集的规模
      - 参数列表完全相同不会发生错误，使用是必须指定特定的版本
- 类、命名空间与作用域
  - 由内向外依次查找每个外层作用域
  - 实参相关的查找与类类型形参
    - 当给函数传递一个类类型的对象时，除了在常规的作用域查找外，还会查找实参类所属的命名空间
  - 友元声明与实参相关的查找
    - 一个另外声明的类或函数如果第一次出现在友元声明中，则我们认为它是最近的外层命名空间的成员
- 多重继承
  - 在某个给定的派生列表中，同一个基类只能出现一次
  - 派生类的构造函数只能负责初始化它的直接基类，不能从多个基类中继承相同的构造函数
  - 基类的构造顺序与派生列表中基类出现顺序保持一致，与初始化值列表中基类的顺序无关
  - 派生类的析构函数只负责本身分配的资源
  - 可以令某个可访问基类的指针或引用直接指向一个派生类
  - 查找过程在所有直接基类中**同时进行**，如果名字在多个基类中被找到，则使用该名字具有二义性
- 虚继承
  ```
  class Raccoon: public virtual ZooAnimal { /*...*/ }; // virtual 和 public的顺序无关
  class Bear: virtual public ZooAnimal { /*...*/ };
  class Panda: public Bear, public Raccoon, public Endangered { /*...*/ };
  ```
  - 令某个类作出声明，承诺cany共享它的基类
  - 派生类中只包含唯一一个共享的虚基类子对象
  - 虚派生只影响从指定了虚基类的派生类进一步派生出的类
  - 基类的成员可以被直接访问
  - 派生类首先初始化虚基类子部分，接下来按照直接基类在派生类派生列表的次序进行初始化
  - 析构顺序与构造顺序相反

## ch19 特殊工具与技术

- new/delete表达式（不能重载）的工作机制
  - new expression
    - 调用`operator new`（或`operator new[]`）分配一块足够大、原始的、未命名的内存空间
    - 编译器运行相应的构造函数构造这些对象
    - 返回一个指向该对象指针
  - delete expression
    - 调用析构函数
    - 调用`operator delete`（或`operator delete[]`）释放内存
- 重载new和delete
  - 重载全局`operator new` (`operator new[]`)、`operator delete` (`operator delete[]`)
  - （类类型）定位成员函数`operator new` (`operator new[]`)、`operator delete` (`operator delete[]`)
- operator new接口和operator delete接口
  ```
  void *operator new(size_t);
  void *operator new[](size_t);
  void operator delete(void*) noexcept;
  void operator delete[](void*)noexcept;
  // 这些版本承诺不会抛出异常
  void *operator new(size_t, nothrow_t&) noexcept;
  void *operator new[](size_t, nothrow_t&) noexcept;
  void operator delete(void*, nothrow_t&) noexcept;
  void operator delete[](void*, nothrow_t&)noexcept;
  ```
  - `operator new`和`operator new[]`返回类型必须是`void*`，第一个形参必须是`size_t`且不能有默认实参
  - `operator delete`和`operator delete[]`返回类型必须是`void`，第一个形参必须是`void*`
  - 如果为类成员函数，必须是static，但可以不加static关键字
- 定位new表达式（`placement new`， 可以重载）
  ```
  new (place_address) type;
  new (place_address) type(initializers)
  new (place_address) type[size];
  new (place_address) type[size]{braced initializer list};
  ```
  - 不分配任何内存，只是简单地返回指针实参
  - 传入的指针可以不是动态分配内存
- 运行时类型识别（run-time type identification, RTTI）
  - `typeid`运算符，返回表达式的类型（如果是指针，返回指针的类型而不是指针所指对象的类型）
  - `dynamic_cast`运算符，将基类的指针或引用安全地转换成派生类的指针或引用
    ```
    // type必须是个类类型
    dynamic_cast<type*>(e);  // e为一个有效的指针，转换失败，返回0
    dynamic_cast<type&>(e);  // e必须为一个左值，转换失败，抛出bad_alloc
    dynamic_cast<type&&>(e); // e不能是左值
    ```
- 枚举类型
  - 限定作用域的枚举类型
    - 关键自`enum class`或`enum struct`
    - 遵循作用域准则，域外不可访问
    - 枚举成员默认下是int类型
  - 不限定作用域的枚举类型
    - 关键字`enum`
    - 枚举成员的作用域与枚举类型本身的作用域相同
    - 没有默认类型
  - 指定enum的大小
    ```
    enum intValues: unsigned long long;
    enum class open_modes: unsinged long;
    enum class direction;
    ```
  - 可以前置声明，但前置声明和定义对该enum是否限定作用域、enum的大小保持一致
- 类成员指针
  - 指向类的非静态成员，知道使用成员指针时，才提供成员所属的对象
  - 数据成员指针
    ```
    const string Screen::*pdata = &Screen::contents; // 定义
    auto pdata1 = &Screen::contents;
    auto s = myScreen.*pdata;                        // 使用
    ```
  - 成员函数指针
    ```
    char (Screen::*pmf)() const; // (Screen::*pmf)括号不能少
    pmf = &Screen::get;
    char c1 = (pScreen->*pmf)();  // 使用
    char c2 = (myScreen.*pmf)();
    ```
  - function可以用成员函数指针生成一个可调用的对象
    ```
    function<bool (const string&)> fcn = &string::empty; // bool是返回类型，const string& 是形参类型
    if(fcn(*iter)) { /*...*/ }
    ```
- 嵌套类
  - 定义在一个类内部的类
  - 静态成员的定义位于外层类作用域之外
- union
  - 有多个数据成员，在任意时刻，只有一个数据成员可以有值
  - 不能含有引用类型的成员
  - 成员是公有的
  - 匿名union
    - 一旦定义了一个匿名类，编译器就自动为该union创建一个为命名的对象
    - union定义所在的作用域该union的成员可以被直接访问
- 局部类
  - 定义在某个函数的内部
  - 所有成员必须完整定义在类的内部
  - 不能有静态数据成员
  - 局部类只能访问外层作用域定义的类型名、静态变量以及枚举成员
- 位域
  ```
  typedef unsigned int Bit;
  class File {
    Bit mode: 2;
    Bit modified: 1;
    Bit prot_owner: 3;
    /*...*/
  };
  ```
  - 类可以将其（非静态）数据成员定义为位域，在一个位域中含有一定数量的二进制位
  - 取地址运算符不能作用与位域，因此，任何指针都无法指向类的位域
- volatile
  - 告诉编译器不应对这样的对象进行优化
  - 可以是const
  - 成员函数也可以是volatile，只有volatile对象可以调用
  - volatile对象只能赋值给volatile的指针
  - 不能使用合成的拷贝/移动构造函数及赋值运算符初始化volatile对象或从volatile对象赋值（必须自己定义）

## plus c++关键字

- auto
  - 自动存储周期说明符（c++11之前）
    - 自动存储周期的对象在块开始前创建，在块结束时销毁，除了显示声明为static、extern外，所有的局部变量都是auto类型的。
  - 自动类型说明符（c++11开始）
    - 对于变量，指定将从其初始化程序自动推导出正在声明的变量的类型。
    - 返回类型将从其返回语句中推断出来。
    ```
    auto a = 1 + 2;
    template<class T, class U>
    auto add(T t, U u) { return t + u; }
    ```
  - 带尾随返回类型的函数声明（c++11开始）
    ```
    // function returning a pointer to f0
    auto fp11() -> void(*)(const std::string&) {
      return f0;
    }
    ```
- const
  - 常量类型说明符
    - 类型为const限定的对象，**或const对象的non-mutable成员**。无法被修改。尝试直接执行此操作是编译时错误，并尝试间接执行此操作（例如，通过引用或指向非const类型的指针修改const对象）会导致未定义的行为。
    - 定义时指定初值
    - （成员）函数返回const，表示函数不能当做“左值”
  - 常量成员函数说明符
    - 可以使用const限定符声明非静态成员函数（此限定符出现在函数声明中的参数列表之后）。表示this指针是指向const对象，与不加const可以相互重载。
    - const成员函数不会修改non-mutable数据成员
- explicit
  - 表明转换构造函数和类型转换函数必须显示
- extern
- inline
  - inline函数说明符
    - 调用点内联展开而非进行函数调用
    - inline不是强制性，具体有编译器决定是否进行内联替换
    - inline表示程序中的函数可能有多个定义，但要求每个定义出现在不同的翻译单元中并且所有定义都相同。
    - 定义在class、struct、union的函数（包括friend函数）都是inline
    - constexpr函数是inline函数
    - 被删除的函数是inline函数
  - inline命名空间说明符
    - 内联命名空间中的名字可以被外层命名空间直接使用
    - `inline`必须出现在第一次定义的地方
- mutable
  - 类型说明符（类的数据成员）
    - 表示该数据成员可以修改，即使的const对象的成员
  - 作用lambda表达式
    - 表示lambda函数体可以改变一个值传递的变量的值，也可以调用该变量的non-const成员函数
      ```
      class Data {
       public:
        Data(): data_(0) {}
        Data(int val): data_(val) {}
        int get_value() { return data_; }
        int data_;
      };
      int main() {
        Data d(10);
        //auto g = [d]() -> int { ++d.data_; return d.get_value(); }; // 语法错误
        auto f = [d]() mutable -> int { ++d.data_; return d.get_value(); };
        cout << f() << " " << d.data_; // 11 10
        system("pause");
        return 0;
      }
      ```
- static
- volatile