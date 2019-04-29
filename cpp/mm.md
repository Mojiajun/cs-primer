# primintives

## C++ 应用程序

<img src="./imgs/cpp-app.png " width="30%">

## C++ memory primitives
|分配|释放|类属|可否重载|
|:-|:-|:-|:-|
|malloc()|free()|C函数|不可|
|new|delete|C++表达式|不可|
|::operator new()|::operator delete()|C++函数|可|
|allocator<T>::allocate()|allocator<T>::deallocate()|C++标准库|可自由设计并以之搭配任何容器|

```
示例
void *p1 = malloc(512); //512 bytes
free(p1);

complex<int> *p2 = new complex<int>; // one object
delete p2;
void *p3 = ::operator new(512); // 512 bytes
::operator delete(p3);

#ifdef _MSC_VER
int *p4 = allocator<int>().allocate(3, (int*)0);// 3 ints
allocator<int>().deallocator(p4, 3);
#endif
#ifdef __BOLANDC__
int *p4 = allocator<int>().allocate(5); // 5 ints
allocator<int>().deallocator(p4, 5);
#endif
#ifdef __GNUC__
void *p4 = allocator<int>().allocate(7);  // 7 ints
allocator<int>()deallocate(p4, 7);

void *p5 = __gnu_cxx::__pool_alloc<int>().allocte(9); //9 ints
__gnu_cxx::__pool_alloc<int>().deallocte(p5, 9);
#endif
```

## new expression

```
Complex *pc = new Complex(1, 2);

// 编译器转化为：
Complex *pc;
try {
  void *mem = operator new(sizeof(Complex)); //allocate
  pc = static_cast<Complex*>(mem); //cast
  pc->Complex::Complex(1, 2);  //construct, 只有编译器才可以直接调用Constructor
  // 想要直接调用constructor，可用placement new
  // 例如：new(pc)Complex(1, 2); // pc 已经分配
} catch(std::bad_alloc) {
  // 不执行constuction
}

// operator new(sizeof(Complex))的底层(vc)
void *operator new(size_t size, const std::nothrow_t&)_THROW0() {
  void *p;
  while((p==malloc(size))==0) {
    _TRY_BEGIN
      if(_callnewh(size)==0)break;
    _CATCH(std::bad_alloc) return (0);
    _CATCH_END;
  }
}
```

## delete expression

```
Complex *pc = new Complex(1, 2);
// ...
delete pc;

// 编译器转化为  
pc->~Complex(); //先析构  
operator detele(pc); //然后释放内存

// operator delete(pc)底层  
void __cdecl operator delete(void *p) _THROW0() {
  free(p);
}
```

## array new, array delete

```
Complex *pca = new Complex[3];
// 调用三次默认构造函数
// 无法传入参数给予处置
// ...
delete[] pca; // 调用3次析构函数
// 缺失[]，只会调用一次析构函数
```

delete缺失[]一定会有内存泄露吗？
- 对class without pointer member可能没影响
- 对class with pointer member通常有影响  
<img src="./imgs/mm-leak.png " width="70%">

