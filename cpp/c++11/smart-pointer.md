# 智能指针
以引用计数为基础的智能指针，引用计数的工作方式如下
- 除了初始化对象外，每个构造函数（拷贝构造函数除外）还要创建一个引用计数，用来记录有多少个对象与正在创建的对象共享状态。当我们创建一个对象时，只有一个对象共享状态，因此将引用计数初始化为 1
- 拷贝构造函数不分配新的计数器，而是拷贝给定对象的数据成员，包括计数器。拷贝构造函数递增共享的计数器，指出给定对象的状态又被一个新用户所共享
- 析构函数递减计数器，指出共享状态的用户少了一个。如果计数器变为 0，则析构函数释放状态
- 拷贝赋值运算符递增右侧运算对象的计数器，递减左侧运算对象的计数器。如果左侧运算对象的计数器变为 0，意味着它的共享状态没有用户了，拷贝运算符就必须销毁状态
- 计数器一般保存在动态内存中。当创建一个对象时，也分配一个新的计数器。当拷贝或赋值对象时，拷贝指向计数器的指针。使用这种方法，副本和原对象都会指向相同的计数器

## `std::shared_ptr`
```
/// @file bits/shared_ptr.h
92   template<typename _Tp>
93     class shared_ptr : public __shared_ptr<_Tp>
94     {
95       template<typename _Ptr>
96         using _Convertible
97           = typename enable_if<is_convertible<_Ptr, _Tp*>::value>::type;
```
模板类 `shared_ptr` 继承于 `__shared_ptr`，具体的实现都在基类

### 构造函数
需要注意的是，`shared_ptr( Y* ptr )` 是 `explicit`。
```
/// TODO
```

### 析构函数
没有显式定义析构函数

### 接口
在基类中定义

### `__shared_ptr`
有两个数据域，一个存放对象指针，一个存放引用计数相关的结构。
```
/// @file bits/shared_ptr_base.h
 866   template<typename _Tp, _Lock_policy _Lp>
 867     class __shared_ptr
 868     {
 869       template<typename _Ptr>
 870         using _Convertible
 871           = typename enable_if<is_convertible<_Ptr, _Tp*>::value>::type;
 872 
 873     public:
 874       typedef _Tp   element_type;

1154     private:
1175       _Tp*                 _M_ptr;         // Contained pointer.
1176       __shared_count<_Lp>  _M_refcount;    // Reference counter.
```

#### 构造函数
只介绍形参是 `weak_ptr` 的构造函数，该函数用于实现 `weak_ptr::lock()`。当 `_M_refcount._M_get_use_count()` 返回 0（没有 `shared_ptr` 指针指向，被管理的对象已经释放）时，`_M_ptr` 被设置为 `nullptr`。
```
1144       // This constructor is used by __weak_ptr::lock() and
1145       // shared_ptr::shared_ptr(const weak_ptr&, std::nothrow_t).
1146       __shared_ptr(const __weak_ptr<_Tp, _Lp>& __r, std::nothrow_t)
1147       : _M_refcount(__r._M_refcount, std::nothrow)
1148       {
1149         _M_ptr = _M_refcount._M_get_use_count() ? __r._M_ptr : nullptr;
1150       }
```

#### 析构函数
使用默认析构函数，对象的管理在 `__shared_count<_Lp>::_M_refcount` 实现
```
/// @file bits/shared_ptr_base.h
925       ~__shared_ptr() = default;
```

#### 接口函数
```
/// @file bits/shared_ptr_base.h
1020       void
1021       reset() noexcept
1022       { __shared_ptr().swap(*this); }
1023 
1024       template<typename _Tp1>
1025         void
1026         reset(_Tp1* __p) // _Tp1 must be complete.
1027         {
1028           // Catch self-reset errors.
1029           _GLIBCXX_DEBUG_ASSERT(__p == 0 || __p != _M_ptr);
1030           __shared_ptr(__p).swap(*this);
1031         }
1032 
1033       template<typename _Tp1, typename _Deleter>
1034         void
1035         reset(_Tp1* __p, _Deleter __d)
1036         { __shared_ptr(__p, __d).swap(*this); }
1037 
1038       template<typename _Tp1, typename _Deleter, typename _Alloc>
1039         void
1040         reset(_Tp1* __p, _Deleter __d, _Alloc __a)
1041         { __shared_ptr(__p, __d, std::move(__a)).swap(*this); }
1042 
1043       // Allow class instantiation when _Tp is [cv-qual] void.
1044       typename std::add_lvalue_reference<_Tp>::type
1045       operator*() const noexcept
1046       {
1047         _GLIBCXX_DEBUG_ASSERT(_M_ptr != 0);
1048         return *_M_ptr;
1049       }
1050 
1051       _Tp*
1052       operator->() const noexcept
1053       {
1054         _GLIBCXX_DEBUG_ASSERT(_M_ptr != 0);
1055         return _M_ptr;
1056       }
1057 
1058       _Tp*
1059       get() const noexcept
1060       { return _M_ptr; }
1061 
1062       explicit operator bool() const // 操作符号重载，可以用于条件判断
1063       { return _M_ptr == 0 ? false : true; }
1064 
1065       bool
1066       unique() const noexcept
1067       { return _M_refcount._M_unique(); }
1068 
1069       long
1070       use_count() const noexcept
1071       { return _M_refcount._M_get_use_count(); }
1072 
1073       void
1074       swap(__shared_ptr<_Tp, _Lp>& __other) noexcept
1075       {
1076         std::swap(_M_ptr, __other._M_ptr);
1077         _M_refcount._M_swap(__other._M_refcount);
1078       }
1079 
1080       template<typename _Tp1>
1081         bool
1082         owner_before(__shared_ptr<_Tp1, _Lp> const& __rhs) const
1083         { return _M_refcount._M_less(__rhs._M_refcount); }
1084 
1085       template<typename _Tp1>
1086         bool
1087         owner_before(__weak_ptr<_Tp1, _Lp> const& __rhs) const
1088         { return _M_refcount._M_less(__rhs._M_refcount); }
```

### `__shared_count`
`__shared_count` 不仅仅存放引用计数器，还**负责管理对象的释放**。只有一个数据域 `_M_pi`，是模板类 `_Sp_counted_base` 类型的指针。`_Sp_counted_base` 是一个基类，其派生类有 `_Sp_counted_ptr`（只有管理对象的指针，默认用 `delete` 释放管理的函数）、`_Sp_counted_deleter`（除了管理对象的指针，还有善后函数 deleter，在析构时调用 `deleter()`，不再调用 `delete`。相当于用用户指定非方式释放对象）或者 `_Sp_counted_ptr_inplace`（`std::make_shared` 申请的对象）。所以指针 `_M_pi` 可能指向不同的派生类。
```
/// @file bits/shared_ptr_base.h
561   template<_Lock_policy _Lp>
562     class __shared_count
563     {

717     private:
720       _Sp_counted_base<_Lp>*  _M_pi;
721     };
```

#### 构造函数
可以传入不同的参数，对象指针，Deleter 或者分配器，此时会分配引用计数的存储空间。无参构造函数不分配引用计数空间
```
/// @file bits/shared_ptr_base.h
565       constexpr __shared_count() noexcept : _M_pi(0)
566       { }
```
当用如下的方式（只传入指针）创建一个 std::shared 对象
```
class Foo {
 public:
  Foo(int val): val(val), next(nullptr) {}
  int val;
  Foo* next;
};

std::shared_ptr<Foo> ptr(new Foo());
```
`__shared_count` 会执行如下构造函数，构造函数完成后 `_M_pi` 指向的是 `_Sp_counted_ptr` 类型的对象。
```
/// @file bits/shared_ptr_base.h
568       template<typename _Ptr>
569         explicit
570         __shared_count(_Ptr __p) : _M_pi(0)
571         {
572           __try
573             {
574               _M_pi = new _Sp_counted_ptr<_Ptr, _Lp>(__p);
575             }
576           __catch(...)
577             {
578               delete __p;
579               __throw_exception_again;
580             }
581         }
```
当用如下方式（传入指针和 Deleter ）创建一个 std::shared 对象
```
struct Deleter {
  void operator()(Foo* p) const {
	std::cout << "Call delete from function object...\n";
	delete p;
  }
};

std::shared_ptr<Foo> ptr(new Foo(-1), Deleter());
```
`__shared_count` 最终会执行如下构造函数，构造函数完成后 `_M_pi` 指向的是 `_Sp_counted_deleter` 类型的对象。
```
/// @file bits/shared_ptr_base.h
588       template<typename _Ptr, typename _Deleter, typename _Alloc>
589         __shared_count(_Ptr __p, _Deleter __d, _Alloc __a) : _M_pi(0)
590         {
591           typedef _Sp_counted_deleter<_Ptr, _Deleter, _Alloc, _Lp> _Sp_cd_type;
592           __try
593             {
594               typename _Sp_cd_type::__allocator_type __a2(__a);
595               auto __guard = std::__allocate_guarded(__a2);
596               _Sp_cd_type* __mem = __guard.get();
597               ::new (__mem) _Sp_cd_type(__p, std::move(__d), std::move(__a)); // placement new
598               _M_pi = __mem;
599               __guard = nullptr;
600             }
601           __catch(...)
602             {
603               __d(__p); // Call _Deleter on __p.
604               __throw_exception_again;
605             }
606         }
```
当用 std::make_shared() 函数创建一个 std::shared_ptr 对象的时候，会执行如下构造函数，构造函数完成后 `_M_pi` 指向的是 `_Sp_counted_ptr_inplace` 类型的对象。
```
/// @file bits/shared_ptr_base.h
608       template<typename _Tp, typename _Alloc, typename... _Args>
609         __shared_count(_Sp_make_shared_tag, _Tp*, const _Alloc& __a,
610                        _Args&&... __args)
611         : _M_pi(0)
612         {
613           typedef _Sp_counted_ptr_inplace<_Tp, _Alloc, _Lp> _Sp_cp_type;
614           typename _Sp_cp_type::__allocator_type __a2(__a);
615           auto __guard = std::__allocate_guarded(__a2);
616           _Sp_cp_type* __mem = __guard.get();
617           ::new (__mem) _Sp_cp_type(std::move(__a),
618                                     std::forward<_Args>(__args)...); 
619           _M_pi = __mem;
620           __guard = nullptr;
621         }
```
拷贝构造函数不分配引用计数空间，而是拷贝传入对象的 `_M_pi`，并且将计数加 1
```
/// @file bits/shared_ptr_base.h
662       __shared_count(const __shared_count& __r) noexcept
663       : _M_pi(__r._M_pi)
664       {
665         if (_M_pi != 0)
666           _M_pi->_M_add_ref_copy();
667       }
```
拷贝赋值运算符增加右侧对象的引用计数，减少左侧的引用计数，如果左侧引用计数变为 0，释放管理的对象（`_M_pi->_M_release()`）
```
/// @file bits/shared_ptr_base.h
669       __shared_count&
670       operator=(const __shared_count& __r) noexcept
671       {
672         _Sp_counted_base<_Lp>* __tmp = __r._M_pi;
673         if (__tmp != _M_pi)
674           {
675             if (__tmp != 0)
676               __tmp->_M_add_ref_copy();
677             if (_M_pi != 0)
678               _M_pi->_M_release();
679             _M_pi = __tmp;
680           }
681         return *this;
682       }
```

#### 析构函数
掉用虚函数 `_M_release()`，不同派生类有其自己的实现
```
/// @file bits/shared_ptr_base.h
656       ~__shared_count() noexcept
657       {
658         if (_M_pi != nullptr)
659           _M_pi->_M_release();
660       }
```

### `_Sp_counted_base`
是一个基类，有两个数据成员 `_M_use_count` 和 `_M_weak_count`，分别表示有多少个 `shared_ptr` 和 `weak_ptr` 指向管理的对象（如果 `_M_use_count` 不为 0，`_M_weak_count` 额外需要加 1）。此外引用计数的相关操作是原子操作。
```
/// @file bits/shared_ptr_base.h
107   template<_Lock_policy _Lp = __default_lock_policy>
108     class _Sp_counted_base
109     : public _Mutex_base<_Lp>
110     {
203     private:  

207       _Atomic_word  _M_use_count;     // #shared
208       _Atomic_word  _M_weak_count;    // #weak + (#shared != 0)
209     };
```
主要定义定义引用计数的递增函数和递减函数、释放资源的函数 `_M_release()` 以及三个虚函数 `_M_dispose()` 、`_M_destroy()` 和 `_M_get_deleter()`。

#### 构造函数
只有无参构造函数，无法指定引用计数。引用计数都必须通过定义的函数接口改变，另外，它的拷贝构造函数和赋值操作是删除的。
```
/// @file bits/shared_ptr_base.h
111     public:  
112       _Sp_counted_base() noexcept
113       : _M_use_count(1), _M_weak_count(1) { }
  
203     private:
204       _Sp_counted_base(_Sp_counted_base const&) = delete;
205       _Sp_counted_base& operator=(_Sp_counted_base const&) = delete;
209     };
```

#### 析构函数
```
/// @file bits/shared_ptr_base.h
115       virtual
116       ~_Sp_counted_base() noexcept
117       { }
```

#### 资源释放`_M_release()` 和 `_M_weak_release()`
`_M_release()` 首先引用计数 `_M_use_count`减 1（`__exchange_and_add_dispatch()`，原子操作，将第二个参数加到第一个参数，返回返回第一个参数的旧值），如果引用计数变为 0，执行虚函数 `_M_dispose()` 析构对象，释放内存。另外将 `_M_weak_count` 减 1，如果变为 0，执行虚函数 `_M_destroy()` 释放引用计数本身对象。
```
/// @file bits/shared_ptr_base.h
142       void
143       _M_release() noexcept
144       {
145         // Be race-detector-friendly.  For more info see bits/c++config.
146         _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(&_M_use_count);
147         if (__gnu_cxx::__exchange_and_add_dispatch(&_M_use_count, -1) == 1)
148           {
149             _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(&_M_use_count);
150             _M_dispose(); // 虚函数，执行派生类实现

155             if (_Mutex_base<_Lp>::_S_need_barriers)
156               {
157                 _GLIBCXX_READ_MEM_BARRIER;
158                 _GLIBCXX_WRITE_MEM_BARRIER;
159               }
160 
161             // Be race-detector-friendly.  For more info see bits/c++config.
162             _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(&_M_weak_count);
163             if (__gnu_cxx::__exchange_and_add_dispatch(&_M_weak_count,
164                                                        -1) == 1)
165               {
166                 _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(&_M_weak_count);
167                 _M_destroy(); // 虚函数，执行派生类实现
168               }
169           }
170       }
```
`_M_weak_release()` 是跟 `weak_ptr` 相关的操作。`_M_weak_count` 减 1，如果变为 0，调用 `_M_destroy()` 释放引用计数本身对象。
```
/// @file bits/shared_ptr_base.h
176       void
177       _M_weak_release() noexcept
178       {
179         // Be race-detector-friendly. For more info see bits/c++config.
180         _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(&_M_weak_count);
181         if (__gnu_cxx::__exchange_and_add_dispatch(&_M_weak_count, -1) == 1)
182           {
183             _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(&_M_weak_count);
184             if (_Mutex_base<_Lp>::_S_need_barriers)
185               {
186                 // See _M_release(),
187                 // destroy() must observe results of dispose()
188                 _GLIBCXX_READ_MEM_BARRIER;
189                 _GLIBCXX_WRITE_MEM_BARRIER;
190               }
191             _M_destroy();
192           }
193       }
```
`_M_weak_count = #weak + (#shared != 0)` 为了确保 `shared_ptr` 管理的对象析构了，`weak_ptr` 仍然可以使用（引用计数对象没有被析构）。例如当出现`_M_use_count = 1, _M_weak_count = 1` 的时候执行 `_M_release()`，管理的对象会释放，引用计数对象也会被释放，仍然存在的一个 `weak_ptr` 将不能使用。

#### `_M_dispose()` 和 `_M_destroy()`
`_M_dispose` 用于当 `_M_use_count` 减为 0 的时候，释放 `this` 管理的对象。`_M_destroy` 用于当 `_M_weak_count` 减为0的时候，释放 `this` 对象。
```
/// @file bits/shared_ptr_base.h
119       // Called when _M_use_count drops to zero, to release the resources
120       // managed by *this.
121       virtual void
122       _M_dispose() noexcept = 0; // 派生类实现
123       
124       // Called when _M_weak_count drops to zero.
125       virtual void
126       _M_destroy() noexcept
127       { delete this; }
```

#### `_M_add_ref_copy()` 和 `_M_weak_add_ref()`
`_M_add_ref_copy()` 用于将 `_M_use_count` 加 1，`_M_weak_add_ref()` 用于将`_M_weak_count` 加 1
```
/// @file bits/shared_ptr_base.h
132       void
133       _M_add_ref_copy()
134       { __gnu_cxx::__atomic_add_dispatch(&_M_use_count, 1); }

172       void
173       _M_weak_add_ref() noexcept
174       { __gnu_cxx::__atomic_add_dispatch(&_M_weak_count, 1); }
```

#### `_M_get_use_count()` 和 `_M_get_deleter()`
```
/// @file bits/shared_ptr_base.h
129       virtual void*
130       _M_get_deleter(const std::type_info&) noexcept = 0; // 派生类实现

195       long
196       _M_get_use_count() const noexcept
197       {
198         // No memory barrier is used here so there is no synchronization
199         // with other threads.
200         return __atomic_load_n(&_M_use_count, __ATOMIC_RELAXED);
201       }
```

#### `_M_add_ref_lock()` 和 `_M_add_ref_lock_nothrow()`
这两个函数用于 `weak_ptr::lock()` 操作。下面仅仅是一部分，还有其他偏特化版本。当`_M_use_count` 为 0，是提升失败
```
/// @file bits/shared_ptr_base.h
221   template<>
222     inline void
223     _Sp_counted_base<_S_mutex>::
224     _M_add_ref_lock()
225     {
226       __gnu_cxx::__scoped_lock sentry(*this);
227       if (__gnu_cxx::__exchange_and_add_dispatch(&_M_use_count, 1) == 0)
228         {
229           _M_use_count = 0;
230           __throw_bad_weak_ptr();
231         }
232     }

264   template<>
265     inline bool
266     _Sp_counted_base<_S_mutex>::
267     _M_add_ref_lock_nothrow()
268     {
269       __gnu_cxx::__scoped_lock sentry(*this);
270       if (__gnu_cxx::__exchange_and_add_dispatch(&_M_use_count, 1) == 0)
271         {
272           _M_use_count = 0;
273           return false;
274         }
275       return true;
276     }
```

#### `_M_dispose()` 、`_M_destroy()` 和 `_M_get_deleter()` 在派生类中的实现
在 `_Sp_counted_ptr` 中的实现
```
/// @file bits/shared_ptr.h
364   template<typename _Ptr, _Lock_policy _Lp>
365     class _Sp_counted_ptr final : public _Sp_counted_base<_Lp>
366     {
367     public:
/// ...
372       virtual void
373       _M_dispose() noexcept
374       { delete _M_ptr; } // 调用 delete
375 
376       virtual void
377       _M_destroy() noexcept
378       { delete this; }
379 
380       virtual void*
381       _M_get_deleter(const std::type_info&) noexcept
382       { return nullptr; } // 没有
```
在 `_Sp_counted_deleter` 中的实现
```
/// @file bits/shared_ptr.h
432   template<typename _Ptr, typename _Deleter, typename _Alloc, _Lock_policy _Lp>
433     class _Sp_counted_deleter final : public _Sp_counted_base<_Lp>
434     {
451     public:
464       virtual void
465       _M_dispose() noexcept
466       { _M_impl._M_del()(_M_impl._M_ptr); } // 调用指定的可调用对象
467 
468       virtual void
469       _M_destroy() noexcept
470       {
471         __allocator_type __a(_M_impl._M_alloc());
472         __allocated_ptr<__allocator_type> __guard_ptr{ __a, this };
473         this->~_Sp_counted_deleter(); // 调用析构函数（空），基类delete
474       }
475 
476       virtual void*
477       _M_get_deleter(const std::type_info& __ti) noexcept
478       {
479 #if __cpp_rtti
480         // _GLIBCXX_RESOLVE_LIB_DEFECTS
481         // 2400. shared_ptr's get_deleter() should use addressof()
482         return __ti == typeid(_Deleter)
483           ? std::__addressof(_M_impl._M_del())
484           : nullptr;
485 #else
486         return nullptr;
487 #endif
```
在 `_Sp_counted_ptr_inplace` 中的实现
```
/// @file bits/shared_ptr.h
498   template<typename _Tp, typename _Alloc, _Lock_policy _Lp>
499     class _Sp_counted_ptr_inplace final : public _Sp_counted_base<_Lp>
500     {
/// ...
512 
513     public:
514       using __allocator_type = __alloc_rebind<_Alloc, _Sp_counted_ptr_inplace>;
/// ...
525 
526       ~_Sp_counted_ptr_inplace() noexcept { }
527 
528       virtual void
529       _M_dispose() noexcept
530       { // 调用分配器的 destroy()
531         allocator_traits<_Alloc>::destroy(_M_impl._M_alloc(), _M_ptr());
532       }
533 
534       // Override because the allocator needs to know the dynamic type
535       virtual void
536       _M_destroy() noexcept
537       {
538         __allocator_type __a(_M_impl._M_alloc());
539         __allocated_ptr<__allocator_type> __guard_ptr{ __a, this };
540         this->~_Sp_counted_ptr_inplace(); // 析构函数为空，基类 delete
541       }
542 
543       // Sneaky trick so __shared_ptr can get the managed pointer
544       virtual void*
545       _M_get_deleter(const std::type_info& __ti) noexcept
546       {
547 #if __cpp_rtti
548         if (__ti == typeid(_Sp_make_shared_tag))
549           return const_cast<typename remove_cv<_Tp>::type*>(_M_ptr());
550 #endif
551         return nullptr; // 没有
552       }
```

## `std::weak_ptr`
不控制所指向对象生命周期的智能指针，它指向一个 `shared_ptr` 管理的对象
```
/// @file bits/shared_ptr.h
469   template<typename _Tp>
470     class weak_ptr : public __weak_ptr<_Tp>
471     {
472       template<typename _Ptr>
473         using _Convertible
474           = typename enable_if<is_convertible<_Ptr, _Tp*>::value>::type;
```
具体实现在基类 `__weak_ptr<_Tp>` 中

### 构造函数
/// TODO

### 析构函数
没有显式定义的析构函数

### `lock`
```
/// @file bits/shared_ptr.h
525       shared_ptr<_Tp>
526       lock() const noexcept
527       { return shared_ptr<_Tp>(*this, std::nothrow); }
```

### `__weak_ptr`
有两个数据域，一个存放对象指针，一个存放引用计数相关的结构。
```
/// @file bits/shared_ptr_base.h
1339   template<typename _Tp, _Lock_policy _Lp>
1340     class __weak_ptr
1341     {
1342       template<typename _Ptr>
1343         using _Convertible
1344           = typename enable_if<is_convertible<_Ptr, _Tp*>::value>::type;

1463     private:
1477       _Tp*               _M_ptr;         // Contained pointer.
1478       __weak_count<_Lp>  _M_refcount;    // Reference counter.
```

#### 析构函数
默认析构函数
```
/// @file bits/shared_ptr_base.h
1346     public:
1355       ~__weak_ptr() = default;
```

#### 接口
```
/// @file bits/shared_ptr_base.h
1430       __shared_ptr<_Tp, _Lp>
1431       lock() const noexcept
1432       { return __shared_ptr<element_type, _Lp>(*this, std::nothrow); }
1433 
1434       long
1435       use_count() const noexcept
1436       { return _M_refcount._M_get_use_count(); }
1437 
1438       bool
1439       expired() const noexcept
1440       { return _M_refcount._M_get_use_count() == 0; }
1441 
1442       template<typename _Tp1>
1443         bool
1444         owner_before(const __shared_ptr<_Tp1, _Lp>& __rhs) const
1445         { return _M_refcount._M_less(__rhs._M_refcount); }
1446 
1447       template<typename _Tp1>
1448         bool
1449         owner_before(const __weak_ptr<_Tp1, _Lp>& __rhs) const
1450         { return _M_refcount._M_less(__rhs._M_refcount); }
1451 
1452       void
1453       reset() noexcept
1454       { __weak_ptr().swap(*this); }
```

### `__weak_count`
只有一个数据成员 `_M_pi`，是 `_Sp_counted_base` 类型，和 `__shared_count` 一样
```
/// @file bits/shared_ptr_base.h
724   template<_Lock_policy _Lp>
725     class __weak_count

814     private:
817       _Sp_counted_base<_Lp>*  _M_pi;
```

#### 构造函数
需要判断 `_M_pi` 的有效性（是否为空指针）
```
/// @file bits/shared_ptr_base.h
727     public:
728       constexpr __weak_count() noexcept : _M_pi(nullptr)
729       { }
730 
731       __weak_count(const __shared_count<_Lp>& __r) noexcept
732       : _M_pi(__r._M_pi)
733       {
734         if (_M_pi != nullptr)
735           _M_pi->_M_weak_add_ref();
736       }
737 
738       __weak_count(const __weak_count& __r) noexcept
739       : _M_pi(__r._M_pi)
740       {
741         if (_M_pi != nullptr)
742           _M_pi->_M_weak_add_ref();
743       }
744 
745       __weak_count(__weak_count&& __r) noexcept
746       : _M_pi(__r._M_pi)
747       { __r._M_pi = nullptr; }
```

#### 析构函数
```
/// @file bits/shared_ptr_base.h
749       ~__weak_count() noexcept
750       {
751         if (_M_pi != nullptr)
752           _M_pi->_M_weak_release();
753       }
```

#### `_M_get_use_count()`
```
/// @file bits/shared_ptr_base.h
797       long
798       _M_get_use_count() const noexcept
799       { return _M_pi != nullptr ? _M_pi->_M_get_use_count() : 0; }
```

## `std::enable_shared_from_this`
拥有一个 `weak_ptr`
```
/// @file bits/shared_ptr.h
556   template<typename _Tp>
557     class enable_shared_from_this
558     {
559     protected:
560       constexpr enable_shared_from_this() noexcept { }
561 
562       enable_shared_from_this(const enable_shared_from_this&) noexcept { }
563 
564       enable_shared_from_this&
565       operator=(const enable_shared_from_this&) noexcept
566       { return *this; }
567 
568       ~enable_shared_from_this() { }
578 
579     private:
580       template<typename _Tp1>
581         void
582         _M_weak_assign(_Tp1* __p, const __shared_count<>& __n) const noexcept
583         { _M_weak_this._M_assign(__p, __n); }
584 
585       template<typename _Tp1>
586         friend void
587         __enable_shared_from_this_helper(const __shared_count<>& __pn,
588                                          const enable_shared_from_this* __pe,
589                                          const _Tp1* __px) noexcept
590         {
591           if (__pe != 0)
592             __pe->_M_weak_assign(const_cast<_Tp1*>(__px), __pn);
593         }
594 
595       mutable weak_ptr<_Tp>  _M_weak_this;
596     };
```

### `shared_from_this()`
```
/// @file bits/shared_ptr.h
570     public:
571       shared_ptr<_Tp>
572       shared_from_this()
573       { return shared_ptr<_Tp>(this->_M_weak_this); }
574 
575       shared_ptr<const _Tp>
576       shared_from_this() const
577       { return shared_ptr<const _Tp>(this->_M_weak_this); }
```

## 管理单个对象的 `std::uniqe_ptr`
拷贝赋值构造函数和赋值运算符是删除的
```
@file bits/unique_ptr.h
128   template <typename _Tp, typename _Dp = default_delete<_Tp> >
129     class unique_ptr
130     {
146       typedef std::tuple<typename _Pointer::type, _Dp>  __tuple_type;
147       __tuple_type                                      _M_t;
149     public:
148 
150       typedef typename _Pointer::type   pointer;
151       typedef _Tp                       element_type;
152       typedef _Dp                       deleter_type;

355       // Disable copy from lvalue.
356       unique_ptr(const unique_ptr&) = delete;
357       unique_ptr& operator=(const unique_ptr&) = delete;
```

### 析构函数
```
@file bits/unique_ptr.h
232       ~unique_ptr() noexcept
233       {
234         auto& __ptr = std::get<0>(_M_t);
235         if (__ptr != nullptr)
236           get_deleter()(__ptr);
237         __ptr = pointer();
238       }
```

### 接口
```
@file bits/unique_ptr.h
287       typename add_lvalue_reference<element_type>::type
288       operator*() const
289       {
290         _GLIBCXX_DEBUG_ASSERT(get() != pointer());
291         return *get();
292       }
295       pointer
296       operator->() const noexcept
297       {
298         _GLIBCXX_DEBUG_ASSERT(get() != pointer());
299         return get();
300       }
303       pointer
304       get() const noexcept
305       { return std::get<0>(_M_t); }
308       deleter_type&
309       get_deleter() noexcept
310       { return std::get<1>(_M_t); }
313       const deleter_type&
314       get_deleter() const noexcept
315       { return std::get<1>(_M_t); }
318       explicit operator bool() const noexcept
319       { return get() == pointer() ? false : true; }
324       pointer
325       release() noexcept
326       {
327         pointer __p = get();
328         std::get<0>(_M_t) = pointer();
329         return __p;
330       }
338       void
339       reset(pointer __p = pointer()) noexcept
340       {
341         using std::swap;
342         swap(std::get<0>(_M_t), __p);
343         if (__p != pointer())
344           get_deleter()(__p);
345       }
348       void
349       swap(unique_ptr& __u) noexcept
350       {
351         using std::swap;
352         swap(_M_t, __u._M_t);
353       }
```

## 管理数组的`std::uniqe_ptr`
增加 `operator[]` 方法

## 其他函数
### `std::make_shared()`
调用 `allocate_shared` 构造一个 `shared_ptr`
```
/// @file bits/shared_ptr.h
609   template<typename _Tp, typename _Alloc, typename... _Args>
610     inline shared_ptr<_Tp>
611     allocate_shared(const _Alloc& __a, _Args&&... __args)
612     {
613       return shared_ptr<_Tp>(_Sp_make_shared_tag(), __a,
614                              std::forward<_Args>(__args)...);
615     }

624   template<typename _Tp, typename... _Args>
625     inline shared_ptr<_Tp>
626     make_shared(_Args&&... __args)
627     {
628       typedef typename std::remove_const<_Tp>::type _Tp_nc;
629       return std::allocate_shared<_Tp>(std::allocator<_Tp_nc>(),
630                                        std::forward<_Args>(__args)...);
631     }
```

### `hash`
根据 `get()` 返回的指针进行 hash
```
/// @file bits/shared_ptr.h
634   template<typename _Tp>
635     struct hash<shared_ptr<_Tp>>
636     : public __hash_base<size_t, shared_ptr<_Tp>>
637     {
638       size_t
639       operator()(const shared_ptr<_Tp>& __s) const noexcept
640       { return std::hash<_Tp*>()(__s.get()); }
641     };
```

### `shared_ptr` 指针转换
创建新的 `std::shared_ptr` 的实例，将管理对象的类型从 `_Tp1` 转换成 `_Tp`。底层仍然共享管理的对象
```
/// @file bits/shared_ptr.h
444   template<typename _Tp, typename _Tp1>
445     inline shared_ptr<_Tp>
446     static_pointer_cast(const shared_ptr<_Tp1>& __r) noexcept
447     { return shared_ptr<_Tp>(__r, static_cast<_Tp*>(__r.get())); }
448 
449   template<typename _Tp, typename _Tp1>
450     inline shared_ptr<_Tp>
451     const_pointer_cast(const shared_ptr<_Tp1>& __r) noexcept
452     { return shared_ptr<_Tp>(__r, const_cast<_Tp*>(__r.get())); }
453 
454   template<typename _Tp, typename _Tp1>
455     inline shared_ptr<_Tp>
456     dynamic_pointer_cast(const shared_ptr<_Tp1>& __r) noexcept
457     {
458       if (_Tp* __p = dynamic_cast<_Tp*>(__r.get()))
459         return shared_ptr<_Tp>(__r, __p);
460       return shared_ptr<_Tp>();
461     }
```

## `std::shared_ptr<void>` 工作方式
下面的方式是可以正常工作的
```
class Foo {
 public:
  Foo(int val): val(val), next(nullptr) {
    std::cout << "Foo\n";
  }
  ~Foo() {
    std::cout << "~Foo\n";
  }
  int val;
  Foo* next;
};

struct Deleter {
  void operator()(Foo* p) const {
	std::cout << "Call delete from function object...\n";
	delete p;
  }
};

int main() {
  shared_ptr<void> ptr;
  ptr.reset(new Foo(-1));
  return 0;
}
```
即使定义的时候，`std::shared_ptr` 的类模板类型是 `void` 类型，我们在 reset() 函数中传入一个 Foo 类型的指针，`std::shared_ptr` 也可以自动地析构 Foo 的对象。如果是 `std::shared_ptr<int>` 没有这种用法。
```
shared_ptr<int> ptr;
ptr.reset(new Foo(-1)); // cannot convert ‘Foo*’ to ‘int*’ in initialization
```
根据分析的继承关系，`shared_ptr` 继承于 `__shared_ptr`，回头看一下 `__shared_ptr` 的实现
```
/// @file bits/shared_ptr_base.h
 866   template<typename _Tp, _Lock_policy _Lp>
 867     class __shared_ptr
 868     {
 869       template<typename _Ptr>
 870         using _Convertible
 871           = typename enable_if<is_convertible<_Ptr, _Tp*>::value>::type;
 872 
 873     public:
 874       typedef _Tp   element_type;

1154     private:
1175       _Tp*                 _M_ptr;         // Contained pointer.
1176       __shared_count<_Lp>  _M_refcount;    // Reference counter.
```
可以看到，`__shared_ptr::_M_ptr` 跟模板参数类型相关，而 `__shared_ptr::_M_refcount` 跟模板参数是无关的。所以当模板参数是 `void` 的时候，`void` 指针可以指向任何对象，而其他指针则不行。根据前面的分析 `_M_refcount` 是负责释放管理的对象的，那即使定义为 `std::shared_ptr<void>`，也可以释放对象，它是如何做到的？在回头看一下 `__shared_count` 的定义
```
/// @file bits/shared_ptr_base.h
561   template<_Lock_policy _Lp>
562     class __shared_count
563     {

717     private:
720       _Sp_counted_base<_Lp>*  _M_pi;
721     };
```
`_Sp_counted_base` 是一个基类，只有两个表示引用计数的成员
```
/// @file bits/shared_ptr_base.h
107   template<_Lock_policy _Lp = __default_lock_policy>
108     class _Sp_counted_base
109     : public _Mutex_base<_Lp>
110     {
203     private:  

207       _Atomic_word  _M_use_count;     // #shared
208       _Atomic_word  _M_weak_count;    // #weak + (#shared != 0)
209     };
```
如前面所说，`_Sp_counted_base` 的 `_M_release()` 会调用派生类的 `_M_dispose()` 进行对象的释放。并且 `__shared_count` 会根据不同的传入参数，创建不同的 `_Sp_counted_base` 对象。接下来分析三个派生类的构造函数。

首先是 `_Sp_counted_ptr`
```
/// @file bits/shared_ptr.h
364   template<typename _Ptr, _Lock_policy _Lp>
365     class _Sp_counted_ptr final : public _Sp_counted_base<_Lp>
366     {
367     public:
368       explicit
369       _Sp_counted_ptr(_Ptr __p) noexcept
370       : _M_ptr(__p) { }
386 
387     private:
388       _Ptr             _M_ptr;
389     };
```
然后是 `_Sp_counted_deleter`
```
/// @file bits/shared_ptr.h
432   template<typename _Ptr, typename _Deleter, typename _Alloc, _Lock_policy _Lp>
433     class _Sp_counted_deleter final : public _Sp_counted_base<_Lp>
434     {
435       class _Impl : _Sp_ebo_helper<0, _Deleter>, _Sp_ebo_helper<1, _Alloc>
436       {
437         typedef _Sp_ebo_helper<0, _Deleter>     _Del_base;
438         typedef _Sp_ebo_helper<1, _Alloc>       _Alloc_base;
439 
440       public:
441         _Impl(_Ptr __p, _Deleter __d, const _Alloc& __a) noexcept
442         : _M_ptr(__p), _Del_base(__d), _Alloc_base(__a)
443         { }
444 
445         _Deleter& _M_del() noexcept { return _Del_base::_S_get(*this); }
446         _Alloc& _M_alloc() noexcept { return _Alloc_base::_S_get(*this); }
447 
448         _Ptr _M_ptr;
449       };
450 
451     public:
455       _Sp_counted_deleter(_Ptr __p, _Deleter __d) noexcept
456       : _M_impl(__p, __d, _Alloc()) { }
457 
458       // __d(__p) must not throw.
459       _Sp_counted_deleter(_Ptr __p, _Deleter __d, const _Alloc& __a) noexcept
460       : _M_impl(__p, __d, __a) { }
489 
490     private:
491       _Impl _M_impl;
492     };
```
最后是 `_Sp_counted_ptr_inplace`
```
/// @file bits/shared_ptr.h
498   template<typename _Tp, typename _Alloc, _Lock_policy _Lp>
499     class _Sp_counted_ptr_inplace final : public _Sp_counted_base<_Lp>
500     {
501       class _Impl : _Sp_ebo_helper<0, _Alloc>
502       {
503         typedef _Sp_ebo_helper<0, _Alloc>       _A_base;
504 
505       public:
506         explicit _Impl(_Alloc __a) noexcept : _A_base(__a) { }
507 
508         _Alloc& _M_alloc() noexcept { return _A_base::_S_get(*this); }
509 
510         __gnu_cxx::__aligned_buffer<_Tp> _M_storage;
511       };
512 
513     public:
514       using __allocator_type = __alloc_rebind<_Alloc, _Sp_counted_ptr_inplace>;
515
516       template<typename... _Args>
517         _Sp_counted_ptr_inplace(_Alloc __a, _Args&&... __args)
518         : _M_impl(__a)
519         {
520           // _GLIBCXX_RESOLVE_LIB_DEFECTS
521           // 2070.  allocate_shared should use allocator_traits<A>::construct
522           allocator_traits<_Alloc>::construct(__a, _M_ptr(),
523               std::forward<_Args>(__args)...); // might throw
524         }
```
可以知道，三个派生类都是模板类，模板参数 `_Ptr` 就是实际管理的对象的类型指针。所以即使在定义 `std::shared_ptr` 指定类模板参数为 `void`。可以看到 reset() 函数也是模板函数
```
1024       template<typename _Tp1>
1025         void
1026         reset(_Tp1* __p) // _Tp1 must be complete.
1027         {
1028           // Catch self-reset errors.
1029           _GLIBCXX_DEBUG_ASSERT(__p == 0 || __p != _M_ptr);
1030           __shared_ptr(__p).swap(*this);
1031         }
```
不仅如此，`__shared_ptr`、`__shared_count` 的有参构造函数都是模板函数。所以通过模板推断，可以推断出 reset() 传入指针的类型，然后传入相应的派生类，因此可以正常析构。