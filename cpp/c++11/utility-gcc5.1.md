# C++ Utility Library

## `std::initlizer_list`
`std::initializer_list<T>` 类型的对象是一个轻量级代理对象，它提供对常量类型对象数组的访问。

在以下情况会自动构造 `std::initializer_list` 对象
1. 用花括号 `{}` 的方式初始化一个对象且该对象有接受 `std::initializer_list` 参数的构造函数
2. 用花括号 `{}` 的方式给一个对象赋值或者函数返回且该对象重载了接受 `initializer_list` 参数的赋值运算符
3. 在 `Range-based for loop` 用于指定范围

其底层实现是一个数组。
```
/// @file initializer_list
46   template<class _E>
47     class initializer_list
48     {
49     public:
50       typedef _E           value_type;
51       typedef const _E&    reference;
52       typedef const _E&    const_reference;
53       typedef size_t               size_type;
54       typedef const _E*    iterator;
55       typedef const _E*    const_iterator;
56 
57     private:
58       iterator                     _M_array;
59       size_type                    _M_len;
60 
61       // The compiler can call a private constructor.
62       constexpr initializer_list(const_iterator __a, size_type __l)
63       : _M_array(__a), _M_len(__l) { }
64 
65     public:
66       constexpr initializer_list() noexcept
67       : _M_array(0), _M_len(0) { }
```
主要由编译器调用。主要有 begin、end 和 size 函数接口
```
/// @file initializer_list
69       // Number of elements.
70       constexpr size_type
71       size() const noexcept { return _M_len; }
72 
73       // First element.
74       constexpr const_iterator
75       begin() const noexcept { return _M_array; }
76 
77       // One past the last element.
78       constexpr const_iterator
79       end() const noexcept { return begin() + size(); }
80     };
```

## Data、time
### `std::chrono::time_point`
TODO

## `std::move()`
move() 获得一个右值引用
```
/// @file include/bits/move.h
 99   template<typename _Tp>
100     constexpr typename std::remove_reference<_Tp>::type&&
101     move(_Tp&& __t) noexcept
102     { return static_cast<typename std::remove_reference<_Tp>::type&&>(__t); }
```
move() 的函数参数 `_Tp&&` 是一个指向模板类型参数的右值引用。通过引用折叠，此参数可以与任何类型的实参匹配。特别是，我们既可以传递给 move() 一个左值，也可以传递给它一个右值。

首先看以下 std::remove_reference 的定义
```
/// @file include/std/type_traits
1574   template<typename _Tp>
1575     struct remove_reference
1576     { typedef _Tp   type; };
1577 
1578   template<typename _Tp>
1579     struct remove_reference<_Tp&>
1580     { typedef _Tp   type; };
1581 
1582   template<typename _Tp>
1583     struct remove_reference<_Tp&&>
1584     { typedef _Tp   type; };
```
`std::remove_reference` 是去除引用：如果类型 `_Tp` 是引用类型（`_Tp&` 或 `_Tp&&`），则提供成员 typedef 类型，该类型是由 `_Tp` 引用的类型。否则类型为 `_Tp`。

通常我们不能将一个右值引用绑定到一个左值上，但是C++在正常绑定规则之外定义了两个例外规则，允许这种绑定：
- 当我们将一个左值传递给函数的右值引用参数，且此右值引用指向模板类型参数时，编译器推断模板类型参数为实参的左值引用类型。
- 如果我们创建一个引用的引用，则这些引用形成了“折叠”（只能应用于间接创建的引用的引用，如类型别名或模板参数）
  - `X& &`、`X& &&`、`X&& &` 都会折叠成 `X&`
  - `X&& &&` 折叠成 `X&&`

这个规则导致了两个重要的结果：
- 如果一个函数参数是一个指向模板类型参数的右值引用，则它可以被绑定到一个左值；且
- 如果实参是一个左值，则推断出的模板实参类型将是一个左值引用，且函数参数将被实例化为一个普通的左值引用参数（`T&`）

因此 std::move() 的工作如下：
1. 传入右值，如 `auto s2 = std::move(string("bye!"));`
   - 推断出的 `_Tp` 的类型为 `string`
   - 因此，`remove_reference` 用 `string` 进行实例化
   - `remove_reference<string>` 的 type 成员是 `string`
   - move() 的返回类型是 `string&&`
   - move() 的函数参数 `__t` 的类型为 `string&&`
2. 传入左值，如 `auto s1 = string("bye); auto s2 = std::move(s1);`
   - 推断出的 `_Tp` 的类型为 `string&`
   - 因此，`remove_reference` 用 `string&` 进行实例化
   - `remove_reference<string>` 的 type 成员是 `string`
   - move() 的返回类型是 `string&`
   - move() 的函数参数 `__t` 的类型为 `string& &&`，会折叠为 `string&`

## `std::swap`
通过移动类交换两个变量。它要求传入的变量是可以移动的（具有移动构造函数和重载移动赋值运算符）。宏定义 `_GLIBCXX_MOVE` 在C++11展开为 std::move()。
```
/// @file include/bits/move.h
174   template<typename _Tp>
175     inline void
176     swap(_Tp& __a, _Tp& __b)
177 #if __cplusplus >= 201103L
178     noexcept(__and_<is_nothrow_move_constructible<_Tp>,
179                 is_nothrow_move_assignable<_Tp>>::value)
180 #endif
181     {
182       // concept requirements
183       __glibcxx_function_requires(_SGIAssignableConcept<_Tp>)
184 
185       _Tp __tmp = _GLIBCXX_MOVE(__a); // 调用移动构造函数构建一个临时对象
186       __a = _GLIBCXX_MOVE(__b); // 移动赋值
187       __b = _GLIBCXX_MOVE(__tmp); // 移动赋值
188     }
189 
190   // _GLIBCXX_RESOLVE_LIB_DEFECTS
191   // DR 809. std::swap should be overloaded for array types.
192   /// Swap the contents of two arrays.
193   template<typename _Tp, size_t _Nm>
194     inline void
195     swap(_Tp (&__a)[_Nm], _Tp (&__b)[_Nm])
196 #if __cplusplus >= 201103L
197     noexcept(noexcept(swap(*__a, *__b)))
198 #endif
199     {
200       for (size_t __n = 0; __n < _Nm; ++__n)
201     swap(__a[__n], __b[__n]);
202     }
```
在移动的过程中，不会有资源的申请和释放。看下面的例子：
```
#define ADD_MOVE
class Test {
 public:
  explicit Test(size_t l): len_(l), data_(new unsigned char[l]) {
    printf("Test Ctor\n");
  }
  Test(const Test& other): len_(other.len_), data_(new unsigned char[len_]) {
    printf("Test Copy Ctor\n");
    memmove(data_, other.data_, len_);
  }
  Test& operator=(const Test& rhs) {
    printf("Test operator=\n");
    if(this != &rhs) {
	  len_ = rhs.len_;
	  data_ = new unsigned char[len_];
	  memmove(data_, rhs.data_, len_);
	  return *this;
    }
  }
#ifdef ADD_MOVE
  Test(Test&& other): len_(len_), data_(other.data_) {
    printf("Test Move Ctor\n");
    other.len_ = 0;
    other.data_ = nullptr;
  }
  Test&operator=(Test&& rhs) {
    printf("Test Move operator=\n");
    if(this != &rhs) {
      len_ = rhs.len_;
      data_ = rhs.data_;
      rhs.len_ = 0;
      rhs.data_ = nullptr;
    }
  }
#endif
  ~Test() {
    printf("Test Dtor\n");
    delete[] data_;
  }

 private:
  size_t len_;
  unsigned char* data_;
};
```
我们重载 `operator new` 和 `operator delete` 输出分配或释放内存操作
```
void* operator new(size_t size) {
  printf("allocate memory\n");
  return malloc(size);
}

void operator delete(void* ptr) {
  printf("free memory\n");
  free(ptr);
}
```
进行交换测试
```
int main() {
  Test a(1), b(1);
  printf("===========Swap==========\n");
  std::swap(a, b);
  printf("===========Done==========\n");
  return 0;
}
```
在对象“无法”移送的时候（没有显式移动构造函数和移动赋值运算符），交换需要拷贝资源
```
+Allocate Memory
Test Ctor
+Allocate Memory
Test Ctor
===========Swap==========
+Allocate Memory
Test Copy Ctor
Test operator=
+Allocate Memory
Test operator=
+Allocate Memory
Test Dtor
-Free Memory
===========Done==========
Test Dtor
-Free Memory
Test Dtor
-Free Memory
```
当对象可以移动时，交换没有资源的拷贝
```
+Allocate Memory
Test Ctor
+Allocate Memory
Test Ctor
===========Swap==========
Test Move Ctor
Test Move operator=
Test Move operator=
Test Dtor
===========Done==========
Test Dtor
-Free Memory
Test Dtor
-Free Memory
```

## `std::forward`
是为了支持C++11转发。某些函数需要将其一个或多个实参连同类型不变地转发给其他函数，在此情况下，我们需要保持被转发实参的所有性质，包括实参类型是否是 `const` 的以及实参是左值还是右值。需要注意的是，std::forward() 必须通过显式模板实参来调用。std::forward() 返回该显式实参类型的右值引用。通过其返回类型上的引用折叠，std::forward() 可以保持给定实参的左值/右值属性。
```
/// @file include/bits/move.h
74   template<typename _Tp>
75     constexpr _Tp&&
76     forward(typename std::remove_reference<_Tp>::type& __t) noexcept
77     { return static_cast<_Tp&&>(__t); }

85   template<typename _Tp>
86     constexpr _Tp&&
87     forward(typename std::remove_reference<_Tp>::type&& __t) noexcept
88     {
89       static_assert(!std::is_lvalue_reference<_Tp>::value, "template argument"
90             " substituting _Tp is an lvalue reference type");
91       return static_cast<_Tp&&>(__t);
92     }
```