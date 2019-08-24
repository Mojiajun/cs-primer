# GCC5.1 STL源码分析

## Iterators
### Iterator Tags
Iterator Tags标志类迭代器能够进行的操作，比如只能前向移动，或双向移动，或随机访问
```
/// @@file bits/stl_iterator_base_types.h
 70 namespace std _GLIBCXX_VISIBILITY(default)
 71 {
 72 _GLIBCXX_BEGIN_NAMESPACE_VERSION
 88   ///  Marking input iterators.
 89   struct input_iterator_tag { }; // 支持operator++
 90 
 91   ///  Marking output iterators.
 92   struct output_iterator_tag { }; // 支持operator++
 93 
 94   /// Forward iterators support a superset of input iterator operations.
 95   struct forward_iterator_tag : public input_iterator_tag { };  // 支持operator++
 96 
 97   /// Bidirectional iterators support a superset of forward iterator
 98   /// operations.  // 支持operator++ operator--
 99   struct bidirectional_iterator_tag : public forward_iterator_tag { };
100 
101   /// Random-access iterators support a superset of bidirectional
102   /// iterator operations.  // 支持operator++ operator-- p+n 等待
103   struct random_access_iterator_tag : public bidirectional_iterator_tag { };
```

### 通用Iterator
每个迭代器都必须提供5类信息：iterator_category、value_type、difference_type、pointer、reference。
第一个数迭代器本身的信息，表示iterator_tag，标识迭代器本身的操作类型，后四种标志迭代器所指数据的相关信息。
```
116   template<typename _Category, typename _Tp, typename _Distance = ptrdiff_t,
117            typename _Pointer = _Tp*, typename _Reference = _Tp&>
118     struct iterator
119     {
120       /// One of the @link iterator_tags tag types@endlink.
121       typedef _Category  iterator_category;
122       /// The type "pointed to" by the iterator.
123       typedef _Tp        value_type;
124       /// Distance between iterators is represented as this type.
125       typedef _Distance  difference_type;
126       /// This type represents a pointer-to-value_type.
127       typedef _Pointer   pointer;
128       /// This type represents a reference-to-value_type.
129       typedef _Reference reference;
130     };
```

###  Iterator Traits
标志迭代器所指数据的相关信息（比如数据类型、引用类型）以及迭代器自身信息（tags）。主要是为了使迭代器和指针能够兼容，比如说标准库算法传入迭代器和传入指针都可以。手段是使用类模板的偏特化（Partial Specialization），偏特化指“针对（任何）template参数更进一步的条件限制所设计出来的一个特化版本”。其本身仍然是类模板，只是模板参数的类型受到了限制。
```
/// @@file bits/stl_iterator_base_types.h
143   template<typename _Iterator, typename = __void_t<>>
144     struct __iterator_traits { };
145 
146   template<typename _Iterator>
147     struct __iterator_traits<_Iterator,
148                  __void_t<typename _Iterator::iterator_category,
149                       typename _Iterator::value_type,
150                       typename _Iterator::difference_type,
151                       typename _Iterator::pointer,
152                       typename _Iterator::reference>>
153     {
154       typedef typename _Iterator::iterator_category iterator_category;
155       typedef typename _Iterator::value_type        value_type;
156       typedef typename _Iterator::difference_type   difference_type;
157       typedef typename _Iterator::pointer           pointer;
158       typedef typename _Iterator::reference         reference;
159     };
160 
161   template<typename _Iterator>
162     struct iterator_traits
163     : public __iterator_traits<_Iterator> { };

176   /// Partial specialization for pointer types.
177   template<typename _Tp> 
178     struct iterator_traits<_Tp*> // 接受指针
179     {
180       typedef random_access_iterator_tag iterator_category;
181       typedef _Tp                         value_type;
182       typedef ptrdiff_t                   difference_type;
183       typedef _Tp*                        pointer;
184       typedef _Tp&                        reference;
185     };
186 
187   /// Partial specialization for const pointer types.
188   template<typename _Tp>
189     struct iterator_traits<const _Tp*> // 接受引用
190     {
191       typedef random_access_iterator_tag iterator_category;
192       typedef _Tp                         value_type;
193       typedef ptrdiff_t                   difference_type;
194       typedef const _Tp*                  pointer;
195       typedef const _Tp&                  reference;
196     };

202   template<typename _Iter>
203     inline typename iterator_traits<_Iter>::iterator_category
204     __iterator_category(const _Iter&)
205     { return typename iterator_traits<_Iter>::iterator_category(); }
```

### 迭代器适配器
#### `__gnu_cxx::__normal_iterator`
该适配器不改变其迭代器的任何操作，主要的目的是将一个指针这样的“迭代器”转换成STL迭代器。指针支持的操作，该适配器都支持。
```
/// @@file bits/stl_iterator.h
707 namespace __gnu_cxx _GLIBCXX_VISIBILITY(default)
708 {
718   using std::iterator_traits;
719   using std::iterator;
720   template<typename _Iterator, typename _Container>
721     class __normal_iterator
722     {
723     protected:
724       _Iterator _M_current;
725 
726       typedef iterator_traits<_Iterator>        __traits_type;
727 
728     public:
729       typedef _Iterator                 iterator_type;
730       typedef typename __traits_type::iterator_category iterator_category;
731       typedef typename __traits_type::value_type    value_type;
732       typedef typename __traits_type::difference_type   difference_type;
733       typedef typename __traits_type::reference     reference;
734       typedef typename __traits_type::pointer       pointer;
735 
736       _GLIBCXX_CONSTEXPR __normal_iterator() _GLIBCXX_NOEXCEPT
737       : _M_current(_Iterator()) { }
738 
739       explicit
740       __normal_iterator(const _Iterator& __i) _GLIBCXX_NOEXCEPT
741       : _M_current(__i) { }
742 
743       // Allow iterator to const_iterator conversion
744       template<typename _Iter>
745         __normal_iterator(const __normal_iterator<_Iter,
746               typename __enable_if<
747                (std::__are_same<_Iter, typename _Container::pointer>::__value),
748               _Container>::__type>& __i) _GLIBCXX_NOEXCEPT
749         : _M_current(__i.base()) { }
750  // .....
936 } // namespace
```

#### `std::reverse_iterator`
双向和随机访问的迭代器可以使用此适配器，目的是改变迭代器的移动方向。该适配器的移动方向其底层装饰的迭代器的移动方向相反。值得注意的是，它的解引用操作的是先移动，在求解。因为迭代器范围是前闭后开区间，最后的迭代器指向的是最后一个元素的下一个位置。
```
/// @@file bits/stl_iterator.h
 68 namespace std _GLIBCXX_VISIBILITY(default)
 69 {
 96   template<typename _Iterator>
 97     class reverse_iterator
 98     : public iterator<typename iterator_traits<_Iterator>::iterator_category,
 99               typename iterator_traits<_Iterator>::value_type,
100               typename iterator_traits<_Iterator>::difference_type,
101               typename iterator_traits<_Iterator>::pointer,
102                       typename iterator_traits<_Iterator>::reference>
103     {
104     protected:
105       _Iterator current;
106 
107       typedef iterator_traits<_Iterator>        __traits_type;
108 
109     public:
110       typedef _Iterator                 iterator_type;
111       typedef typename __traits_type::difference_type   difference_type;
112       typedef typename __traits_type::pointer       pointer;
113       typedef typename __traits_type::reference     reference;
114 
115       /**
116        *  The default constructor value-initializes member @p current.
117        *  If it is a pointer, that means it is zero-initialized.
118       */
119       // _GLIBCXX_RESOLVE_LIB_DEFECTS
120       // 235 No specification of default ctor for reverse_iterator
121       reverse_iterator() : current() { }
122 
123       /**
124        *  This %iterator will move in the opposite direction that @p x does.
125       */
126       explicit
127       reverse_iterator(iterator_type __x) : current(__x) { }
128 
129       /**
130        *  The copy constructor is normal.
131       */
132       reverse_iterator(const reverse_iterator& __x)
133       : current(__x.current) { }
134 
135       /**
136        *  A %reverse_iterator across other types can be copied if the
137        *  underlying %iterator can be converted to the type of @c current.
138       */
139       template<typename _Iter>
140         reverse_iterator(const reverse_iterator<_Iter>& __x)
141     : current(__x.base()) { }
160       reference
161       operator*() const
162       {
163     _Iterator __tmp = current;
164     return *--__tmp; // 先减1
165       }

397   template<typename _Iterator>
398     inline reverse_iterator<_Iterator>
399     make_reverse_iterator(_Iterator __i)
400     { return reverse_iterator<_Iterator>(__i); }
```

#### `std::back_insert_iterator`
底层是容器，将对`back_insert_iterator`的赋值操作转变为对容器`push_back`的操作。此外，只有`operator++()`、`operator++(int)`和`operator*`操作。
```
/// @@file bits/stl_iterator.h
414   template<typename _Container>
415     class back_insert_iterator
416     : public iterator<output_iterator_tag, void, void, void, void>
417     {
418     protected:
419       _Container* container;
420 
421     public:
422       /// A nested typedef for the type of whatever container you used.
423       typedef _Container          container_type;
424 
425       /// The only way to create this %iterator is with a container.
426       explicit
427       back_insert_iterator(_Container& __x) : container(&__x) { }

448       back_insert_iterator&
449       operator=(const typename _Container::value_type& __value)
450       {
451     container->push_back(__value);
452     return *this;
453       }

490   template<typename _Container>
491     inline back_insert_iterator<_Container>
492     back_inserter(_Container& __x)
493     { return back_insert_iterator<_Container>(__x); }
```

#### `std::front_insert_iterator`
底层是容器，将对`front_insert_iterator`的赋值操作转变为对容器`push_front`的操作。此外，只有`operator++()`、`operator++(int)`和`operator*`操作。
```
/// @@file bits/stl_iterator.h
505   template<typename _Container>
506     class front_insert_iterator
507     : public iterator<output_iterator_tag, void, void, void, void>
508     {
509     protected:
510       _Container* container;
511 
512     public:
513       /// A nested typedef for the type of whatever container you used.
514       typedef _Container          container_type;
515 
516       /// The only way to create this %iterator is with a container.
517       explicit front_insert_iterator(_Container& __x) : container(&__x) { }

538       front_insert_iterator&
539       operator=(const typename _Container::value_type& __value)
540       {
541     container->push_front(__value);
542     return *this;
543       }

580   template<typename _Container>
581     inline front_insert_iterator<_Container>
582     front_inserter(_Container& __x)
583     { return front_insert_iterator<_Container>(__x); }
```

#### `std::insert_iterator`
底层是容器，将对`insert_iterator`的赋值操作转变为对容器`insert`的操作，赋值完成后，`insert_iterator`中的`iter`迭代器指向新插入元素的下一个元素。此外，只有`operator++()`、`operator++(int)`和`operator*`操作。
```
599   template<typename _Container>
600     class insert_iterator
601     : public iterator<output_iterator_tag, void, void, void, void>
602     {
603     protected:
604       _Container* container;
605       typename _Container::iterator iter;
606 
607     public:
608       /// A nested typedef for the type of whatever container you used.
609       typedef _Container          container_type;
610 
611       /**
612        *  The only way to create this %iterator is with a container and an
613        *  initial position (a normal %iterator into the container).
614       */
615       insert_iterator(_Container& __x, typename _Container::iterator __i)
616       : container(&__x), iter(__i) {}

650       insert_iterator&
651       operator=(const typename _Container::value_type& __value)
652       {
653     iter = container->insert(iter, __value);
654     ++iter; // 移动
655     return *this;
656       }

694   template<typename _Container, typename _Iterator>
695     inline insert_iterator<_Container>
696     inserter(_Container& __x, _Iterator __i)
697     {
698       return insert_iterator<_Container>(__x,
699                      typename _Container::iterator(__i));
700     }

```

#### `std::move_iterator`
除了它的`[]`运算符隐式地将底层迭代器的解引用运算符返回的值转换为右值引用，其他具有与底层迭代器相同的行为。可以使用移动迭代器调用一些通用算法来替换使用移动的复制。
```
/// @@file bits/stl_iterator.h
 940 namespace std _GLIBCXX_VISIBILITY(default)
 941 {
 958   template<typename _Iterator>
 959     class move_iterator
 960     {
 961     protected:
 962       _Iterator _M_current;
 963 
 964       typedef iterator_traits<_Iterator>        __traits_type;
 965       typedef typename __traits_type::reference     __base_ref;
 966 
 967     public:
 968       typedef _Iterator                 iterator_type;
 969       typedef typename __traits_type::iterator_category iterator_category;
 970       typedef typename __traits_type::value_type    value_type;
 971       typedef typename __traits_type::difference_type   difference_type;
 972       // NB: DR 680.
 973       typedef _Iterator                 pointer;
 974       // _GLIBCXX_RESOLVE_LIB_DEFECTS
 975       // 2106. move_iterator wrapping iterators returning prvalues
 976       typedef typename conditional<is_reference<__base_ref>::value,
 977              typename remove_reference<__base_ref>::type&&,
 978              __base_ref>::type      reference;
 979 
 980       move_iterator()
 981       : _M_current() { }
 982 
 983       explicit
 984       move_iterator(iterator_type __i)
 985       : _M_current(__i) { }
 986 
 987       template<typename _Iter>
 988     move_iterator(const move_iterator<_Iter>& __i)
 989     : _M_current(__i.base()) { }

1055       reference
1056       operator[](difference_type __n) const
1057       { return std::move(_M_current[__n]); } // 返回右值
1058     };

1156   template<typename _Iterator>
1157     inline move_iterator<_Iterator>
1158     make_move_iterator(_Iterator __i)
1159     { return move_iterator<_Iterator>(__i); } // 返回适配器

1172 } // namespace
```

### 迭代器算法
#### `std::distance`
返回两个迭代器的距离n（__first + n == __last），会根据迭代器的类型，调用高效的算法。对于随机访问的迭代器，可在常数时间内完成。其他迭代器具有线性时间复杂度。
```
/// @@file bits/stl_iterator_base_funcs.h
112   template<typename _InputIterator>
113     inline typename iterator_traits<_InputIterator>::difference_type
114     distance(_InputIterator __first, _InputIterator __last)
115     {
116       // concept requirements -- taken care of in __distance
117       return std::__distance(__first, __last,
118                  std::__iterator_category(__first));
119     }
```
对于随机访问迭代器，可以`__last - __first`
```
/// @@file bits/stl_iterator_base_funcs.h
88   template<typename _RandomAccessIterator>
89     inline typename iterator_traits<_RandomAccessIterator>::difference_type
90     __distance(_RandomAccessIterator __first, _RandomAccessIterator __last,
91                random_access_iterator_tag)
92     {
93       // concept requirements
94       __glibcxx_function_requires(_RandomAccessIteratorConcept<
95                   _RandomAccessIterator>)
96       return __last - __first;
97     }
```
但是，其他类型的迭代器，只能通过移动判断的方式
```
/// @@file bits/stl_iterator_base_funcs.h
71   template<typename _InputIterator>
72     inline typename iterator_traits<_InputIterator>::difference_type
73     __distance(_InputIterator __first, _InputIterator __last,
74                input_iterator_tag)
75     {
76       // concept requirements
77       __glibcxx_function_requires(_InputIteratorConcept<_InputIterator>)
78 
79       typename iterator_traits<_InputIterator>::difference_type __n = 0;
80       while (__first != __last)
81     {
82       ++__first;
83       ++__n;
84     }
85       return __n;
86     }
```

#### `std::advance`
将迭代器移动n个距离。
```
/// @@file bits/stl_iterator_base_funcs.h
171   template<typename _InputIterator, typename _Distance>
172     inline void
173     advance(_InputIterator& __i, _Distance __n)
174     {
175       // concept requirements -- taken care of in __advance
176       typename iterator_traits<_InputIterator>::difference_type __d = __n;
177       std::__advance(__i, __d, std::__iterator_category(__i));
178     }
```
随机访问类型的迭代器直接用加法
```
/// @@file bits/stl_iterator_base_funcs.h
148   template<typename _RandomAccessIterator, typename _Distance>
149     inline void
150     __advance(_RandomAccessIterator& __i, _Distance __n,
151               random_access_iterator_tag)
152     {
153       // concept requirements
154       __glibcxx_function_requires(_RandomAccessIteratorConcept<
155                   _RandomAccessIterator>)
156       __i += __n;
157     }
```
其他类型的迭代器用移动判断的方式
```
/// @@file bits/stl_iterator_base_funcs.h
121   template<typename _InputIterator, typename _Distance>
122     inline void
123     __advance(_InputIterator& __i, _Distance __n, input_iterator_tag)
124     {
125       // concept requirements
126       __glibcxx_function_requires(_InputIteratorConcept<_InputIterator>)
127       _GLIBCXX_DEBUG_ASSERT(__n >= 0);
128       while (__n--)
129     ++__i;
130     }
131 
132   template<typename _BidirectionalIterator, typename _Distance>
133     inline void
134     __advance(_BidirectionalIterator& __i, _Distance __n,
135           bidirectional_iterator_tag)
136     {
137       // concept requirements
138       __glibcxx_function_requires(_BidirectionalIteratorConcept<
139                   _BidirectionalIterator>)
140       if (__n > 0)
141         while (__n--)
142       ++__i;
143       else
144         while (__n++)
145       --__i;
146     }
```

#### `std::next`和`std::prev`
```
/// @@file bits/stl_iterator_base_funcs.h
182   template<typename _ForwardIterator>
183     inline _ForwardIterator
184     next(_ForwardIterator __x, typename
185      iterator_traits<_ForwardIterator>::difference_type __n = 1)
186     {
187       std::advance(__x, __n);
188       return __x;
189     }
190 
191   template<typename _BidirectionalIterator>
192     inline _BidirectionalIterator
193     prev(_BidirectionalIterator __x, typename
194      iterator_traits<_BidirectionalIterator>::difference_type __n = 1)
195     {
196       std::advance(__x, -__n);
197       return __x;
198     }
```

## Allocators
### `std::allocator<_Tp>`
定义`__allocator_base`，定义为`__gnu_cxx::new_allocator<_Tp>`的别名
```
namespace std {
/// @@file bits/c++allocator.h
35 #if __cplusplus >= 201103L
36 namespace std
37 {
47   template<typename _Tp>
48     using __allocator_base = __gnu_cxx::new_allocator<_Tp>;
49 }
```

定义`allocator<_Tp>`。仅仅继承`__allocator_base<_Tp>`，没有过多的修改
```
/// @@file bits/allocator.h
 52 namespace std _GLIBCXX_VISIBILITY(default)
 61   /// allocator<void> specialization.
 62   template<>
 63     class allocator<void> // 是一个空类
 64     {
 65     public:
 66       typedef size_t      size_type;
 67       typedef ptrdiff_t   difference_type;
 68       typedef void*       pointer;
 69       typedef const void* const_pointer;
 70       typedef void        value_type;
 71 
 72       template<typename _Tp1>
 73         struct rebind
 74         { typedef allocator<_Tp1> other; };
 75 
 76 #if __cplusplus >= 201103L
 77       // _GLIBCXX_RESOLVE_LIB_DEFECTS
 78       // 2103. std::allocator propagate_on_container_move_assignment
 79       typedef true_type propagate_on_container_move_assignment;
 80 #endif
 81     };
 91   template<typename _Tp>
 92     class allocator: public __allocator_base<_Tp>
 93     {
 94    public:
 95       typedef size_t     size_type;
 96       typedef ptrdiff_t  difference_type;
 97       typedef _Tp*       pointer;
 98       typedef const _Tp* const_pointer;
 99       typedef _Tp&       reference;
100       typedef const _Tp& const_reference;
101       typedef _Tp        value_type;
102 
103       template<typename _Tp1>
104         struct rebind
105         { typedef allocator<_Tp1> other; };
106 
107 #if __cplusplus >= 201103L
108       // _GLIBCXX_RESOLVE_LIB_DEFECTS
109       // 2103. std::allocator propagate_on_container_move_assignment
110       typedef true_type propagate_on_container_move_assignment;
111 #endif
112 
113       allocator() throw() { }
114 
115       allocator(const allocator& __a) throw()
116       : __allocator_base<_Tp>(__a) { }
117 
118       template<typename _Tp1>
119         allocator(const allocator<_Tp1>&) throw() { }
120 
121       ~allocator() throw() { }
122 
123       // Inherit everything else.
124     };
}
```

### `__gnu_cxx::new_allocator<_Tp>`
分配和释放分别调用`::operator new`和`::operator delete`，没有接手内存管理
```
/// @@file ext/new_allocator.h
 40 namespace __gnu_cxx _GLIBCXX_VISIBILITY(default)
 41 {
 42 _GLIBCXX_BEGIN_NAMESPACE_VERSION
 43 
 44   using std::size_t;
 45   using std::ptrdiff_t;
 57   template<typename _Tp>
 58     class new_allocator
 59     {
 60     public:
 61       typedef size_t     size_type;
 62       typedef ptrdiff_t  difference_type;
 63       typedef _Tp*       pointer;
 64       typedef const _Tp* const_pointer;
 65       typedef _Tp&       reference;
 66       typedef const _Tp& const_reference;
 67       typedef _Tp        value_type;
 68 
 69       template<typename _Tp1>
 70         struct rebind
 71         { typedef new_allocator<_Tp1> other; };
 72 
 73 #if __cplusplus >= 201103L
 74       // _GLIBCXX_RESOLVE_LIB_DEFECTS
 75       // 2103. propagate_on_container_move_assignment
 76       typedef std::true_type propagate_on_container_move_assignment;
 77 #endif
 78 
 79       new_allocator() _GLIBCXX_USE_NOEXCEPT { }
 80 
 81       new_allocator(const new_allocator&) _GLIBCXX_USE_NOEXCEPT { }
 82 
 83       template<typename _Tp1>
 84         new_allocator(const new_allocator<_Tp1>&) _GLIBCXX_USE_NOEXCEPT { }
 85 
 86       ~new_allocator() _GLIBCXX_USE_NOEXCEPT { }
 87 
 88       pointer
 89       address(reference __x) const _GLIBCXX_NOEXCEPT
 90       { return std::__addressof(__x); }
 91 
 92       const_pointer
 93       address(const_reference __x) const _GLIBCXX_NOEXCEPT
 94       { return std::__addressof(__x); }
 95 
 96       // NB: __n is permitted to be 0.  The C++ standard says nothing
 97       // about what the return value is when __n == 0.
 98       pointer
 99       allocate(size_type __n, const void* = 0)
100       { 
101         if (__n > this->max_size())
102           std::__throw_bad_alloc();
103 
104         return static_cast<_Tp*>(::operator new(__n * sizeof(_Tp)));
105       }
106 
107       // __p is not permitted to be a null pointer.
108       void
109       deallocate(pointer __p, size_type)
110       { ::operator delete(__p); }
111 
112       size_type
113       max_size() const _GLIBCXX_USE_NOEXCEPT
114       { return size_t(-1) / sizeof(_Tp); }
115 
116 #if __cplusplus >= 201103L
117       template<typename _Up, typename... _Args>
118         void
119         construct(_Up* __p, _Args&&... __args)
120         { ::new((void *)__p) _Up(std::forward<_Args>(__args)...); }
121 
122       template<typename _Up>
123         void 
124         destroy(_Up* __p) { __p->~_Up(); }
125 #else
126       // _GLIBCXX_RESOLVE_LIB_DEFECTS
127       // 402. wrong new expression in [some_] allocator::construct
128       void 
129       construct(pointer __p, const _Tp& __val) 
130       { ::new((void *)__p) _Tp(__val); }
131 
132       void 
133       destroy(pointer __p) { __p->~_Tp(); }
134 #endif
135     };
```
总结：标准库的分配器`std::allocator<_Tp>`的分配和释放对象最终分别调用`::operator new`和`::operator delete`，本身没有内存管理。具有内存管理的分配器是`__gnu_cxx::__pool_alloc<_Tp>`。此外`std::allocator`将内存申请、构造、析构和内存释放分开，在内存释放前需要考虑析构函数，不然会引起内存泄漏。

### 内部构造工具
#### `std::_Construct`
底层调用`placement new`，在指针所指的内存显式调用构造函数构造对象
```
/// @file bits/stl_construct.h
72   template<typename _T1, typename... _Args>
73     inline void
74     _Construct(_T1* __p, _Args&&... __args)
75     { ::new(static_cast<void*>(__p)) _T1(std::forward<_Args>(__args)...); }
```
`placement new`简单地将传入指针返回，没有分配内存的操作
```
/// @file new
// Default placement versions of operator new.
inline void* operator new(std::size_t, void* __p) _GLIBCXX_USE_NOEXCEPT
{ return __p; }
```
用`placement new`可以显式的调用构造函数，比如
```
void* ptr = malloc(sizeof(int));
::new(ptr)int(10);
```

#### `std::_Destroy`
##### 单个对象，直接调用析构函数，没有内存释放的操作
```
/// @file bits/stl_construct.h
90   template<typename _Tp>
91     inline void
92     _Destroy(_Tp* __pointer)
93     { __pointer->~_Tp(); }
```

##### 如果是多个对象，视数据类型不同，操作不同：
1. 不借助分配器
```
/// @file bits/stl_construct.h
120   template<typename _ForwardIterator>
121     inline void
122     _Destroy(_ForwardIterator __first, _ForwardIterator __last)
123     {
124       typedef typename iterator_traits<_ForwardIterator>::value_type
125                        _Value_type;
126       std::_Destroy_aux<__has_trivial_destructor(_Value_type)>::
127     __destroy(__first, __last);
128     }
```
其中`_Destroy_aux`是一个模板类，会根据`_Value_type`对象是否有`trivival`析构函数（析构函数是`trivival`可以理解为可以用一个默认析构函数替代。即使一个空类，其析构函数中调用某个函数，其也不是trival的，因为没法用默认析构函数替代）调用不同实例。如果待析构对象的析构函数是`trivial`的化，什么也不做，不然的化，逐一调用析构函数
```
/// @file bits/stl_construct.h
 90   template<typename _Tp>
 91     inline void
 92     _Destroy(_Tp* __pointer)
 93     { __pointer->~_Tp(); }
 94 
 95   template<bool>
 96     struct _Destroy_aux
 97     {
 98       template<typename _ForwardIterator>
 99         static void
100         __destroy(_ForwardIterator __first, _ForwardIterator __last)
101     { // 逐一析构
102       for (; __first != __last; ++__first)
103         std::_Destroy(std::__addressof(*__first));
104     }
105     };
106 
107   template<>
108     struct _Destroy_aux<true>
109     {
110       template<typename _ForwardIterator>
111         static void
112         __destroy(_ForwardIterator, _ForwardIterator) { } // 什么也不做
113     };
```
- 借助分配器
```
/// @file bits/stl_construct.h
136   template<typename _ForwardIterator, typename _Allocator>
137     void
138     _Destroy(_ForwardIterator __first, _ForwardIterator __last,
139          _Allocator& __alloc)
140     {
141       typedef __gnu_cxx::__alloc_traits<_Allocator> __traits;
142       for (; __first != __last; ++__first)
143     __traits::destroy(__alloc, std::__addressof(*__first)); // 逐一析构
144     }
145 
146   template<typename _ForwardIterator, typename _Tp>
147     inline void
148     _Destroy(_ForwardIterator __first, _ForwardIterator __last,
149          allocator<_Tp>&)
150     {
151       _Destroy(__first, __last); // 会有优化
152     }
```
对于没有使用标准分配器(`std::allocator`)的，无论其构造函数是否是`trivial`的，不做优化，都逐一调用析构函数。比如说如果使用`vector<int, __gnu_cxx::__pool_alloc<int>> vec`，在析构的时候就调用`__gnu_cxx::__pool_alloc<int>`

### allocator算法
#### `uninitialized_copy(first, last, d_first)`
和`copy`类似，不过不需要目的地址是已经构造的内存。将元素从[first, last)拷贝到d_first所指的未构造的内存中。如果拷贝过程中抛出异常，所有已拷贝的对象都会析构。
```
/// @file bits/stl_uninitialized.h
105   template<typename _InputIterator, typename _ForwardIterator>
106     inline _ForwardIterator
107     uninitialized_copy(_InputIterator __first, _InputIterator __last,
108                _ForwardIterator __result)
109     {
110       typedef typename iterator_traits<_InputIterator>::value_type
111     _ValueType1;
112       typedef typename iterator_traits<_ForwardIterator>::value_type
113     _ValueType2;
114 #if __cplusplus < 201103L
115       const bool __assignable = true;
116 #else
117       // trivial types can have deleted assignment
118       typedef typename iterator_traits<_InputIterator>::reference _RefType1;
119       typedef typename iterator_traits<_ForwardIterator>::reference _RefType2;
120       const bool __assignable = is_assignable<_RefType2, _RefType1>::value;
121 #endif
122 
123       return std::__uninitialized_copy<__is_trivial(_ValueType1)
124                        && __is_trivial(_ValueType2)
125                        && __assignable>::
126     __uninit_copy(__first, __last, __result);
127     }
```
`__uninitialized_copy`会根据元素的类型具体执行，如果不是`TrivalType`，会逐一执行构造函数。
```
/// @file bits/stl_uninitialized.h
63   template<bool _TrivialValueTypes>
64     struct __uninitialized_copy
65     {
66       template<typename _InputIterator, typename _ForwardIterator>
67         static _ForwardIterator
68         __uninit_copy(_InputIterator __first, _InputIterator __last,
69                       _ForwardIterator __result)
70         {
71           _ForwardIterator __cur = __result;
72           __try
73             {
74               for (; __first != __last; ++__first, ++__cur)
75                 std::_Construct(std::__addressof(*__cur), *__first);
76               return __cur;
77             }
78           __catch(...)
79             {
80               std::_Destroy(__result, __cur);
81               __throw_exception_again;
82             }
83         }
84     };
```
如果是`TrivalType`，直接用`std::copy`拷贝
```
/// @file bits/stl_uninitialized.h
86   template<>
87     struct __uninitialized_copy<true>
88     {
89       template<typename _InputIterator, typename _ForwardIterator>
90         static _ForwardIterator
91         __uninit_copy(_InputIterator __first, _InputIterator __last,
92                       _ForwardIterator __result)
93         { return std::copy(__first, __last, __result); }
94     };
```
`std::copy`会进行优化，包括运用移动`std::move`，或者调用更高效的`memmove`（memmove能够保证源串在被覆盖之前将重叠区域的字节拷贝到目标区域中，但复制后源内容会被更改）

#### `uninitialized_copy_n(first, n, d_first)`
从`first`拷贝n个元素到`d_first`
```
/// @file bits/stl_uninitialized.h
677   template<typename _InputIterator, typename _Size, typename _ForwardIterator>
678     inline _ForwardIterator
679     uninitialized_copy_n(_InputIterator __first, _Size __n,
680                          _ForwardIterator __result)
681     { return std::__uninitialized_copy_n(__first, __n, __result,
682                                          std::__iterator_category(__first)); }
```
`__uninitialized_copy_n`会根据迭代器类型进行优化，如果是随机访问迭代器，直接调用`uninitialized_copy(first, first+n, d_first)`
```
/// @file bits/stl_uninitialized.h
660   template<typename _RandomAccessIterator, typename _Size,
661            typename _ForwardIterator>
662     inline _ForwardIterator
663     __uninitialized_copy_n(_RandomAccessIterator __first, _Size __n,
664                            _ForwardIterator __result,
665                            random_access_iterator_tag)
666     { return std::uninitialized_copy(__first, __first + __n, __result); }
```
其他迭代器，逐一拷贝
```
/// @file bits/stl_uninitialized.h
640   template<typename _InputIterator, typename _Size,
641            typename _ForwardIterator>
642     _ForwardIterator
643     __uninitialized_copy_n(_InputIterator __first, _Size __n,
644                            _ForwardIterator __result, input_iterator_tag)
645     {
646       _ForwardIterator __cur = __result;
647       __try
648         {
649           for (; __n > 0; --__n, ++__first, ++__cur)
650             std::_Construct(std::__addressof(*__cur), *__first);
651           return __cur;
652         }
653       __catch(...)
654         {
655           std::_Destroy(__result, __cur);
656           __throw_exception_again;
657         }
658     }
```

#### `uninitialized_fill(first, last, value)`
在[first, last)指定的原始内存中构造对象，对象的值为value。如果构造过程中抛出异常，已构造的对象会析构
```
/// @file bits/stl_uninitialized.h
171   template<typename _ForwardIterator, typename _Tp>
172     inline void
173     uninitialized_fill(_ForwardIterator __first, _ForwardIterator __last,
174                        const _Tp& __x)
175     {
176       typedef typename iterator_traits<_ForwardIterator>::value_type
177         _ValueType;
178 #if __cplusplus < 201103L
179       const bool __assignable = true;
180 #else
181       // trivial types can have deleted assignment
182       const bool __assignable = is_copy_assignable<_ValueType>::value;
183 #endif
184 
185       std::__uninitialized_fill<__is_trivial(_ValueType) && __assignable>::
186         __uninit_fill(__first, __last, __x);
187     }
```
`__uninitialized_fill`会根据元素的类型具体执行，如果不是`TrivalType`，会逐一执行构造函数。
```
/// @file bits/stl_uninitialized.h
130   template<bool _TrivialValueType>
131     struct __uninitialized_fill
132     {
133       template<typename _ForwardIterator, typename _Tp>
134         static void
135         __uninit_fill(_ForwardIterator __first, _ForwardIterator __last,
136                       const _Tp& __x)
137         {
138           _ForwardIterator __cur = __first;
139           __try
140             {
141               for (; __cur != __last; ++__cur)
142                 std::_Construct(std::__addressof(*__cur), __x);
143             }
144           __catch(...)
145             {
146               std::_Destroy(__first, __cur);
147               __throw_exception_again;
148             }
149         }
150     };
```
如果是`TrivalType`，直接调用`std::fill`
```
/// @file bits/stl_uninitialized.h
152   template<>
153     struct __uninitialized_fill<true>
154     {
155       template<typename _ForwardIterator, typename _Tp>
156         static void
157         __uninit_fill(_ForwardIterator __first, _ForwardIterator __last,
158                       const _Tp& __x)
159         { std::fill(__first, __last, __x); }
160     };
```
#### uninitialized_fill_n(first, n, value)
在[first, first+n)指定的原始内存中构造对象，对象的值为value
```
/// @file bits/stl_uninitialized.h
234   template<typename _ForwardIterator, typename _Size, typename _Tp>
235     inline _ForwardIterator
236     uninitialized_fill_n(_ForwardIterator __first, _Size __n, const _Tp& __x)
237     {
238       typedef typename iterator_traits<_ForwardIterator>::value_type
239         _ValueType;
240 #if __cplusplus < 201103L
241       const bool __assignable = true;
242 #else
243       // trivial types can have deleted assignment
244       const bool __assignable = is_copy_assignable<_ValueType>::value;
245 #endif
246       return __uninitialized_fill_n<__is_trivial(_ValueType) && __assignable>::
247         __uninit_fill_n(__first, __n, __x);
248     }
```
`__uninitialized_fill_n`根据元素是否是`TrivalType`进行优化，如果不是，逐一调用构造函数
```
/// @file bits/stl_uninitialized.h
190   template<bool _TrivialValueType>
191     struct __uninitialized_fill_n
192     {
193       template<typename _ForwardIterator, typename _Size, typename _Tp>
194         static _ForwardIterator
195         __uninit_fill_n(_ForwardIterator __first, _Size __n,
196                         const _Tp& __x)
197         {
198           _ForwardIterator __cur = __first;
199           __try
200             {
201               for (; __n > 0; --__n, ++__cur)
202                 std::_Construct(std::__addressof(*__cur), __x);
203               return __cur;
204             }
205           __catch(...)
206             {
207               std::_Destroy(__first, __cur);
208               __throw_exception_again;
209             }
210         }
211     };
```
否则，直接调用`std::fill_n`
```
/// @file bits/stl_uninitialized.h
213   template<>
214     struct __uninitialized_fill_n<true>
215     {
216       template<typename _ForwardIterator, typename _Size, typename _Tp>
217         static _ForwardIterator
218         __uninit_fill_n(_ForwardIterator __first, _Size __n,
219                         const _Tp& __x)
220         { return std::fill_n(__first, __n, __x); }
221     };
```

## 序列式容器
### array(C++11)
对C语言式数组的封装，定义时指定长度，定义后不能更改长度。与内置数据不同的是，数组名不是指向首元素地址。
```
/// @file include/array
80   template<typename _Tp, std::size_t _Nm>
81     struct array
82     {
83       typedef _Tp                                value_type;
84       typedef value_type*                        pointer;
85       typedef const value_type*                       const_pointer;
86       typedef value_type&                                reference;
87       typedef const value_type&                          const_reference;
88       typedef value_type*                                iterator;
89       typedef const value_type*                          const_iterator;
90       typedef std::size_t                                size_type;
91       typedef std::ptrdiff_t                             difference_type;
92       typedef std::reverse_iterator<iterator>            reverse_iterator;
93       typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
94 
95       // Support for zero-sized arrays mandatory.
96       typedef _GLIBCXX_STD_C::__array_traits<_Tp, _Nm> _AT_Type;
97       typename _AT_Type::_Type                         _M_elems;
```

### vector
动态数组，自动控制存储空间，根据需要进行扩展和收缩。通常拥有额外的空间以处理未来的增长，这样，每次插入元素时，不需要每次重新分配内存，仅在附加内存耗尽时才重新分配。可使用`capacity()`查看分配的内存总量，也可以调用`shrink_to_fit()`将额外的内存返回给系统。如果事先知道元素数量，可调用`reserve(n)`一次性分配内存。

#### Traits相关
```
/// @file bits/stl_vector.h
213   template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
214     class vector : protected _Vector_base<_Tp, _Alloc>
215     {
225     public:
226       typedef _Tp                                        value_type;
227       typedef typename _Base::pointer                    pointer;
228       typedef typename _Alloc_traits::const_pointer      const_pointer;
229       typedef typename _Alloc_traits::reference          reference;
230       typedef typename _Alloc_traits::const_reference    const_reference;
231       typedef __gnu_cxx::__normal_iterator<pointer, vector> iterator;
232       typedef __gnu_cxx::__normal_iterator<const_pointer, vector>
233       const_iterator;
234       typedef std::reverse_iterator<const_iterator>  const_reverse_iterator;
235       typedef std::reverse_iterator<iterator>            reverse_iterator;
236       typedef size_t                                     size_type;
237       typedef ptrdiff_t                                  difference_type;
238       typedef _Alloc                                     allocator_type;
```
提供的迭代器有两种，一种是封装指针的的`iterator`，一种是反向迭代器`reverse_iterator`。`_Vector_base`保存用三个指针`_M_start`、
`_M_finish`和`_M_end_of_storage`，标志一段连续内存区域。`_M_start`标志内存开始位置，`_M_end_of_storage`标志内存结束位置，而`_M_finish`标志存有有用数据的内存的结尾位置。

#### vector构造函数原型
构造时，内存分配的操作在`_Vector_base`中执行
```
vector();
explicit vector(const allocator_type& __a);\
explicit vector(size_type __n, const allocator_type& __a = allocator_type())
vector(size_type __n, const value_type& __value, const allocator_type& __a = allocator_type());
vector(const vector& __x);
vector(vector&& __x) noexcept;
vector(const vector& __x, const allocator_type& __a);
vector(vector&& __rv, const allocator_type& __m)
vector(initializer_list<value_type> __l, const allocator_type& __a = allocator_type())

template<typename _InputIterator,
typename = std::_RequireInputIter<_InputIterator>>
vector(_InputIterator __first, _InputIterator __last, const allocator_type& __a = allocator_type());
```

#### 析构函数
vector析构函数调用`std::_Destroy`，前面已经介绍，其行为跟元素类型以及使用的分配其有关.在默认情况下（使用标准分配器`std::allocator`）,如果元素`__has_trivial_destructor`（可以理解为可以用一个默认析构函数替代，即使一个空类，其析构函数中调用某个函数，其也不是trival的，因为没法用默认析构函数替代），什么也不做。否则逐一调用元素的析构函数，释放内存。如果没有使用标准分配器，一律逐一调用析构函数。如果存储的元素是指针，指针所指的对象不会被析构。
```
423       ~vector() _GLIBCXX_NOEXCEPT
424       { std::_Destroy(this->_M_impl._M_start, this->_M_impl._M_finish,
425                       _M_get_Tp_allocator()); }
```

#### 动态增长，以`push_back`为例
如果还有额外的内存（`_M_finish != _M_end_of_storage`），调用拷贝构造函数直接添加。
```
/// @file bits/stl_vector.h
913       push_back(const value_type& __x)
914       {
915         if (this->_M_impl._M_finish != this->_M_impl._M_end_of_storage)
916           {
917             _Alloc_traits::construct(this->_M_impl, this->_M_impl._M_finish,
918                                      __x);
919             ++this->_M_impl._M_finish;
920           }
921         else
922 #if __cplusplus >= 201103L
923           _M_emplace_back_aux(__x);
924 #else
925           _M_insert_aux(end(), __x);
926 #endif
927       }
```
否则，需要重新分配内存，然后将现有元素拷贝至新内存，再添加，其操作在`_M_emplace_back_aux(__x)`完成。

首先是重新分配一块容量更大的内存
```
/// @file bits/vector.tcc
404   template<typename _Tp, typename _Alloc>
405     template<typename... _Args>
406       void
407       vector<_Tp, _Alloc>::
408       _M_emplace_back_aux(_Args&&... __args)
409       {
410    const size_type __len =
411      _M_check_len(size_type(1), "vector::_M_emplace_back_aux");
412    pointer __new_start(this->_M_allocate(__len)); // 分配内存
413    pointer __new_finish(__new_start);
```
此次分配内存的大小通过跳用`_M_check_len`获取，定义如下
```
/// @file bits/stl_vector.h
1421       size_type
1422       _M_check_len(size_type __n, const char* __s) const
1423       {
1424         if (max_size() - size() < __n)
1425           __throw_length_error(__N(__s));
1426 
1427         const size_type __len = size() + std::max(size(), __n);
1428         return (__len < size() || __len > max_size()) ? max_size() : __len;
1429       }
```
可以看到，`_M_check_len(size_type(1), "vector::_M_emplace_back_aux")`返回值最小为1（初始为空，`size()`为零时），否则返回2倍于当前`size()`的值。

然后在新分配内存的正确位置处构造待添加元素，之后将元素拷贝到新分配的内存位置。在此过程中，如果有任何异常抛出，
```
/// @file bits/vector.tcc
414    __try
415      {
416        _Alloc_traits::construct(this->_M_impl, __new_start + size(),
417                                 std::forward<_Args>(__args)...);
418        __new_finish = pointer();
419 
420        __new_finish
421          = std::__uninitialized_move_if_noexcept_a
422          (this->_M_impl._M_start, this->_M_impl._M_finish,
423           __new_start, _M_get_Tp_allocator());
424 
425        ++__new_finish;
426      }
427    __catch(...)
428      {
429        if (!__new_finish)
430          _Alloc_traits::destroy(this->_M_impl, __new_start + size());
431        else
432          std::_Destroy(__new_start, __new_finish, _M_get_Tp_allocator());
433        _M_deallocate(__new_start, __len);
434        __throw_exception_again;
435      }
```
上面`Alloc_traits::construct`就是`placement new`的封装，`_Alloc_traits::destroy`是调用析构函数的封装。

最后调用`std::_Destroy`进行析构，然后调用`_M_deallocate`释放内存，，更改三个指针`_M_start`、
`_M_finish`和`_M_end_of_storage`
```
/// @file bits/vector.tcc
436    std::_Destroy(this->_M_impl._M_start, this->_M_impl._M_finish,
437                  _M_get_Tp_allocator());
438    _M_deallocate(this->_M_impl._M_start,
439                  this->_M_impl._M_end_of_storage
440                  - this->_M_impl._M_start);
441    this->_M_impl._M_start = __new_start;
442    this->_M_impl._M_finish = __new_finish;
443    this->_M_impl._M_end_of_storage = __new_start + __len;
444       }
```

#### `_Vector_base`
定义内存分配和释放相关的操作
```
/// @file bits/stl_vector.h
66 namespace std _GLIBCXX_VISIBILITY(default)
67 {
68 _GLIBCXX_BEGIN_NAMESPACE_CONTAINER
69 
70   /// See bits/stl_deque.h's _Deque_base for an explanation.
71   template<typename _Tp, typename _Alloc>
72     struct _Vector_base
73     {
74       typedef typename __gnu_cxx::__alloc_traits<_Alloc>::template
75         rebind<_Tp>::other _Tp_alloc_type;
76       typedef typename __gnu_cxx::__alloc_traits<_Tp_alloc_type>::pointer
77         pointer;
/// 省略构造函数
159       ~_Vector_base() _GLIBCXX_NOEXCEPT
160       { _M_deallocate(this->_M_impl._M_start, this->_M_impl._M_end_of_storage
161                       - this->_M_impl._M_start); }
162 
163     public:
164       _Vector_impl _M_impl;
165 
166       pointer
167       _M_allocate(size_t __n)
168       {
169         typedef __gnu_cxx::__alloc_traits<_Tp_alloc_type> _Tr;
170         return __n != 0 ? _Tr::allocate(_M_impl, __n) : pointer();
171       }
172 
173       void
174       _M_deallocate(pointer __p, size_t __n)
175       {
176         typedef __gnu_cxx::__alloc_traits<_Tp_alloc_type> _Tr;
177         if (__p)
178           _Tr::deallocate(_M_impl, __p, __n);
179       }
180 
181     private:
182       void
183       _M_create_storage(size_t __n) // 构造函数中调用，分配内存
184       {
185         this->_M_impl._M_start = this->_M_allocate(__n);
186         this->_M_impl._M_finish = this->_M_impl._M_start;
187         this->_M_impl._M_end_of_storage = this->_M_impl._M_start + __n;
188       }
189     };
```
`_Vector_impl`继承类内存分配器，并定义内存管理的三个指针`_M_start`、
`_M_finish`和`_M_end_of_storage`
```
79       struct _Vector_impl 
80       : public _Tp_alloc_type
81       {
82         pointer _M_start;
83         pointer _M_finish;
84         pointer _M_end_of_storage;
/// 省略
```
总结：`vector`默认使用标准分配器（`std::allocator`）,当分配内存用完，重新申请内存时，新申请的内存大小是当前内存大小的2倍（两倍增长）。将元素拷贝到新内存时，先将待添加的元素在新内存的正确位置构造好，再拷贝现有元素，最后进行析构和内存释放。元素的析构函数并不一定会执行。

### list
双向链表

#### Traits相关
```
/// @file bits/stl_list.h
506   template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
507     class list : protected _List_base<_Tp, _Alloc>
508     {
518     public:
519       typedef _Tp                                        value_type;
520       typedef typename _Tp_alloc_type::pointer           pointer;
521       typedef typename _Tp_alloc_type::const_pointer     const_pointer;
522       typedef typename _Tp_alloc_type::reference         reference;
523       typedef typename _Tp_alloc_type::const_reference   const_reference;
524       typedef _List_iterator<_Tp>                        iterator;
525       typedef _List_const_iterator<_Tp>                  const_iterator;
526       typedef std::reverse_iterator<const_iterator>      const_reverse_iterator;
527       typedef std::reverse_iterator<iterator>            reverse_iterator;
528       typedef size_t                                     size_type;
529       typedef ptrdiff_t                                  difference_type;
530       typedef _Alloc                                     allocator_type;
```
list是一个双向链表，可以正向和反向遍历。iterator是_List_iterator<_Tp>的别名，是一个双向迭代器。默认分配器是`std::allocator`

#### 构造函数
```
list();
explicit list(const Allocator& alloc);
explicit list(size_type count, const T& value = T(),
              const Allocator& alloc = Allocator());
list(size_type count, const T& value, const Allocator& alloc = Allocator());
explicit list(size_type count);
list(const list& other);
list(const list& other, const Allocator& alloc);
list(list&& other);
list(list&& other, const Allocator& alloc);
list(std::initializer_list<T> init, const Allocator& alloc = Allocator());

template< class InputIt >
list(InputIt first, InputIt last, const Allocator& alloc = Allocator() );
```

#### 析构函数
list类中没有数据成员，没有显式定义析构函数。数据成员在其基类`_List_base<_Tp, _Alloc>`中析构。
```
/// @file bits/stl_list.h
445       ~_List_base() _GLIBCXX_NOEXCEPT
446       { _M_clear(); }
```
`_List_base`的析构函数中调用`_M_clear()`进行清理。在`_M_clear()`里面，遍历链表`while (__cur != &_M_impl._M_node)`。对于每个结点，调用分配器（默认的分配器是`std::new_allocator`）的`destroy`将结点析构，然后调用`_M_put_node`释放内存。和vector不同的是，无论什么类型，list都会先执行析构函数，再释放内存。如果存储的元素是指针，指针所指的对象不会被析构。
```
/// @file bits/list.tcc
63   template<typename _Tp, typename _Alloc>
64     void
65     _List_base<_Tp, _Alloc>::
66     _M_clear() _GLIBCXX_NOEXCEPT
67     {
68       typedef _List_node<_Tp>  _Node;
69       __detail::_List_node_base* __cur = _M_impl._M_node._M_next;
70       while (__cur != &_M_impl._M_node)
71     {
72       _Node* __tmp = static_cast<_Node*>(__cur);
73       __cur = __tmp->_M_next;
74 #if __cplusplus >= 201103L
75       _M_get_Node_allocator().destroy(__tmp);
76 #else
77       _M_get_Tp_allocator().destroy(std::__addressof(__tmp->_M_data));
78 #endif
79       _M_put_node(__tmp);
80     }
81     }
```

#### `List_base`
主要接口有结点的分配`_M_get_node()`和释放`_M_put_node(_List_node<_Tp>* __p)`函数。此外定义计算两个结点距离的函数`_S_distance`。
```
/// @file bits/stl_list.h
297   template<typename _Tp, typename _Alloc>
298     class _List_base
299     {
300     protected:
314       typedef typename _Alloc::template rebind<_List_node<_Tp> >::other
315         _Node_alloc_type;
316 
317       typedef typename _Alloc::template rebind<_Tp>::other _Tp_alloc_type;
318 
319       static size_t
320       _S_distance(const __detail::_List_node_base* __first,
321                   const __detail::_List_node_base* __last)
322       {
323         size_t __n = 0;
324         while (__first != __last)
325           {
326             __first = __first->_M_next;
327             ++__n;
328           }
329         return __n;
330       }

356       _List_impl _M_impl;
390       _List_node<_Tp>*
391       _M_get_node()
392       { return _M_impl._Node_alloc_type::allocate(1); }
393 
394       void
395       _M_put_node(_List_node<_Tp>* __p) _GLIBCXX_NOEXCEPT
396       { _M_impl._Node_alloc_type::deallocate(__p, 1); }

445       ~_List_base() _GLIBCXX_NOEXCEPT
446       { _M_clear(); }
447 
448       void
449       _M_clear() _GLIBCXX_NOEXCEPT;
450 
451       void
452       _M_init() _GLIBCXX_NOEXCEPT
453       { // 初始化，头结点指向自己
454         this->_M_impl._M_node._M_next = &this->_M_impl._M_node;
455         this->_M_impl._M_node._M_prev = &this->_M_impl._M_node;
456         _M_set_size(0);
457       }
```
`_List_impl`定义一个结点，此结点为头结点，数据域存放链表结点个数（不包括头结点）
```
/// @file bits/stl_list.h
332       struct _List_impl
333       : public _Node_alloc_type
334       {
335 #if _GLIBCXX_USE_CXX11_ABI
336         _List_node<size_t> _M_node;
337 #else
338         __detail::_List_node_base _M_node;
339 #endif
```
结点类型为`_List_node<size_t>`，`_List_node`是一个模板类，继承于`_List_node_base`。模板参数决定结点数据域类型
```
/// @file bits/stl_list.h
105   template<typename _Tp>
106     struct _List_node : public __detail::_List_node_base
107     {
108       ///< User's data.
109       _Tp _M_data;
110 
111 #if __cplusplus >= 201103L
112       template<typename... _Args>
113         _List_node(_Args&&... __args)
114         : __detail::_List_node_base(), _M_data(std::forward<_Args>(__args)...) 
115         { }
116 #endif
117     };
```
`_List_node_base`是定义了两个指针`_M_next`和`_M_prev`，以及添加结点和删除结点的函数，分布为`_M_hook`和`_M_unhook`。
```
/// @file bits/stl_list.h
77     struct _List_node_base
78     {
79       _List_node_base* _M_next;
80       _List_node_base* _M_prev;

92       void
93       _M_hook(_List_node_base* const __position) _GLIBCXX_USE_NOEXCEPT;
94 
95       void
96       _M_unhook() _GLIBCXX_USE_NOEXCEPT;
```
`_M_hook`是将自己插入到`__position`指向的前面。`_M_unhook`删除自己。
```
/// @file src/c++98/list.cc
126     void
127     _List_node_base::
128     _M_hook(_List_node_base* const __position) _GLIBCXX_USE_NOEXCEPT
129     {
130       this->_M_next = __position;
131       this->_M_prev = __position->_M_prev;
132       __position->_M_prev->_M_next = this;
133       __position->_M_prev = this;
134     }
135 
136     void
137     _List_node_base::_M_unhook() _GLIBCXX_USE_NOEXCEPT
138     {
139       _List_node_base* const __next_node = this->_M_next;
140       _List_node_base* const __prev_node = this->_M_prev;
141       __prev_node->_M_next = __next_node;
142       __next_node->_M_prev = __prev_node;
143     }
```
总结：`_List_node`是一个模板类，类的参数决定数据域的类型。并且体够将自己添加到一个链表中或者将自己从一个链表中删除的操作。只有三个数据成员：两个指针域`_M_next`和`_M_prev`和一个数据域`_M_data`


### forward_list(C++11)
单向链表

### deque
双向开口的分段连续线性数据结构，可以在常数时间内在两端完成插入和删除操作。
```
828   template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
829     class deque : protected _Deque_base<_Tp, _Alloc>
830     {
841     public:
842       typedef _Tp                                        value_type;
843       typedef typename _Alloc_traits::pointer            pointer;
844       typedef typename _Alloc_traits::const_pointer      const_pointer;
845       typedef typename _Alloc_traits::reference          reference;
846       typedef typename _Alloc_traits::const_reference    const_reference;
847       typedef typename _Base::iterator                   iterator;
848       typedef typename _Base::const_iterator             const_iterator;
849       typedef std::reverse_iterator<const_iterator>      const_reverse_iterator;
850       typedef std::reverse_iterator<iterator>            reverse_iterator;
851       typedef size_t                             size_type;
852       typedef ptrdiff_t                          difference_type;
853       typedef _Alloc                             allocator_type;
```

#### 析构函数
``` 
@file bits/stl_deque.h
1037       ~deque()
1038       { _M_destroy_data(begin(), end(), _M_get_Tp_allocator()); }
```

## 数据结构


### `哈希表`
#### 相关概念
- 冲突：不同的关键字得到同一哈希地址的现象。
- 哈希表：根据设定的哈希函数H(key)和处理冲突的方法将一组关键字映像到一个有限的连续的地址集（区间）上，并以关键字在地址集中的“像”作为记录在表中的储存位置，这种表便是哈希表。
- 哈希函数构造方法
 - 直接定址法  -- 取关键字或关键字的某个现行函数值为哈希地址；
 - 数字分析法  -- 取关键字的若干数位组成哈希地址；
 - 平方取中法  -- 取关键字平方后的中间几位为哈希地址；
 - 折叠法  -- 将关键字分割成位数相同的几部分（最后一部分的位数可以不同），然后取这几部分的叠加和（舍去进位）作为哈希地址；
 - **除留余数法** -- 取关键字被某个不大于哈希表表长m的数p除后所得余数作为哈希地址；
 - 随机数法  -- 选择一个随机函数，取关键字的随机函数值为他的哈希地址；
- 处理冲突的方法
 - 开放定址法 $H_i = (H(key) + d_i) MOD m$
   - $d_i=1, 2, 3,..., m-1$，称为线性探测再散列；
   - $d_i=1^2, -1^2, 2^2, -2^2, ...$，称为二次探测再散列；
   - $d_i=$伪随机数序列，称为伪随机探测再散列；
 - 再哈希法 $H_i = RH_i(key)$，即在同义词产生地址冲突时计算另一个函数函数的地址，直到冲突不再发生
 - **链地址法** -- 将所有关键字为同义词的记录存储在同一线性链表中。
 - 建立公共溢出区

#### `_Hashtable`
`_Hashtable`有很多模板参数，其含义如下：
- `_Key`：结点的键值域类型
- `_Value`：结点的数据树类型
- `_Alloc`：空间配置器
- `_ExtractKey`：从结点中取出键值的方法（函数或者仿函数）
- `_Equal`：判断键值是否相同的方法
- `_H1`：哈希函数，调用形式`size_t(_Key)`
- `_H2`：范围哈希函数，调用形式`size_t(size_t, size_t)`，返回指定范围的哈希值
- `_Hash`：范围哈希函数，调用形式`size_t(size_t, size_t)`，返回指定范围的哈希值。默认为`_H2(_H1(k), N)`，如果指定了这个哈希函数，`_H1`和`_H2`将忽略
- `_RehashPolicy`：当冲突过多，需要更大的空间时间，需要此类中计算出新的bucket的大小。三个有用的成员函数：
  - `_M_next_bkt(n)`：返回一个不小于n的`bucket count`
  - `_M_bkt_for_elements(n)`：返回一个在n附近的`bucket count`
  - `_M_need_rehash(n_bkt, n_elt, n_ins)`：判断是否需要再哈希，是返回`make_pair(true, n)`，否返回`make_pair(false, <anything>)`
- `_Traits`：类型萃取机

```
/// @file bits/hashtable.h
166   template<typename _Key, typename _Value, typename _Alloc,
167            typename _ExtractKey, typename _Equal,
168            typename _H1, typename _H2, typename _Hash,
169            typename _RehashPolicy, typename _Traits>
170     class _Hashtable
171     : public __detail::_Hashtable_base<_Key, _Value, _ExtractKey, _Equal,
172                                        _H1, _H2, _Hash, _Traits>,
173       public __detail::_Map_base<_Key, _Value, _Alloc, _ExtractKey, _Equal,
174                                  _H1, _H2, _Hash, _RehashPolicy, _Traits>,
175       public __detail::_Insert<_Key, _Value, _Alloc, _ExtractKey, _Equal,
176                                _H1, _H2, _Hash, _RehashPolicy, _Traits>,
177       public __detail::_Rehash_base<_Key, _Value, _Alloc, _ExtractKey, _Equal,
178                                     _H1, _H2, _Hash, _RehashPolicy, _Traits>,
179       public __detail::_Equality<_Key, _Value, _Alloc, _ExtractKey, _Equal,
180                                  _H1, _H2, _Hash, _RehashPolicy, _Traits>,
181       private __detail::_Hashtable_alloc<
182         typename __alloctr_rebind<_Alloc,
183           __detail::_Hash_node<_Value,
184                                _Traits::__hash_cached::value> >::__type
```
注意的是`_Hashtable`没有选定默认空间配置器，使用的是`_Hashtable_alloc`分配器，该分配器是对传入的分配器的包装。其基类`__detail::_Hashtable_base`主要定义了迭代器。`_Hashtable`中类型相关的定义如下
```
/// @file bits/hashtable.h
201     public:
202       typedef _Key                                              key_type;
203       typedef _Value                                            value_type;
204       typedef _Alloc                                            allocator_type;
205       typedef _Equal                                            key_equal;
206 
207       // mapped_type, if present, comes from _Map_base.
208       // hasher, if present, comes from _Hash_code_base/_Hashtable_base.
209       typedef typename __value_alloc_traits::pointer            pointer;
210       typedef typename __value_alloc_traits::const_pointer      const_pointer;
211       typedef value_type&                                       reference;
212       typedef const value_type&                                 const_reference;

301     public:
302       using size_type = typename __hashtable_base::size_type;
303       using difference_type = typename __hashtable_base::difference_type;
304 
305       using iterator = typename __hashtable_base::iterator;
306       using const_iterator = typename __hashtable_base::const_iterator;
307 
308       using local_iterator = typename __hashtable_base::local_iterator;
309       using const_local_iterator = typename __hashtable_base::
310 				   const_local_iterator;
```
其中迭代器是`__detail::_Node_iterator<value_type, __constant_iterators::value, __hash_cached::value>`，是一个`forward_iterator_tag`迭代器。`_Hashtable`的数据结构相关的定义如下：
```
/// @file bits/hashtable.h
312     private:
313       __bucket_type*            _M_buckets              = &_M_single_bucket;
314       size_type                 _M_bucket_count         = 1;
315       __node_base               _M_before_begin;
316       size_type                 _M_element_count        = 0;
317       _RehashPolicy             _M_rehash_policy;
325       __bucket_type             _M_single_bucket        = nullptr;
```
其中，`__node_base`的定义如下：
```
/// @file bits/hashtable_policy.h
227   struct _Hash_node_base
228   {
229     _Hash_node_base* _M_nxt;
230 
231     _Hash_node_base() noexcept : _M_nxt() { }
232 
233     _Hash_node_base(_Hash_node_base* __next) noexcept : _M_nxt(__next) { }
234   };
```
`__bucket_type`是`_node_base*`的别名。每个结点的数类类型为`_Hash_node`，定义如下：
```
/// @file bits/hashtable_policy.h
276   template<typename _Value>
277     struct _Hash_node<_Value, true> : _Hash_node_value_base<_Value>
278     {
279       std::size_t  _M_hash_code;
280 
281       _Hash_node*
282       _M_next() const noexcept
283       { return static_cast<_Hash_node*>(this->_M_nxt); }
284     };

291   template<typename _Value>
292     struct _Hash_node<_Value, false> : _Hash_node_value_base<_Value>
293     {
294       _Hash_node*
295       _M_next() const noexcept
296       { return static_cast<_Hash_node*>(this->_M_nxt); }
297     };
```
如果指定缓冲哈希值，会有一个`_M_hash_code`。`_Hash_node`继承于`_Hash_node_value_base`，定义如下：
```
/// @file bits/hashtable_policy.h
241   template<typename _Value>
242     struct _Hash_node_value_base : _Hash_node_base
243     {
244       typedef _Value value_type;
245 
246       __gnu_cxx::__aligned_buffer<_Value> _M_storage;
247 
248       _Value*
249       _M_valptr() noexcept
250       { return _M_storage._M_ptr(); }
251 
252       const _Value*
253       _M_valptr() const noexcept
254       { return _M_storage._M_ptr(); }
255 
256       _Value&
257       _M_v() noexcept
258       { return *_M_valptr(); }
259 
260       const _Value&
261       _M_v() const noexcept
262       { return *_M_valptr(); }
263     };
```
因此每个`_Hash_node`结点中的数据成员：
```
_Hash_node_base* _M_nxt;
__gnu_cxx::__aligned_buffer<_Value> _M_storage;
std::size_t  _M_hash_code; //        _M_hash_code if cache_hash_code is true
```

#### `_M_insert_unique_node`实例
传入的参数包括bucket的序号`__bkt`，哈希值`__code`以及待插入结点`__node`。返回指向插入结点的迭代器。
```
/// @file bits/hashtable.h
1581   template<typename _Key, typename _Value,
1582            typename _Alloc, typename _ExtractKey, typename _Equal,
1583            typename _H1, typename _H2, typename _Hash, typename _RehashPolicy,
1584            typename _Traits>
1585     auto
1586     _Hashtable<_Key, _Value, _Alloc, _ExtractKey, _Equal,
1587                _H1, _H2, _Hash, _RehashPolicy, _Traits>::
1588     _M_insert_unique_node(size_type __bkt, __hash_code __code,
1589                           __node_type* __node)
1590     -> iterator
1591     {
```
首先是判断是否需要进行扩容，再哈希，调用`_Prime_rehash_policy::_M_need_rehash`进行判断是否需要再哈希，是返回`make_pair(true, n)`，否返回`make_pair(false, <anything>)`
```
/// @file bits/hashtable.h
1592       const __rehash_state& __saved_state = _M_rehash_policy._M_state();
1593       std::pair<bool, std::size_t> __do_rehash
1594         = _M_rehash_policy._M_need_rehash(_M_bucket_count, _M_element_count, 1);
```
`_M_need_rehash`参数含义：`__n_bkt`表示桶（bucekt）的总数量，`__n_elt`表示当前桶（将元素插入进去）的元素个数，`__n_ins`表示插入的元素的个数。`_Prime_rehash_policy::_M_next_resize`是一个阈值，表示单个桶能够储存元素的上界，由`_M_max_load_factor`控制。如果某个桶的元素个数达到这个阈值，就可能需要扩容。`_M_max_load_factor`表示一个负载系数上限，表示单个桶的元素个数与桶的个数的最大比例（默认是1）。因此，默认情况下，某个桶的元素个数大于等于桶的个数，需要扩容。

由于`_M_max_load_factor`是可以改变的（例如`std::unordered_set::max_load_factor(float ml)`就可以改变该值），所以即使超过`__M_next_resize`，也不一定扩容。只有`__min_bkts >= __n_bkt`才真正需要扩容，调用`_M_next_bkt`返回一个桶的数量。`_S_growth_factor`是一个常量，大小为2，所有扩容大致上是二倍增长。
```
/// src/c++11/hashtable-c++0x.cc
75   std::pair<bool, std::size_t>
76   _Prime_rehash_policy::
77   _M_need_rehash(std::size_t __n_bkt, std::size_t __n_elt,
78          std::size_t __n_ins) const
79   {
80     if (__n_elt + __n_ins >= _M_next_resize)
81       {
82     long double __min_bkts = (__n_elt + __n_ins)
83                    / (long double)_M_max_load_factor;
84     if (__min_bkts >= __n_bkt)
85       return std::make_pair(true,
86         _M_next_bkt(std::max<std::size_t>(__builtin_floor(__min_bkts) + 1,
87                           __n_bkt * _S_growth_factor)));
88     // _M_max_load_factor可能改变，所有每次需要更新_M_next_resize
89     _M_next_resize
90       = __builtin_floor(__n_bkt * (long double)_M_max_load_factor);
91     return std::make_pair(false, 0);
92       }
93     else
94       return std::make_pair(false, 0);
95   }
```

如果需要扩容，先扩容，（`_M_rehash`调用）`_M_rehash_aux`函数申请新空间并转移元素。
```
/// @file bits/hashtable.h
1596       __try
1597         {
1598           if (__do_rehash.first)
1599             {
1600               _M_rehash(__do_rehash.second, __saved_state);
1601               __bkt = _M_bucket_index(this->_M_extract()(__node->_M_v()), __code);
1602             }
```
`_M_rehash_aux`输入参数两个，`__n`表示大小，另一个表示键值是否唯一（分别对应两个函数`_M_rehash_aux(size_type __n, std::true_type)`和`_M_rehash_aux(size_type __n, std::false_type)`）
```
/// @file bits/hashtable.h
1965   template<typename _Key, typename _Value,
1966            typename _Alloc, typename _ExtractKey, typename _Equal,
1967            typename _H1, typename _H2, typename _Hash, typename _RehashPolicy,
1968            typename _Traits>
1969     void
1970     _Hashtable<_Key, _Value, _Alloc, _ExtractKey, _Equal,
1971                _H1, _H2, _Hash, _RehashPolicy, _Traits>::
1972     _M_rehash_aux(size_type __n, std::true_type)
1973     {
1974       __bucket_type* __new_buckets = _M_allocate_buckets(__n);
1975       __node_type* __p = _M_begin();
1976       _M_before_begin._M_nxt = nullptr;
1977       std::size_t __bbegin_bkt = 0;
1978       while (__p)
1979         {
1980           __node_type* __next = __p->_M_next();
1981           std::size_t __bkt = __hash_code_base::_M_bucket_index(__p, __n);
1982           if (!__new_buckets[__bkt])
1983             {
1984               __p->_M_nxt = _M_before_begin._M_nxt;
1985               _M_before_begin._M_nxt = __p;
1986               __new_buckets[__bkt] = &_M_before_begin;
1987               if (__p->_M_nxt)
1988                 __new_buckets[__bbegin_bkt] = __p;
1989               __bbegin_bkt = __bkt;
1990             }
1991           else
1992             {
1993               __p->_M_nxt = __new_buckets[__bkt]->_M_nxt;
1994               __new_buckets[__bkt]->_M_nxt = __p;
1995             }
1996           __p = __next;
1997         }
1998 
1999       _M_deallocate_buckets();
2000       _M_bucket_count = __n;
2001       _M_buckets = __new_buckets;
2002     }
```
然后进行插入`_M_insert_bucket_begin`，总是在链表头部插入。这个过程如果有异常产生，插入失败，`__node`会被释放
```
/// @file bits/hashtable.h
1604           this->_M_store_code(__node, __code);
1605 
1606           // Always insert at the beginning of the bucket.
1607           _M_insert_bucket_begin(__bkt, __node);
1608           ++_M_element_count;
1609           return iterator(__node);
1610         }
1611       __catch(...)
1612         {
1613           this->_M_deallocate_node(__node);
1614           __throw_exception_again;
1615         }
1616     }
```