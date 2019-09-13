# std::alogrithm 常用函数
## std::sort()
标准库里面 sort() 有两个接口，其中一个可以指定元素间比较的方法。默认的比较方式是 *it1 < *it2。需要注意的是，sort() 接受的迭代器类型是随机访问迭代器，其他迭代器不支持。
```
/// @file bits/stl_algo.h
4687   template<typename _RandomAccessIterator>
4688     inline void
4689     sort(_RandomAccessIterator __first, _RandomAccessIterator __last)
4690     {
4691       // concept requirements
4692       __glibcxx_function_requires(_Mutable_RandomAccessIteratorConcept<
4693         _RandomAccessIterator>)
4694       __glibcxx_function_requires(_LessThanComparableConcept<
4695         typename iterator_traits<_RandomAccessIterator>::value_type>)
4696       __glibcxx_requires_valid_range(__first, __last);
4697 
4698       std::__sort(__first, __last, __gnu_cxx::__ops::__iter_less_iter());
4699     }

4716   template<typename _RandomAccessIterator, typename _Compare>
4717     inline void
4718     sort(_RandomAccessIterator __first, _RandomAccessIterator __last,
4719      _Compare __comp)
4720     {
4721       // concept requirements
4722       __glibcxx_function_requires(_Mutable_RandomAccessIteratorConcept<
4723         _RandomAccessIterator>)
4724       __glibcxx_function_requires(_BinaryPredicateConcept<_Compare,
4725         typename iterator_traits<_RandomAccessIterator>::value_type,
4726         typename iterator_traits<_RandomAccessIterator>::value_type>)
4727       __glibcxx_requires_valid_range(__first, __last);
4728 
4729       std::__sort(__first, __last, __gnu_cxx::__ops::__iter_comp_iter(__comp));
4730     }
```
两者的都是继续调用函数 std::__sort()。

### std::__sort()
\__sort() 依次调用两个函数 \__introsort_loop() 和 \__final_insertion_sort()。
```
/// @file bits/stl_algo.h
1956   template<typename _RandomAccessIterator, typename _Compare>
1957     inline void
1958     __sort(_RandomAccessIterator __first, _RandomAccessIterator __last,
1959            _Compare __comp)
1960     {
1961       if (__first != __last)
1962         {
1963           std::__introsort_loop(__first, __last,
1964                                 std::__lg(__last - __first) * 2, // 递归深度
1965                                 __comp);
1966           std::__final_insertion_sort(__first, __last, __comp); // 插入排序
1967         }
1968     }
```

### std::\__introsort_loop()
\__introsort_loop() 是内省排序。我们知道快速排序的平均时间复杂度是 O(NlongN)，空间复杂度是 O(logN)（递归调用堆栈），但是其最坏情况下的时间复杂度是 O(N^2)。通常可以通过三数取中（首尾和中间三个数的中位数）选取划分的方式避免进入最坏的情况，但是却增加了一定的复杂度。内省排序是对快速排序的改进，限制了递归调用的深度，一旦递归深度超过某个阈值，就采用堆排序的方式。阈值选为最优划分的调用栈深度的两倍（2*LogN）。此外，对于元素个数小于一定数目（16）的区间，不再进行排序处理。

\__introsort_loop() 接受四个参数，前两个参数是迭代器，表示对区间 [\__first, \__last) 的数据进行排序，最后一个参数是元素间的比较方法。剩下一个参数表示最大递归深度。
```
/// @file bits/stl_algo.h
1933   template<typename _RandomAccessIterator, typename _Size, typename _Compare>
1934     void
1935     __introsort_loop(_RandomAccessIterator __first,
1936                      _RandomAccessIterator __last,
1937                      _Size __depth_limit, _Compare __comp)
1938     {
1939       while (__last - __first > int(_S_threshold)) // enum { _S_threshold = 16 };
1940         {
1941           if (__depth_limit == 0) // 达到最大递归调用深度
1942             {
1943               std::__partial_sort(__first, __last, __last, __comp); // 采用堆排序
1944               return;
1945             }
1946           --__depth_limit; // 否则继续递归调用，可递归深度减1
1947           _RandomAccessIterator __cut =
1948             std::__unguarded_partition_pivot(__first, __last, __comp); // 选择pivot
1949           std::__introsort_loop(__cut, __last, __depth_limit, __comp); // 递归后一段
1950           __last = __cut; // 前一段继续循环
1951         }
1952     }
```
接着来看 \__unguarded_partition_pivot() 返回值是什么
```
/// @file bits/stl_algo.h
1910   template<typename _RandomAccessIterator, typename _Compare>
1911     inline _RandomAccessIterator
1912     __unguarded_partition_pivot(_RandomAccessIterator __first,
1913                                 _RandomAccessIterator __last, _Compare __comp)
1914     {
1915       _RandomAccessIterator __mid = __first + (__last - __first) / 2; // 中间位置的元素
1916       std::__move_median_to_first(__first, __first + 1, __mid, __last - 1,
1917                                   __comp); // frist和{first+1, mid, last-1}的中位数交换
1918       return std::__unguarded_partition(__first + 1, __last, __first, __comp);
1919     }
```
\__move_median_to_first() 的作用是选取 pivot，方法是选取三元组 {\__first+1, \__mid, \__last-1} 的中位数与 \__first 交换，也就是说函数返回后，\__first 保存的是 pivot。\__unguarded_partition() 定义如下：
```
/// @file bits/stl_algo.h
1889   template<typename _RandomAccessIterator, typename _Compare>
1890     _RandomAccessIterator
1891     __unguarded_partition(_RandomAccessIterator __first,
1892                           _RandomAccessIterator __last,
1893                           _RandomAccessIterator __pivot, _Compare __comp)
1894     {
1895       while (true)
1896         {
1897           while (__comp(__first, __pivot))
1898             ++__first;
1899           --__last;
1900           while (__comp(__pivot, __last))
1901             --__last;
1902           if (!(__first < __last))
1903             return __first;
1904           std::iter_swap(__first, __last);
1905           ++__first;
1906         }
1907     }
``` 
\__unguarded_partition() 的操作就是对区间 [\__first, \__last) 按照 \__pivot 进行一次划分，不过在划分结束后，不会将 \__pivot 放到正确的位置上（\__pivot 的位置始终保持不变）。其返回值为划分后的区间上第一个大于 \__pivot 的位置。比如说，对数据 {4, 7, 5, 6, 2，3, 1} 以 4 为 pivot 划分后为 {4，1, 3, 2, 6  5, 7}，返回的值下标 4。 从上面的分析知道，在 \__introsort_loop() 返回后，整体并不是有序的，而是段间有序，段内无序，并且每段的长度不会超过 \_S_threshold 。比如，可能出现下列的结果：
```
11 2 9 0 3 7 1 6 4 5 10 8 |    /* 第1段 */
13 17 14 12 15 16 18 |         /* 第2段 */
27 19 23 25 20 22 24 26 21 |   /* 第3段 */
34 32 30 33 29 28 31 |         /* 第4段 */
37 41 46 43 40 44 39 36 |      /* 第5段 */
45 35 38 42 48 47 49 |         /* 第6段 */
```
然后在进行插入排序，是整体有序。插入排序调用 \__final_insertion_sort()

### std::\__final_insertion_sort()
```
1873   template<typename _RandomAccessIterator, typename _Compare>
1874     void
1875     __final_insertion_sort(_RandomAccessIterator __first,
1876                            _RandomAccessIterator __last, _Compare __comp)
1877     {
1878       if (__last - __first > int(_S_threshold)) // 大于阈值
1879         {
1880           std::__insertion_sort(__first, __first + int(_S_threshold), __comp);
1881           std::__unguarded_insertion_sort(__first + int(_S_threshold), __last,
1882                                           __comp); // 后面的继续分段插入排序
1883         }
1884       else
1885         std::__insertion_sort(__first, __last, __comp);
1886     }
```
先看一下插入排序 std::\__insertion_sort() 的实现
```
1833   template<typename _RandomAccessIterator, typename _Compare>
1834     void
1835     __insertion_sort(_RandomAccessIterator __first,
1836                      _RandomAccessIterator __last, _Compare __comp)
1837     {
1838       if (__first == __last) return;
1839
1840       for (_RandomAccessIterator __i = __first + 1; __i != __last; ++__i)
1841         { // [first, i)是有序的
1842           if (__comp(__i, __first)) // [first, i-1]整体后移一个位置，然后放置i
1843             { // i需要放到
1844               typename iterator_traits<_RandomAccessIterator>::value_type
1845                 __val = _GLIBCXX_MOVE(*__i); // 暂存i的值
1846               _GLIBCXX_MOVE_BACKWARD3(__first, __i, __i + 1); // 整体后移动
1847               *__first = _GLIBCXX_MOVE(__val); // 放入i的值
1848             }
1849           else // [x, i-1]整体后移一个位置，然后放置i
1850             std::__unguarded_linear_insert(__i,
1851                                 __gnu_cxx::__ops::__val_comp_iter(__comp));
1852         }
1853     }
```
接着分析 std::\____unguarded_linear_insert() 的实现
```
1814   template<typename _RandomAccessIterator, typename _Compare>
1815     void
1816     __unguarded_linear_insert(_RandomAccessIterator __last,
1817                               _Compare __comp)
1818     { // [, last-1]有序
1819       typename iterator_traits<_RandomAccessIterator>::value_type
1820         __val = _GLIBCXX_MOVE(*__last); // 暂存last的值
1821       _RandomAccessIterator __next = __last;
1822       --__next;
1823       while (__comp(__val, __next))
1824         { // 每次移动一个
1825           *__last = _GLIBCXX_MOVE(*__next);
1826           __last = __next;
1827           --__next;
1828         }
1829       *__last = _GLIBCXX_MOVE(__val); // 放置val
1830     }
```
需要注意的是，\__unguarded_linear_insert() 无法处理将 last 插入到首元素的位置的情况，所以调用前必须确保插入位置不再首元素位置。如果可以保证，则不必要对 next 进行越界检查。从上面分析可以看到，\__insertion_sort() 就是普通的插入排序。只不过进行了优化，当插入位置在第一个元素位置时，调用 std::move_backward() 整体移动一个元素的偏移（可以处理区间重合的情况）。否则，调用 std::move() 一个元素一个元素移动。接下来分析 std::\__unguarded_insertion_sort()
```
1856   template<typename _RandomAccessIterator, typename _Compare>
1857     inline void
1858     __unguarded_insertion_sort(_RandomAccessIterator __first,
1859                                _RandomAccessIterator __last, _Compare __comp)
1860     {
1861       for (_RandomAccessIterator __i = __first; __i != __last; ++__i)
1862         std::__unguarded_linear_insert(__i,
1863                                 __gnu_cxx::__ops::__val_comp_iter(__comp));
1864     }
```
可以看到，\__unguarded_insertion_sort() 直接调用 \__unguarded_linear_insert()。从上面的分析可知，\__unguarded_linear_insert() 不能处理插入位置为首元素的位置，所有必须确保 [\__i, \__last) 区间的元素不会有首元素。我们看到，在调用\__unguarded_insertion_sort() 前，调用了 \__insertion_sort() 对 [\__first, \__first + \_S_threshold) 区间的元素进行插入排序。而 \__introsort_loop() 返回后，段间有序且段最长不会超过 \_S_threshold)，因此 [\__first + \_S_threshold, \__last) 区间的元素都比 [\__first, \__first + \_S_threshold) 大。因此保证了不会在首元素地方插入。

## std::move()
对迭代器的要求很低，其作用是将区间 [\__first, \__last) 的数据移动到 [\__result, \__result + \__last - \__frist)。底层会尽力调用 memmove()。可以处理区间重叠的情况。
```
/// @file bits/stl_algobase.h
491   template<typename _II, typename _OI>
492     inline _OI
493     move(_II __first, _II __last, _OI __result)
494     {
495       // concept requirements
496       __glibcxx_function_requires(_InputIteratorConcept<_II>)
497       __glibcxx_function_requires(_OutputIteratorConcept<_OI,
498             typename iterator_traits<_II>::value_type>)
499       __glibcxx_requires_valid_range(__first, __last);
500 
501       return std::__copy_move_a2<true>(std::__miter_base(__first),
502                                        std::__miter_base(__last), __result);
503     }
```

## std::move_backward()
move_backward() 传入的迭代器必须是双向迭代器（bidirectional iterator），作用是从最后一个元素开始将区间 [\__first, \__last) 的数据移动到区间 [\__result + \__first - \__last, \__result)，返回指向最后一个移动的元素的迭代器，即 \__result + \__first - \__last 的结果。move_backward() 底层函数会尽一切努力去调用 memmove() 函数。可以处理区间重叠的情况。
```
/// @file bits/stl_algobase.h
668   template<typename _BI1, typename _BI2>
669     inline _BI2
670     move_backward(_BI1 __first, _BI1 __last, _BI2 __result)
671     {
672       // concept requirements
673       __glibcxx_function_requires(_BidirectionalIteratorConcept<_BI1>)
674       __glibcxx_function_requires(_Mutable_BidirectionalIteratorConcept<_BI2>)
675       __glibcxx_function_requires(_ConvertibleConcept<
676             typename iterator_traits<_BI1>::value_type,
677             typename iterator_traits<_BI2>::value_type>)
678       __glibcxx_requires_valid_range(__first, __last);
679 
680       return std::__copy_move_backward_a2<true>(std::__miter_base(__first),
681                                                 std::__miter_base(__last),
682                                                 __result);
683     }
```

## std::search()
```
/// @file bits/stl_algo.h
4020   template<typename _ForwardIterator1, typename _ForwardIterator2>
4021     inline _ForwardIterator1
4022     search(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
4023            _ForwardIterator2 __first2, _ForwardIterator2 __last2)
4024     {
4025       // concept requirements
4026       __glibcxx_function_requires(_ForwardIteratorConcept<_ForwardIterator1>)
4027       __glibcxx_function_requires(_ForwardIteratorConcept<_ForwardIterator2>)
4028       __glibcxx_function_requires(_EqualOpConcept<
4029             typename iterator_traits<_ForwardIterator1>::value_type,
4030             typename iterator_traits<_ForwardIterator2>::value_type>)
4031       __glibcxx_requires_valid_range(__first1, __last1);
4032       __glibcxx_requires_valid_range(__first2, __last2);
4033 
4034       return std::__search(__first1, __last1, __first2, __last2,
4035                            __gnu_cxx::__ops::__iter_equal_to_iter());
4036     }
4037 
4059   template<typename _ForwardIterator1, typename _ForwardIterator2,
4060            typename _BinaryPredicate>
4061     inline _ForwardIterator1
4062     search(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
4063            _ForwardIterator2 __first2, _ForwardIterator2 __last2,
4064            _BinaryPredicate  __predicate)
4065     {
4066       // concept requirements
4067       __glibcxx_function_requires(_ForwardIteratorConcept<_ForwardIterator1>)
4068       __glibcxx_function_requires(_ForwardIteratorConcept<_ForwardIterator2>)
4069       __glibcxx_function_requires(_BinaryPredicateConcept<_BinaryPredicate,
4070             typename iterator_traits<_ForwardIterator1>::value_type,
4071             typename iterator_traits<_ForwardIterator2>::value_type>)
4072       __glibcxx_requires_valid_range(__first1, __last1);
4073       __glibcxx_requires_valid_range(__first2, __last2);
4074 
4075       return std::__search(__first1, __last1, __first2, __last2,
4076                            __gnu_cxx::__ops::__iter_comp_iter(__predicate));
4077     }
```
调用 \__search()
```
/// @file bits/stl_algo.h
202   template<typename _ForwardIterator1, typename _ForwardIterator2,
203            typename _BinaryPredicate>
204     _ForwardIterator1
205     __search(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
206              _ForwardIterator2 __first2, _ForwardIterator2 __last2,
207              _BinaryPredicate  __predicate)
208     {
209       // Test for empty ranges
210       if (__first1 == __last1 || __first2 == __last2)
211         return __first1;
212 
213       // Test for a pattern of length 1.
214       _ForwardIterator2 __p1(__first2);
215       if (++__p1 == __last2)
216         return std::__find_if(__first1, __last1,
217                 __gnu_cxx::__ops::__iter_comp_iter(__predicate, __first2));
218 
219       // General case.
220       _ForwardIterator2 __p;
221       _ForwardIterator1 __current = __first1;
222 
223       for (;;)
224         {
225           __first1 =
226             std::__find_if(__first1, __last1,
227                 __gnu_cxx::__ops::__iter_comp_iter(__predicate, __first2));
228 
229           if (__first1 == __last1)
230             return __last1;
231 
232           __p = __p1;
233           __current = __first1;
234           if (++__current == __last1)
235             return __last1;
236 
237           while (__predicate(__current, __p))
238             {
239               if (++__p == __last2)
240                 return __first1;
241               if (++__current == __last1)
242                 return __last1;
243             }
244           ++__first1;
245         }
246       return __first1;
247     }
```

## std::lower_bound()
返回区间 [\__first，\__last) 中不小于（即大于或等于）\__val 的第一个元素的迭代器，如果没有找到这样的元素，则返回\__last。直接调用 \__lower_bound() 函数
```
/// @file bits/stl_algobase.h
 992   template<typename _ForwardIterator, typename _Tp>
 993     inline _ForwardIterator
 994     lower_bound(_ForwardIterator __first, _ForwardIterator __last,
 995                 const _Tp& __val)
 996     {
 997       // concept requirements
 998       __glibcxx_function_requires(_ForwardIteratorConcept<_ForwardIterator>)
 999       __glibcxx_function_requires(_LessThanOpConcept<
1000             typename iterator_traits<_ForwardIterator>::value_type, _Tp>)
1001       __glibcxx_requires_partitioned_lower(__first, __last, __val);
1002 
1003       return std::__lower_bound(__first, __last, __val,
1004                                 __gnu_cxx::__ops::__iter_less_val());
1005     }
```
\__lower_bound() 函数会根据不同迭代器类型进行优化。传入随机迭代器最后的算法是二分查找
```
/// @file bits/stl_algobase.h
954   template<typename _ForwardIterator, typename _Tp, typename _Compare>
955     _ForwardIterator
956     __lower_bound(_ForwardIterator __first, _ForwardIterator __last,
957                   const _Tp& __val, _Compare __comp)
958     {
959       typedef typename iterator_traits<_ForwardIterator>::difference_type
960         _DistanceType;
961       // 不同迭代器类型 distance性能不同
962       _DistanceType __len = std::distance(__first, __last);
963 
964       while (__len > 0)
965         {
966           _DistanceType __half = __len >> 1;
967           _ForwardIterator __middle = __first;
968           std::advance(__middle, __half); // 不同迭代器类型性能不同
969           if (__comp(__middle, __val))
970             {
971               __first = __middle;
972               ++__first;
973               __len = __len - __half - 1;
974             }
975           else
976             __len = __half;
977         }
978       return __first;
979     }
```

## std::upper_bound()
返回区间 [\__first，\__last) 中大于（即大于或等于）\__val 的第一个元素的迭代器，如果没有找到这样的元素，则返回\__last。直接调用 \__lower_bound() 函数
```
/// @file bits/stl_algo.h
2035   template<typename _ForwardIterator, typename _Tp, typename _Compare>
2036     _ForwardIterator
2037     __upper_bound(_ForwardIterator __first, _ForwardIterator __last,
2038                   const _Tp& __val, _Compare __comp)
2039     {
2040       typedef typename iterator_traits<_ForwardIterator>::difference_type
2041         _DistanceType;
2042 
2043       _DistanceType __len = std::distance(__first, __last);
2044 
2045       while (__len > 0)
2046         {
2047           _DistanceType __half = __len >> 1;
2048           _ForwardIterator __middle = __first;
2049           std::advance(__middle, __half);
2050           if (__comp(__val, __middle))
2051             __len = __half;
2052           else
2053             {
2054               __first = __middle;
2055               ++__first;
2056               __len = __len - __half - 1;
2057             }
2058         }
2059       return __first;
2060     }
2061 
2073   template<typename _ForwardIterator, typename _Tp>
2074     inline _ForwardIterator
2075     upper_bound(_ForwardIterator __first, _ForwardIterator __last,
2076                 const _Tp& __val)
2077     {
2078       typedef typename iterator_traits<_ForwardIterator>::value_type
2079         _ValueType;
2080 
2081       // concept requirements
2082       __glibcxx_function_requires(_ForwardIteratorConcept<_ForwardIterator>)
2083       __glibcxx_function_requires(_LessThanOpConcept<_Tp, _ValueType>)
2084       __glibcxx_requires_partitioned_upper(__first, __last, __val);
2085 
2086       return std::__upper_bound(__first, __last, __val,
2087                                 __gnu_cxx::__ops::__val_less_iter());
2088     }
2089 
2105   template<typename _ForwardIterator, typename _Tp, typename _Compare>
2106     inline _ForwardIterator
2107     upper_bound(_ForwardIterator __first, _ForwardIterator __last,
2108                 const _Tp& __val, _Compare __comp)
2109     {
2110       typedef typename iterator_traits<_ForwardIterator>::value_type
2111         _ValueType;
2112 
2113       // concept requirements
2114       __glibcxx_function_requires(_ForwardIteratorConcept<_ForwardIterator>)
2115       __glibcxx_function_requires(_BinaryPredicateConcept<_Compare,
2116                                   _Tp, _ValueType>)
2117       __glibcxx_requires_partitioned_upper_pred(__first, __last,
2118                                                 __val, __comp);
2119 
2120       return std::__upper_bound(__first, __last, __val,
2121                                 __gnu_cxx::__ops::__val_comp_iter(__comp));
2122     }
```