# STL内核分析

- STL六大部件
  - 容器（Containers）
  - 分配器（Allocators）
  - 算法（Algorithms）
  - 迭代器（Iterators）
  - 适配器（Adapters）
  - 仿函数（Functions）

## 容器默认分配器（GNU4.9以及以后）以vector为例

```
template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
class vector: protected _Vector_base<_Tp, _Alloc> {
  // ...
}
``` 
其中，`std::allocator`的定义如下：
```
namespace std{
template<typename _Tp>
class allocator: public __allocator_base<_Tp> {
  // ...
};
}
```
`__allocator_base`是一个别名  
`#define __allocator_base __gnu_cxx::new_allocator`   
其中`__gnu_cxx::new_allocator`的定义如下：  
```
namespace __gnu_cxx {
template<typename _Tp>
class new_allocator {
  //...
  pointer allocate(size_t __n, const void* = 0) {
    if(__n > this->max_size())
    std::__throw_bad_alloc();
    return static_cast<*_Tp>(::operator new(__n *sizeof(_Tp)));  // 里面是malloc
  }

  void deallocate(pointer __p, size_type) {
    ::operator delete(__p);  // 里面是free
    // ...
  }
};
}
```
所以在默认情况下，容器使用的分配器是简单对`malloc`和`free`封装，没有多余的内存管理。
