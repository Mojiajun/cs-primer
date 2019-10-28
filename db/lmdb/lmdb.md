# Lighting Memory-Mapped Database Manager (LMDB) 分析
## 简介
LMDB 是一个基于B-树的数据库管理库，其接口与 BerkeleyDB 类似，但是更加简介。LMDB 基本做法是使用内存映射，每次数据读取都直接从映射都内存区域返回数据，因此在数据读取期间不会调用 malloc() 或者 memcpy() 函数。因此，LMDB 没有页面缓存，结构相对简单，并且具有极高都性能和内存效率。此外，LMDB 也具有完全ACID语义的事务性，并且当内存映射为只读时，数据库完整性不会被应用程序代码中的野指针（stray pointer）写入破坏。

LMDB 支持来自多个进程和线程的并发读/写访问。数据页使用写时复制（Copy-on-Write）策略，因此不会覆盖任何活动数据页，这也可以防止损坏，并且在系统崩溃后无需任何特殊的恢复过程。写入是完全序列化的，一次只能有一个写事务处于活动状态，这保证了写程序永远不会死锁。数据库结构是多版本的，读操作不需要占用任何锁，写操作不会阻塞读操作，读操作也不会阻塞写操作。

与其他使用预写事务日志或者只追加写入都著名数据库机制不同，LMDB 在操作期间不需要任何维护。预写事务日志或者只追加写入都需要定期检查或合并其日志或数据库文件，否在它们会无限制地增长。LMDB 跟踪数据库中都空闲页面，将它们重新用于新的写操作，因此正常使用下，数据库不会无限制增长。

内存映射可以用作只读或读写映射。默认情况下它是只读的，因为它可以完全防止损坏。使用读写模式可提供更高的写入性能，但是增加了由于应用程序的野指针引起数据库破坏都可能性。当然，如果你的程序没有任何错误（bug-free），这些都不是事。

## 简单使用
### 环境（Environment）
LMDB 一切从环境开始，由函数 mdb_env_create() 创建。环境提供了访问单个数据库文件所需都数据结构，因此访问的数据库文件和打开都环境总是一一对应的关系。每个进程的每个数据库文件只能打开一个环境，通常，打开都环境一般不关闭它，一直保存到进程结束。一旦创建完成，就可以用 mdb_env_open() 打开。mdb_env_open() 传递了一个路径名，并且该路径必须已经存在，不存在不会创建。该路径下，讲生成一个锁文件和一个存储文件。如果你不想使用目录，则可以传递 MDB_NOSUBDIR 选项。在这种情况下，你提供都路径将直接用作数据文件，并且另一个使用“-lock”后缀都名在但做锁文件。通常在调用 mdb_env_create() 之后，调用 mdb_env_open() 之前，调用 mdb_env_set_mapsize() 设置环境内存映射的大小。当然，也可以在后面多次调用，但调用者必须确保此时没有事务处于活动状态。大小值应该是操作系统页面大小都整数倍，默认值为 10465760 字节（10MB）。内存映射的大小也是数据库的最大大小，该值应选择尽可能大，以适应数据库将来都增长。另外，如果需要一个环境中需要多个数据库，必须在mdb_env_create() 之后和 mdb_env_open() 之前调用 mdb_env_set_maxdbs()，以设置要支持的最大命名数据库数。将环境用作单个未命名数据库的简单应用程序可以忽略此选项。
```
MDB_env *env;
if (const int ret = mdb_env_create(&env)) {
  // ERROR
}
mdb_env_set_maxdbs(env, 50);
mdb_env_set_mapsize(env, (size_t)1048576 * (size_t)100000); // 1MB * 100000
if (const int ret = mdb_env_open(env, "/path/to/database", MDB_NOTLS | MDB_RDONLY, 0664)) {
  // ERROR
}
```

### 事务（Transaction）
当环境打开后，就可以调用 mdb_txn_begin() 在里面创建一个事务。与数据库内容的任何交互（读取或写入）都必须进行事务处理。事务始终适用于整个环境（代表数据库文件），而不是针对单个命名数据库。事务提供完整的ACID语义。事务可以是读写的，也可以是只读的，而读写事务可以嵌套。一个事务一次只能由一个线程使用。事务提供了数据的一致视图。
```
MDB_txn *parent_transaction = nullptr;
MDB_txn *transaction;
if (const int ret = mdb_txn_begin(env, parent_transaction, readonly ? MDB_RDONLY : 0, &transaction)) {
  // ERROR
}
if (read_only) {
  mdb_txn_abort(transaction);
} else {
  mdb_txn_commit(transaction);
}
```

### 数据库（Database）
创建事务后，可以使用mdb_dbi_open() 在其中打开数据库。如果在该环境中仅使用一个数据库，则可以将 NULL 传递为数据库名称。对于命名数据​​库，如果数据库不存在，则必须使用 MDB_CREATE 标志来创建数据库。命名数据库每个进程仅应打开一次，然后应重用 DBI。 打开数据库时必须格外小心，因为打开同一个数据库时不得有两个并发事务（有关警告的更多信息）。
```
MDB_dbi dbi;
if (const int ret = mdb_dbi_open(transacton, "databasename", MDB_DUPSORT | MDB_CREATE, &dbi)) {
  // ERROR
}
```
在事务中，如果您只需要 mdb_get() 和 mdb_put() 可以存储单个键-值对。键-值对表示为两个 MDB_val 结构。该结构具有两个字段，mv_size 和 mv_data。mv_data 是一个 void 指针，只想大小为 mv_size 个字节都数组。

### 游标（Cursor）
游标允许您使用排序的键在数据库中查找键，然后遍历前一个或后一个键。在事务内，可以使用 mdb_cursor_open() 创建一个游标。使用此光标，我们可以使用 mdb_cursor_get()、mdb_cursor_put() 和 mdb_cursor_del() 来存储/检索/删除（多个）值。mdb_cursor_get() 的位置取决于所请求的光标操作，对于某些操作，取决于提供的键。例如，要列出数据库中的所有键-值对，请对第一个 mdb_cursor_get() 调用使用 MDB_FIRST 操作，并在随后的调用中使用 MDB_NEXT，直到结束。要检索从指定键值开始的所有键，请使用 MDB_SET。当使用mdb_cursor_put() 时，该函数将根据键为您定位光标，或者您可以使用 MDB_CURRENT 操作使用光标的当前位置。请注意，键必须与当前位置的键匹配。
```
MDB_cursor *cursor;
if (const int ret = mdb_cursor_open(transaction, dbi, &cursor)) {
  // ERROR
}

MDB_val key = {(size)str.size(), (void*)str.data()};
MDB_val data;

if (const int ret = mdb_cursor_get(cursor, &key, &data, MDB_SET_RANGE)) {
  // NO VALUE FOUND
  mdb_cursor_close(cursor);
  return 0;
}
mdb_cursor_get(cursor, &key, &data, MDB_NEXT);
mdb_cursor_close(cursor);
```