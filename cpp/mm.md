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

## placement new

- placement new允许我们将构造于allocated memory中
- 没有所谓的placement delete，因为placement new根本没有分配memory。
- 与placement new对应的operator new称为placement delete
- 重载的时候要写一个placement delete与placement new对应

例如
```
#include <new>
char *buf = new char[sizeof(Complex)*3];
Complex *pc = new(buf)Complex(1, 2);  // [1]
//...
delete[] buf;
```
编译器转化为：
```
Complex *pc;
try {
  void *mem = operator new(sizeof(Complex), buf);//[2], buf为[1]生成的
  pc = static_cast<Complex*>(mem);
  pc->Complex::Complex(1, 2);
} catch(std::bad_alloc) {
  // 若alloocation失败就不执行constructor
}
```
函数[2]的底层
- 不分配内存，只是将传入的buf返回
```
void *operator new(size_t, void* loc) {
  return loc; 
}
```

## C++应用程序，分配内存的途径

<img src="./imgs/mm-reload.png " width="70%">

# C++容器，分配内存的途径

<img src="./imgs/mm-container.png " width="70%">

## 重载::operator new / ::operator delete
- 重载全局函数的影响无远弗届
```
void *myAlloc(size_t size) {
  return malloc(size);
}
void myFree(void *ptr) {
  return free(ptr);
}
// 他们不可以声明与一个namespace内
inline void *operator new(size_t size) {
  cout << "overload global new()\n";
  return myAlloc(size);
}
inline void *operator new[](size_t size) {
  cout << "overload global new[]()\n";
  return myAlloc(size);
}
inline void operator delete(void *ptr) {
  cout << "overload global delete()\n";
  return myFree(ptr);
}
inline void operator delete[](void *ptr) {
  cout << "overload global delete[]()\n";
  return myFree(ptr);
}
```

## 重载operator new/operator delete

- 在一个具体的类中重载
- 必须是静态函数，但是可以省略static，编译器自动添加
- 原因：构建对象的时候，对象不存在，没有this指针

```
class Foo {
 public:
   void *operator new(size_t);
   void *operator new[](size_t);
   void operator delete(void*, size_t); // size_t is optional
   void operator delete[](void*, size_t);
}
```

## 重载placement new: new() / delete() 

- 我们可以重载class member `operator new()`，写出多个版本，前提是每一个版本的声明都必须有独特的参数列，其中第一个参数不许是`size_t`，其余参数以`new`所指定的`placement arguments`为初值。出现`new(...)`小括号内的便是所谓`placement new`。
- 我们也可以重载class member `operator delete()`，写出多个版本。但他们绝不会被`delete`调用。<u>**只有当`new`所调用的构造函数抛出exception，才会调用这些重载版本的`operator delete()`**</u>。它只可能这样被调用，主要用来<u>**归还未能完全创建成功的object所占用的内存**</u>。
- placement new和placement delete参数对应关系
  - placement new第一个参数必须是size_t；placement delete第一个参数是void*
  - 后继参数一一对应
- 即使operator(placement) delete(...)未能一一对应于operator new(...)也不会出现任何报错。没有回应的operator(placement) delete(...)表示放弃处理构造函数发出的异常。
- 示例
  ```
  class Foo {
   public:
    Foo() { cout << "Foo::Foo()" << endl; }
    Foo(int) {cout << "Foo::Foo(int)" << endl; throw Bad(); }
    // (1)这个就是一般的operator new()的重载
    void *operator new(size_t size) { return malloc(size); }
    // (2)这个就是标准库已提供placement new()的重载形式
    void *operator new(size_t size, void *start) { return start; }
    // (3)这个才是崭新的placement new
    void *operator new(size_t, long extra) { return malloc(size+extra); }
    // (4)这又是一个placement new
    void *operator new(size_t size, long extra, char init) { return malloc(size+extra); }
    // (5)这又是一个placement new，但故意写错第一参数的type（必须是size_t）
    void *operator new(long extra, char init) { return malloc(extra); }
    
    // 以下是搭配上述placement new的各个所谓placement delete
    // 当构造函数抛出异常，这儿对应的operator(placement) delete就会被调用
    // 其用途是释放对应之placement new分配的内存
    // (6)这个就是一般的operator delete()的重载
    void operator delete(void *, size_t) {cout << "operator delete(void* size_t)" << endl; }
    // (7) 对应于（2）
    void operator delete(void *, void*) {cout << "operator delete(void* void*)" << endl; }
    // (8) 对应于（3）
    void operator delete(void *, long) {cout << "operator delete(void* long)" << endl; }
    // (9) 对应于（4）
    void operator delete(void *, long char) {cout << "operator delete(void* long char)" << endl; }
  }
  ```

## Per-class allocator

- version1
  增加一个next指针用来管理内存，连接成一个链表（造成内存浪费）
  ```
  #include <cstddef>
  #include <iostream>
  using namespace std;
  class Screen {
   public:
    Screen(int x): i(x) {}
    int get() { return x; }
    void *operator new(size_t);
    void operator delete(void*, size_t);
    // ...
   private:
    Screen *next;
    static Screen* freeStore;
    static const int screenChunk;
    int i;
  };
  Screen* Screen::freeStore = NULL:
  const int Screen::screenChunk = 24;
  void *Screen::operator new(size_t size) {
    Screen *p;
    if(!freeStore) {
      size_t chunk = screenChunk * size;
      freeStore = p = static_cast<Screen*>(new char[chunk]);
      for(;p != &freeStore[screenChunk-1]; ++p)
        p->next = p+1;
      p->next = 0;
    }
    p = freeStore;
    freeStore = freeStore->next;
    return p;
  }
  void operator delere(void *p, size_t size) { // 回收，插入头部
    (static_cast<Screen*>(p))->next = freeStore;
    freeStore = static_cast<Screen*>(p);
  }
  ```

- version2  
  使用嵌入式指针来节省内存
  ```
  class Airplane {
   private:
    struct AirplaneRep {
      unsigned long miles;
      char type;
    };
    union {
      AirplaneRep rep; // 此针对使用中的object
      Airplane* next; // 此针对free list上的object
    };
   public:
    unsigned long getMiles() { return rep.miles; }
    char getType() { return rep.type; }
    void set(unsigned long m, char t) { rep.mile = m; rep.type = t; }
   public:
    static void *operator new(size_t);
    static void *operator delete(void *, size_t);
   private:
    static const int BLOCK_SIZE;
    static Airplane *headOfFreeList;
  };
  const int Airplane::BLOCK_SIZE = 512;
  Airplane* Airplane::headOfFreeList = NULL;

  void *Airplane::operator new(size_t size) {
    if(size != sizeof(Airplane))
      return ::operator new(size);
    Airplane *p = headOfFreeList;
    if(p)
      headOfFreeList = p->next;
    else {
      Airplane *newBlock = static_cast<Airplane*>(::operator new(BLOCK_SIZE*sizeof(Airplane)));
      for(int i = 1; i < BLOCK_SIZE-1; ++i)
        newBlock[i].next = &newBlock[i+1];
      p = newBlock;
      headOfFreeList = &newBlock[1];
    }
    return p;
  }
  void Airplane::operator delete(void *deadObject, size_t size) {
    if(deadObject == 0) return;
    if(size != sizeof(Airplane))
      ::operator delete(deadObject);
    Airplane *carass = static_cast<Airplane*>(deadObject);
    carass->next = headOfFreeList;
    headOfFreeList = carass;
  }
  ```

- version3-  static allocator  
  当你受困于必须为不同的classes重写一遍几乎相同的member operator new和member operator delete时，应该有方法将一个总是分配特定尺寸区块的memory allocator概念包装起来，是他容易被重复使用。一下展示一种做法，每个allocator object都是一个分配器，它维持一个free list；不同的allocator objects维持不用的free list。
  ```
  class allocator {
   public:
    void *allocate(size_t);
    void deallocate(void *, size_t);
   private:
    struct obj { struct obj* next; }; // 嵌入式指针
    obj * freeStore = nullptr;
    const int CHUNK = 4;
  };
  void *allocator::allocate(size_t size) {
    obj *p;
    if(!freeStore) { // 为空，申请一大块
      size_t chunk = CHUNK * size;
      freeStore = p = (obj*)malloc(chunk);
      // 将分配得来的一大块当做linked list
      for(int i = 0; i < CHUNK-1; ++i) {
        p->next = (obj *)((char*)p + size);
        p = p->next;
      }
      p->next = nullptr;
    }
    p = freeStore;
    freeStore = freeStore->next;
    return p;
  }
  void allocator::deallocate(void* p, size_t) {
    // 将*p收回插入free list前端
    ((obj*)p)->next = freeStore;
    freeStore = (obj *)p;
  }
  ```
  
  在其他类中应用这个allocator。不需要进行内存分配操作，让allocator去操心。
  ```
  class Foo {
   public:
    Foo(long l): L(l) {}
    static void* operator new(size_t size) { return myAlloc.allocate(size); }
    static void operator delete(void *p, size_t size) { return myAlloc.deallocate(p, size); }
   private:
    long L;
    string str;
    static allocator myAlloc;
  }
  allocator Foo::myAlloc;
  ```

- version4 - macro for static allocator  
 
  将用到allocator的部分定义成宏
  ```
  #define DECLARE_POOL_ALLOC() \
  public: \
   void *operator new(size_t size) { return myAlloc.allocate(size); } \
   void operator delete(void *p, size_t size) { return myAlloc.deallocate(p, size); } \
  protected: \
   static allocate myAlloc;

  #define IMPLEMENT_POOL_ALLOC(classname) \
  allocator class_name::myAlloc;
  ```

  则`class Foo`可以简写为
  ```
  class Foo {
     DECLARE_POOL_ALLOC();
     public:
      Foo(long l): L(l) {}
     private:
      long L;
      string str;
      static allocator myAlloc;
    }
    IMPLEMENT_POOL_ALLOC(Foo);
  ```

## global allocator (with multiple free-lists)

将前述allocator进一步发展为具有16条free-lists，并以此不再以application classes内的static呈现，而是一个global allocator -- 这就是GNU2.9的std::alloc的雏形。
<img src="./imgs/mflist.png " width="70%">

## new handler
当operator new没有能力为你分配出你申请的memory，会抛出一个`std::bad_alloc exception`。某些老式编译器则是返回0--你仍然可以领编译器那么做：  
`new(nothrow) Foo;`  
此为<u>nothrow形式</u>。

抛出exception之前会先（**不只一次**）调用一个<u>可由client指定的handler</u>，以下是new handler的形式和设定方法：
```
typedef void(*new_handler)();
new_handler set_new_handler(new_handler p) throw();
```

良好设计的new handler只有两个选择：
- 让更多的memory可用
- 调用`abort()`或者`exit()`

# std::allocator

## VC6标准分配器之实现
VC6的`allocator`只以`::operator new`和`::operator delete`完成`allocate()`和`deallocate()`，**没有任何特殊的设计**。
```
#ifndef _FARQ
#define _FARQ
#define _PDFT ftrdiff_t
#define _SIZE size_t
#endif

template<class _Ty>
class allocator {
 public:
  typedef _SIZE size_type;
  typedef _PDFT difference_type;
  typedef _Ty _FARQ *pointer;
  typedef _Ty value_type;
  pointer allocate(size_type _N, const void*) {
    return (_Allocate((difference_type)_N, (pointer)0));
  }
  void deallocate(void _FARQ *_P, size_type) { operator delete(_P); }
}
```
其中用到的`_Allocate()`定义如下：
```
template<class _Ty> inline
_Ty _FARQ *_Allocate(_PDFT _N, _Ty _FARQ*) {
  if(_N < 0) _N = 0;
  return ((_Ty _FARQ*)operator new((_SIZE)_N * sizeof(_Ty)));
}
```

## G2.9标准分配器之实现
G2.9的`allocator`只以`::operator new`和`::operator delete`完成`allocate()`和`deallocate()`，**没有任何特殊的设计**。
```
template <class T>
class allocator {
 public:
  typedef T  value_type;
  typedef T* pointer;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  pointer allocate(size_type n) { return ::allocate((difference_type)n, (pointer)0); }
  void deallocate(pointer p) { ::deallocate(p); }
}
```
其中`::allocate`和`::deallocate`的实现为：
```
template <class T>
inline T* allocate(prtdiff_t size, T*) {
  set_new_handler(0);
  T *tmp = (T*)(::operator new((size_t)(size*sizeof(T)));
  if(tmp == 0) {
    cerr << "out of memory" << endl;
    exit(1);
  }
  return tmp;
}

template <class T>
inline void deallocate(T* buffer) { ::operator delete(buffer); }
```

**这个分配器并没有引入到任何SGI STL的容器中，用的是另外一个`std::alloc`**

```
template <class T, class Alloc=alloc>
class vector {//...}

template <class T, class Alloc=alloc>
class list {//...}

template <class T, class Alloc=alloc, size_t BufSiz=0>
class deque {//...}

template <class Key,
          class T,
          class Compare=less<Key>,
          class Alloc=alloc>
class map {//...}

template <class Key,
          class Compare=less<Key>,
          class Alloc=alloc>
class set {//...}
```

- **G2.9版本下的`std::alloc`在G4.9替换为`__gnu_cxx::__pool_alloc`**

## G4.9标准分配器之实现

G4.9的`allocator`只以`::operator new`和`::operator delete`完成`allocate()`和`deallocate()`，**没有任何特殊的设计**。

```
// .../bits/new_allocator.h
template<typename _Tp>
class new_allocator {
  // ...
  pointer allocate(size_type __n, const void* = 0) {
    if(__n > this->max_size())
      std::__throw_bad_alloc();
    return static_cast<_Tp*>(::operator new(__n*sizeof(_Tp)));
  }
  void deallocate(pointer __p, size_type) { 
