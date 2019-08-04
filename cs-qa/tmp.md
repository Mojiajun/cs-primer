# 计算机基础知识
## C++
### 语法基础

#### 对于C/C++编写的程序，从源代码到可执行文件的过程
1. 预处理，产生.ii文件
2. 编译，产生汇编文件(.s文件)
3. 汇编，产生目标文件(.o或.obj文件)
4. 链接,产生可执行文件(.out或.exe文件)

#### 声明和定义的区别
声明规定变量的类型和名字，使得名字为程序所知；而定义还申请存储空间，负责创建与名字关联的实体，也可能会为变量赋予一个初始值

#### 指针和引用的区别
1. 指针保存的是指向对象的地址，引用相当于变量的别名
2. 引用在定义的时候必须初始化，指针没有这个要求
3. 指针可以改变地址，引用必须从一而终
4. 不存在空应引用，但是存在空指针NULL，相对而言引用更加安全
5. 引用的创建不会调用类的拷贝构造函数  

#### new/delete与malloc/free的区别
1. new是运算符，malloc是C语言库函数
2. new可以重载，malloc不能重载
3. new的变量是数据类型，malloc的是字节大小
4. new可以调用构造函数，delete可以调用析构函数，malloc/free不能
5. new返回的是指定对象的指针，而malloc返回的是void*，因此malloc的返回值一般都需要进行类型转化
6. malloc分配的内存不够的时候可以使用realloc扩容，new没有这样的操作
7. new内存分配失败抛出bad_malloc，malloc内存分配失败返回NULL值 、

#### define/const/inline的区别  
- 本质：define只是字符串替换，const参与编译运行，具体的
  - define不会做类型检查，const拥有类型，会执行相应的类型检查
  - define仅仅是宏替换，不占用内存，而const会占用内存、
  - const内存效率更高，编译器通常将const变量保存在符号表中，而不会分配存储空间，这使得它成为一个编译期间的常量，没有存储和读取的操作  
- 本质：define只是字符串替换，inline由编译器控制，具体的
  - 内联函数在编译时展开，而宏是由预处理器对宏进行展开
  - 内联函数会检查参数类型，宏定义不检查函数参数 ，所以内联函数更安全。
  - 宏不是函数，而inline函数是函数
  - 宏在定义时要小心处理宏参数，（一般情况是把参数用括弧括起来）。  

#### 构造函数为什么不能定义为虚函数，析构函数为什么可以？
1. 虚函数的执行依赖于虚函数表。而虚函数表需要在构造函数中进行初始化工作，即初始化vptr，让他指向正确的虚函数表。而在构造对象期间，虚函数表还没有被初始化，将无法进行
2. 在类的继承中，如果有基类指针指向派生类，那么用基类指针delete时，如果不定义成虚函数，派生类中派生的那部分无法析构
3. 构造函数不要调用虚函数。在基类构造的时候，虚函数是非虚，不会走到派生类中，既是采用的静态绑定。显然的是：当我们构造一个子类的对象时，先调用基类的构造函数，构造子类中基类部分，子类还没有构造，还没有初始化，如果在基类的构造中调用虚函数，如果可以的话就是调用一个还没有被初始化的对象，那是很危险的，所以C++中是不可以在构造父类对象部分的时候调用子类的虚函数实现。但是不是说你不可以那么写程序，你这么写，编译器也不会报错。只是你如果这么写的话编译器不会给你调用子类的实现，而是还是调用基类的实现

#### 字节对齐的原则，如何强制不对齐？
1. 从0位置开始存储
2. 变量存储的起始位置是该变量大小的整数倍
3. 结构体总的大小是其最大元素的整数倍，不足的后面要补齐
4. 结构体中包含结构体，从结构体中最大元素的整数倍开始存
5. 如果加入pragma pack(n) ，取n和变量自身大小较小的一个。
6. #pragma pack()，取消对齐，按照编译器的优化对齐方式对齐

#### 重写、重载与隐藏的区别
1. 重载的函数都是在类内的。只有参数类型或者参数个数不同，重载不关心返回值的类型
2. 覆盖（重写）派生类中重新定义的函数，其函数名，返回值类型，参数列表都跟基类函数相同，并且基类函数前加了virtual关键字
3. 隐藏是指派生类的函数屏蔽了与其同名的基类函数，注意只要同名函数。有两种情况
   - 参数列表不同，不管有无virtual关键字，都是隐藏
   - 参数列表相同，但是无virtual关键字，也是隐藏。

#### 静态链接和动态链接的区别
1. 静态链接：在编译时直接将需要的执行代码拷贝到调用处。优点是在程序发布的时候就不需要依赖库，也就是不再需要带着库一块发布，程序可以独立执行，但是体积可能会相对大些
2. 动态链接：在编译的时候不直接拷贝可执行代码，而是通过记录一些列符号和参数，在程序运行或加载时将这些信息传递给操作系统，操作系统负责将需要的动态库加载到内存中，然后程序在运行到指定代码时，区共享执行内存中已经加载的动态库可执行代码，最终达到运行时链接的目的。优点是多个程序可以共享同一段代码，而不需要在磁盘上存储多个拷贝;缺点是由于是运行时加载，可能会影响程序的前期执行。

#### C++如何创建一个类，使得只能在堆或者栈上创建
- 只能在堆上生成对象：将析构函数设置为私有
  - 原因：C++是静态绑定语言，编译器管理栈上对象的生命周期，编译器在为类对象分配栈空间时，会先检查类的析构函数的访问性。若析构函数不可访问，则不能在栈上创建对象。  
- 只能在栈上生成对象：将new 和 delete 重载为私有
  - 原因：在堆上生成对象，使用new关键词操作，其过程分为两阶段：第一阶段，使用new在堆上寻找可用内存，分配给对象；第二阶段，调用构造函数生成对象。将new操作设置为私有，那么第一阶段就无法完成，就不能够再堆上生成对象。

#### 一个空类sizeof是多少，为什么？
这是因为空类同样可以被实例化，如果定义对空类取sizeof()的值为0，那么该空的类或结构体实例化出很多实例时，在内存地址上就不能区分该类实例化出的实例。所以，为了实现每个实例在内存中都有一个独一无二的地址，编译器往往会给一个空类隐含的加一个字节，这样空类在实例化后在内存得到了独一无二的地址，所以空类所占的内存大小是1个字节。

#### C++拷贝构造函数
如果一个构造函数的第一个参数是自身类类型的引用，且任何外参数都有默认值，则此构造函数是拷贝构造函数

#### 枚举类型的大小

#### 解释C++指针；
#### sizeof（）函数；（比如sizeof一个指针）
#### C++多态，虚函数，智能指针（包括底层方式和适用场景以及不适用场景）
#### int a[10]，求sizeof（a）和sizeof（a*）；
#### int (*a)[10] 解释；（指针数组）



#### union和结构体的区别与联系？
#### 引用声明为null的空指针会触发什么操作？

### 关键字

#### const 关键字的作用  
1. 欲阻止一个变量被改变，可以使用const关键字。在定义该const变量时，通常需要对它进行初始化，因为以后就没有机会再去改变它了
2. 对指针来说，可以指定指针本身为const，也可以指定指针所指的数据为const，或二者同时指定为const
3. 在一个函数声明中，const可以修饰形参，表明它是一个输入参数，在函数内部不能改变其值
4. 对于类的成员函数，若指定其为const类型，则表明其是一个常函数，不能修改类的成员变
5. 对于类的成员函数，有时候必须指定其返回值为const类型，以使得其返回值不为“左值“; 

#### static关键字的作用  
1. 修饰局部变量。该变量便存放在静态数据区，其生命周期一直持续到整个程序执行结束（初始化只进行一次）；虽然用static对局部变量进行修饰过后，其生命周期以及存储空间发生了变化，但是其作用域并没有改变，其仍然是一个局部变量，作用域仅限于该语句块
2. 修饰全局变量。对于一个全局变量，它既可以在本源文件中被访问到，也可以在同一个工程的其它源文件中被访问(只需用extern进行声明即可)
3. 修饰函数。用static修饰函数的话，情况与修饰全局变量大同小异，就是改变了函数的作用域
4. 在class内的static全局变量可以被class内所有函数访问，但不能被class外其他函数访问，对类的所有对象只有一份复制
5. 在类中的static成员函数属于整个类所拥有，这个函数不接收this指针，因而只能访问类的static成员变量  

#### extern关键字的作用  
1. extern可置于变量或者函数前，以表示变量或者函数的定义在别的文件中，提示编译器遇到此变量和函数时在其他模块中寻找其定义。另外，extern也可用来进行链接指定
2. 修饰函数。如果函数的声明中带有关键字extern，仅仅是暗示这个函数可能在别的源文件里定义，没有其它作用
3. extern “C”。 C++语言在编译的时候为了解决函数的多态问题，会将函数名和参数联合起来生成一个中间的函数名称，而C语言则不会，因此会造成链接时找不到对应函数的情况，此时C函数就需要用extern “C”进行链接指定，这告诉编译器，请保持我的名称，不要给我生成用于链接的中间函数名。  

#### volatile关键字的作用  
1. volatile是变量修饰符，其修饰的变量具有可见性，volatile不保证原子性，同时volatile禁止指令重排，使用volatile修饰符的变量是直接读写主存
2. 一个参数可以即是const又是volatile的吗？可以，一个例子是只读状态寄存器，是volatile是因为它可能被意想不到的被改变，是const告诉程序不应该试图去修改他 

### 虚函数与多态

#### 多态的实现方法都有哪几种
- 重载
  - 如果同一作用域（一个类就是一个作用域，具有继承关系的类是在不用的作用域）的几个函数名字相同但形参列表不同（参数个数、类型、顺序），称之为函数重载
  - 在C++的底层，有重命名机制，比如下面这个函数。底层的重命名机制将函数根据参数的个数，参数的类型，返回值的类型都做了重新命名。在底层，不同的重载类型具有不同的函数名。
- 模板
  - 所谓函数模板，实际上是建立一个通用函数，其函数类型和形参类型不具体指定，用一个虚拟的类型来代表。这个通用函数就称为函数模板。在调用函数时系统会根据实参的类型来取代模板中的虚拟类型，从而实现了不同函数的功能。
- 虚函数（覆盖或重写）
  - 派生类中的函数只有与基类虚函数的调用形式一样（有一个例外，返回对象本身的指针或应用）才构成覆盖（重写）
  - 具有虚函数的类，有一个指针，通常称为`vptr`，指向一个某表格（virtual table）。表格中持有类的虚函数地址。这些地址在编译时确定，是固定不变的。每个虚函数都被指派一个表格索引值。比如说，有这么一个例子：
  ```
  base_ptr->z();
  ```
  每次调用z()时，我们并不知道base_ptr指向的真正类型。然而我们知道，经过base_ptr可以存取到该对象的virtual table。并且，在表格特定的偏移处，有z()的地址。比如说是在slot 4中，则编译器可以将上述调用转换为：
  ```
  (*base_ptr->vptr[4](base_ptr));
  ```
  执行期唯一确定base_ptr，不同的base_ptr调用不同的实例。

#### 静态函数和虚函数调用时的区别

### STL

#### STL迭代器类型
有五种迭代器
- 输入迭代器（Input Iterator）：只能读取迭代器指向的元素，并且迭代器只能++（不能访问遍历过的元素）
- 输出迭代器（Output Iterator）：可以改变迭代器指向的元素，并且迭代器只能++
- 前进迭代器（Forward Iterator）：只能++，但是可以多次访问
- 双向迭代器（Bidirectional Iterator）：可以++和--
- 随机访问迭代器（Random Iterator）：可以随机访问

#### vector list map 介绍一下，并说vector和list有什么不同
#### vector内存是怎么增长的
#### map里面为什么要用红黑树
#### vector及底层实现，迭代器失效的情况
#### map和hash_map；

### C++11

#### C++11了解吗？说一下智能指针

### 设计模式
#### 了解过那些设计模式？（单例模式、reactor设计模式）
#### 那我问你点简单的，讲讲工厂和抽象工厂吧

#### 面向对象设计原则 (S.O.L.I.D)  
|简写|全拼|中文翻译|备注|
|:-:|:-:|:-:|:-:|
|SRP|The Single Resoposibility Principle|单一责任原则|修改一个类的原因只有一个（一个类负责一件事）|
|OCP|The Open Closed Principle|开放封闭原则|类应该对扩展开放，对修改关闭|
|LSP|The Liskov Substitution Principle|里氏替换原则|子类对象必须能够替换掉所有父类对象|
|ISP|The Interface Segregation Principle|接口分离原则|不应该强迫客户依赖于他们不用的方法|
|DIP|The Dependency Inversion Principle|依赖倒置原则|高层模块不应该依赖底层模块，二者都应该依赖于抽象;<br>抽象不应该依赖于细节，细节应该依赖于抽象|

## 算法与数据结构

### 数据结构

#### 二叉排序树
#### 平衡二叉树
#### 红黑树
#### B树
#### B+树
#### 跳表
#### 哈希表
#### 图（邻接矩阵、邻接链表）

### 算法
#### 写一个二分查找
#### 在一个旋转数组中查找一个数，要求时间复杂度为O(log(N))
#### 快速排序
#### 海量数据Bitmap
#### 海量数据排序（归并、基数排序）
#### 用两个栈实现队列
#### a的n次方怎么计算。
#### 自己实现一个atoi函数；
#### 如何判断一个二叉树是不是平衡二叉树
#### 输出当前数组数据中前两位大的数，提供优化方案和并行方案
#### 如何判断两个单向链表是否重叠？（使用快慢指针）
#### 快速排序的时间复杂度和空间复杂度？
#### 稳定的排序和不稳定的排序的概念？比如？


## 网络
## 操作系统
## 数据库
## 工具
## 其他
#### Python里的init方法与new方法的区别
#### Python导入一个包的过程