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

## 迭代器Iterator

- 需要遵循的原则--必须提供5种associated types
  ```
  // ----------------------------------------------------------
  // G2.9
  // ----------------------------------------------------------
  template<class T, class Ref, class Ptr>
  struct __list_iterator {
    typedef bidirectional_iterator_tag iterator_category; // (1)
    typedef T                          value_type;        // (2)
    typedef Ptr                        pointer;           // (3)
    typedef Ref                        reference;         // (4)
    typedef ptrdiff_t                  difference_type;   // (5)
    ...
  };
   // ----------------------------------------------------------
  // G4.9
  // ----------------------------------------------------------
  template<typename _Tp>
  struct _List_iterator {
    typedef bidirectional_iterator_tag iterator_category; // (1)
    typedef _Tp                        value_type;        // (2)
    typedef _Tp*                       pointer;           // (3)
    typedef _Tp&                       reference;         // (4)
    typedef ptrdiff_t                  difference_type;   // (5)
    ...
  };
  ```
  <img src="./imgs/stl-iterator-traits.png" width="50%">

- Traits特性
  - `Iterator Traits`用以区分`class iterators`和`non-class iterators`
  - 这个traits机器必须有能力区分它所获得的iterator是(1)`class iterator T`或是(2)`native pointer to T`。利用`partial specialization`(偏特化)可达到目标。
  <img src="./imgs/stl-iterator-traits1.png" width="50%">
  ```
  // 完整的iterator_traits
  template <class I>
  struct iterator_traits {
    typedef typename I::iterator_category iterator_category;
    typedef typename I::value             value type;
    typedef typename I::difference_type   difference_type;
    typedef typename I::pointer           pointer;
    typedef typename I::reference         reference;
  };

  // partial specialization for regular pointers
  template <class T>
  struct iterator_traits<T*> {
    typedef random_access_iterator_tag iterator_category;
    typedef T                          value_type;
    typedef ptrdifft_t                 difference_type;
    typedef T*                         pointer;
    typedef T&                         reference;
  };

  // partial specialization for regular const pointers
  template <class T>
  struct iterator_traits<const T*> {
    typedef random_access_iterator_tag iterator_category;
    typedef T                          value_type; // 值得注意，不是const T。
    typedef ptrdifft_t                 difference_type;
    typedef const T*                   pointer;
    typedef const T&                   reference;
  };
  ```
  备注：  
  value_type主要是用来声明变量，而声明一个无法被赋值的变量没有声明用，所以iterator(即便是const iterator)的value_type不应加上const。

## 容器vector深度探索

<img src="./imgs/stl-vector.png" width="50%">  

```
template <class T, class Alloc=alloc>
class vector {
 public:
  typedef T           value_type;
  typedef value_type* iterator;
  typedef value_type& reference;
  typedef size_t      size_type;
 protected:
  iterator start;
  iterator finish;
  iterator end_of_storage;
 public:
  iterator begin() { return start; }
  iterator end() { return finish; }
  size_type size() const { return end() - begin(); }
  size_type capacity() const { return size_type(end_of_storage - begin()); }
  bool empty() const { return begin() == end(); }
  reference operator[](size_type n) { return *(begin() + n); }
  reference front() { return *begin(); }
  reference back() { return *(end() - 1); }
  
  // 二倍增长
  void push_back(const T& x) {
    if(finish != end_of_storage) { // 尚有空余空间
      construct(finish, x);        // 全局函数
      ++finish;                    // 调整水位高粗
    } else {                       // 已经没有空间
      insert_aux(end(), x);
    }
  }
 private:
  void insert_aux(iterator position, const T& x);
};
```
`insert_aux`定义如下：
```
template <class T, class Alloc>
void vector<T, Alloc>::insert_aux(iterator position, const T& x) {
  if(finish != end_of_storage) { // 尚有借用空间
    // 在借用空间起始处构建一个元素，并以vector最后一个元素为其初值
    construct(finish, *(finish - 1));
    ++finish;
    T x_copy x;
    copy_backward(position, finish-2, finish-1);
    *position = x_copy;
  } else { // 没有空间
    const size_type old_size = size();
    const size_type len = old_size != 0 ? 2 * old_size : 1; // 两倍增长

    iterator new_start = data_allocator::allocate(len)
    iterator new_finish = new_start;
    try {
      // 将原vector的内容拷贝到新的vector
      new_finish = uninitialized_copy(start, position, new_start);
      construct(new_finish, x); // 为新元素设初值
      ++new_finish;
      // 拷贝安插点后的原内容（因为它可可能被insert(p, x)调用）
      new_finish = unintialized_copy(position, finish, new_finish);
    } catch(...) {
      // "commit or rollback" semantics
      destory(new_start, new_finish);
      data_allocator::deallocate(new_start, len);
      throw;
    }
    // 析构并释放原vector
    destory(begin(), end());
    deallocate();
    // 调整迭代器，指向新vector
    start = new_start;
    finish = new_finish;
    end_of_storage = new_start + len;
  }
}
```

## 容器list深度探索

<img src="./imgs/stl-list.png" width="50%">

```
template<class T>
struct __list_node {
  typedef void* void_pointer;
  void_pointer prev;
  void_pointer newx;
  T data;
};
```
```
template<class T, class Alloc=alloc>
class list {
 protected:
  typedef __list_node<T> list_node;
 public:
  typedef list_node* link_type;
  typedef __list_iterator<T, T&, T*> iterator;
 protected:
  link_type node;
 // ...
}
```
```
template<class T, class Ref, class Ptr>
struct __list_iterator {
  typedef __list_iterator<T, Ref, Ptr> self;
  typedef bidirection_iterator_tag iterator_category;
  typedef T   value_type;
  typedef Ptr pointer;
  typedef Ref reference;
  typedef __list_node<T>* link_type;
  typedef ptrdiff_t difference_type;

  link_type node;

  reference operator*() const { return (*node).data; }
  pointer operator->() const { return &(operator*()); }
  self& operator++() { // 前置++，返回引用
    node = (link_type)((node*).next);
    return *this;
  }
  self operator++(int) { // 后置++
    self tmp = *this;
    ++*this;
    return tmp;
  }
  // ...
};
```

- 备注说明
  - 前置++返回是是引用，后置++返回的是值；--也是类似；
  - 当你对某个type实施`operator->`而该type并非build-in指针时，编译器会做一件有趣的事情：在找出user-defined `operator->`并将它实施于该type后，编译器会对执行结果再次实施`operator->`。编译器不断执行这个动作直到触及一个build-in指针，然后才进行成员存取。
  - G4.9的改进  
  <img src="./imgs/stl-list-com.png" width="50%">

## 容器array深度探索

```
// -----------------------------------------------------------
// TR1
// -----------------------------------------------------------
template <typename _Tp, std::size_t _Nm>
struct array {
  typedef _Tp         value_type;
  typedef _Tp*        pointer;
  typedef value_type* iterator; // native pointer，G2.9vector也是如此

  // Support for zero-sized arrays mandatory
  value_type _M_instance[_Nm ? _Nm : 1];

  iterator begin() { return iterator(&_M_instance[0]); }
  iterator end() { return iterator(&_M_instance[_Nm]); }
  //...
}
```

## 容器deque深度探索

<img src="./imgs/stl-deque.png" width="50%">

```
//G2.9
template <class T, class Ref, class Ptr, size_t BufSiz>
struct __deque_iterator {
  typedef random_access_iterator_tag iterator_category;
  typedef T                          value_type;
  typedef Ptr                        pointer;
  typedef Ref                        reference;
  typedef size_t                     size_type;
  typedef ptrdiff_t                  difference_type;
  typedef T**                        map_pointer;
  typedef __deque_iterator           self;

  T*          cur;
  T*          first;
  T*          last;
  map_pointer node;
  //...
}

template <class T, class Alloc=alloc size_t BufSiz=0>
class deque {
 public:
  typedef T                                   value_type;
  typedef value_type*                         pointer;
  typedef value_type&                         reference;
  typedef size_t                              size_type;
  typedef ptrdiff_t                           difference_type;
  typedef __deque_iterator<T, T&, T*, BufSiz> iterator;
 protected:
  typedef pointer*                            map_pointer; // T**
 protected:
  iterator    start;
  iterator    finish;
  map_pointer map;
  size_type   map_size;
 public:
  iterator begin() { return start; }
  iterator end() { return finish; }
  size_type size() const { return finish - start; }
  
  // 在position处插入一个元素，其值为x
  iterator insert(iterator position, const value_type& x) {
    if(position.cur == start.cur) {         // 如果插入点事deque最前端
      push_front(x);                        // 交给push_front()做
      return start;
    } else if(position.cur == finish.cur) { // 如果插入点是deque的最尾端
      push_back(x);                         // 交给push_back()做
      iterator tmp = finish;
      --tmp;
      return tmp;
    } else {                               // 其他插入点，交给insert_aux()做
      return insert_aux(position, x)
    }
  }
}
```
`insert_aux`定义如下：
```
template <class T, class Alloc, size_t BufSize>
typename dequee<T, Alloc, BufSize>::iterator
deque<T, Alloc, BUfSize>::insert_aux(iterator pos, const value_type& x) {
  difference_type index = pos - start;  // 插入点之前的元素个数
  value_type x_cooy = x;
  if(index < size / 2) {                // 如果插入点之前的元素个数较少
    push_front(front());                // 在最前端加入与首元素同值的元素
    //...
    copy(front2, pos1, front1);         // 元素移动
  } else {                              // 如果插入点之后的元素个数较少
    push_back(back());                  // 在最末端加入与末尾元素同值的元素
    //...
    copy_backward(pos, back2, back1);   // 元素移动
  }
  *pos = x_copy;                        // 在插入点设定新值
  return pos;
}
```
`BufSiz`的确定：
```
inline size_t __deque_buf_size(size_t n, size_t sz) {
  // sz为元素大小
  return n != 0 ? n : (sz < 512 ? size_t(512 / sz) : size_t(1));
}
```
deque如何模拟连续空间
```
// 简化，定义在deque
reference operator[](size_type n) { return start[difference_type(n)]; }

reference front() { return *start; }

reference back() {
  iterator tmp = finish;
  --tmp;
  return *tmp;
}

size_type size() const { return finish - start; }

bool empty() const { return finish == start; }

// 定义在__deque_iterator
reference operator*() const { return *cur; }

reference operator->() const { return &(operator*()); }

difference_type operator-(const self& x) const {
  return difference_type(buffer_size()) * (node - x.node - 1) + 
         (cur - first) + (x.last - x.cur);
}

void set_node(map_pointer new_node) {
  node = new_node;
  first = *new_node;
  last = first + difference_type(buffer_size());
}

self& operator++() {
  ++cur;                // 切换到下一个元素
  if(cur == node) {     // 如果到达buffer的尾端
    set_node(node + 1); // 调至下一个buffer
    cur = first;        // 的前端
  }
  return *this;
}

self operator++(int) {
  self tmp = *this;
  ++*this;
  return tmp;
}

self& operator--() {
  if (cur == first) {    // 如果目前在buffer的起点
    set_node(node - 1);  // 调至前一个buffer
    cur = last;          // 的末端
  }
  --cur;                 // 往前移一个元素
  return *this;
}

self operator--(int) {
  self tmp = *this;
  --*this;
  returm tmp;
}

self &operator+=(difference_type n) {
  difference_type offset = n + (cur + first);
  if (offset >= 0 && offset < difference_type(buffer_size())) // 目标位置在同一级buffer
    cur += n;
  else { // 目标位置不在同一级buffer
    difference_type = node_offset = offset > 0
                                    ? offset / difference_type(buffer_size())
                                    : - difference_type((- offset - 1) / buffer_size()) - 1;
    set_node(node + node_offset);  // 切换至正确的buffer
    cur = first + (offset - node_offset * difference(buffer_size())); // 切换至正确的元素
  }
  return *this;
}

self operator+(difference_type n) const {
  self tmp = *this;
  return tmp += n;
}

self& operator-=(difference_type n) { return *this += -n; }

self operator-(difference_type n) const {
  self tmp = *this;
  return tmp -= n;
}
reference operator[](difference_type n) const { return *(*this + n); }
```

## 适配器queue & stack

- queue
  ```
  template <class T, class Sequence=deque<T>>
  class queue {
  // ...
   public:
    typedef typename Sequence::value_type      value_type;
    typedef typename Sequence::size_type       size_type
    typedef typename Sequence::reference       reference
    typedef typename Sequence::const_reference const_reference
   protected:
    Sequence c;
   public:
    bool empty() { return c.empty(); }
    size_type size() const { return c.size(); }
    reference front() { return c.front(); }
    const_reference front() const { return c.front(); }
    reference back() { return c.back(); }
    const reference back() { return c.back(); }
    void push(const value_type& x) { c.push_back(x); }
    void pop() { c.pop_front(); }
  };
  ```
- stack

  ```
  template <class T, class Sequence=deque<T>>
  class stack {
  // ...
   public:
    typedef typename Sequence::value_type      value_type;
    typedef typename Sequence::size_type       size_type
    typedef typename Sequence::reference       reference
    typedef typename Sequence::const_reference const_reference
   protected:
    Sequence c;
   public:
    bool empty() const { return c.empty(); }
    size_type size() const { return c.size(); }
    reference top() { return c.back(); }
    const reference top() const { reutrn c.back(); }
    void push(const value_type& x) { c.push_back(x); }
    void pop() { c.pop_back(); }
  };
  ```
- stack和queue都不允许遍历，也不提供iterator
- stack和queue都可选择list或deque作为底层结构
- queue不可以选择vector作为底层结构
- stack可以选择vector作为底层结构
- stack和queue都不可选择set或者map作为底层结构