# 数据库系统概论

## 关系模型介绍
- 关系数据库的结构
  - 关系数据库由表的集合构成，每个表有唯一的名字
  - 表：元组的集合（无序性，通常按其第一个属性排序）
  - 行：元组（一组值的序列）
  - 列：属性
  - 域：允许取值的集合
- 数据库模式
  - 数据库模式：数据库的逻辑设计
  - 数据库实例：给定时刻数据库中的一个快照
- 码（整个关系的一种性质）
  - 超码：一个或多个属性的集合，可以在一个关系中唯一地标识一个元组
  - 候选码：任意真子集都不能成为超码的超码
  - 主码：被数据库设计者选中的，主要用来在一个关系中区分不同元组的候选码
  - 外码：一个关系模式（r2）可能在它的属性中包含另一个关系模式（r2）的主码，这个属性在r1上称作参照r2的外码

## SQL

- 基本类型
  - char(n)：固定长度（n）字符串
  - varchar(n)：可变长度（最大长度n）字符串
  - int：整型
  - smallint：小整型
  - numeric(p, d)：定点数，精度由用户控制；p表示位数，d表示精度（小数位数）
  - real, double precision：浮点数或双精度浮点数
  - float(n)：精度至少为n的浮点数
- 基本模式定义
  ```
  create table instructor
    (ID, varchar(5),
     name varchar(20) not null,
     dept_name, varchar(15),
     salary, numeric(8, 2),
     primary key(ID),
     foreign key(dept_name) reference department);
  ```
  以上在数据库中创建了instructor关系，选ID为主码， dept_name为外码。`create table`的通用形式是：
  ```
  create table r  // 换行不是必须
  (
    A1, D1,
    A2, D2,
    ...,
    An, Dn,
    <完整性约束1>,
    ...,
    <完整性约束2>
  );
  ```
  - 完整性约束实例
    - 主码`primary key(Aj1, Aj2, ...)`
    - 外码`foreign key(Ak1, Ak2, ...) reference 3rd_relation;`
    - `not null`，要求该属性不允许空值
  - 删除`drop table`或`delete from`
- SQL查询的基本结构
  ```
  select A1, A2, ..., An
  from r1, r2, ..., rm
  where condition;
  ```
  - 单关系查询
    ```
    select name from instructor;
    select distinct name from instructor; // 去重
    select ID, name, dept_name, salary*1.1 from instructor; // + - * /运算
    select name, salary from instructor where dept_name = 'Comp. Sci.' and salary > 7000; // 条件<, <=, >, >=, =, <>
    ```
  - 多关系查询
    ```
    select name, instructor.dept_name, building
    from instructor, department
    where instructor.dept_name = department.dept_name
    ```
  - 自然连接
    ```
    select A1, A2, ..., An
    from r1 natural join r2 natural join ... natural join rm;
    ```
  - 只考虑那些在连个关系模式中都出现的属性上取值相同的元组对
    ```
    // tabel r1              // tabel r2
    id  name  code       |   id  grade  code
    1   张三   20181601   |   1   55     20181601
    2   李四   20181602   |   2   88     20181602
    3   小红   20181603   |   3   99     20181605
    4   小明   20181604   |   4   33     20181611
    5   小青   20181605   |
    
    r1 natural join r2
    id  code     name  grade
    1   20181601 张三   55
    2   20181602 李四   88
    ```
  - 选定一个属性名列表 `join...using` 
- 附加的基本运算
  - 更名运算 `old-name as new-name`
    ```
    // 重命名结果关系中的属性
    select name as instructor_name, course_id
    from instructor, teaches
    where instructor.ID = teaches.ID;
    
    // 重命名关系
    select T.name, S.course_id
    from instructor as T, teaches as S
    where T.ID = S.ID; // 长的关系名替换为短的

    selec distinct T.name
    from instructor as T, instructor as S
    where T.salary > S.salary and S.dept_name='Biology'; // 处理同一个关系中的元组 
    ```
  - 字符串运算
    - `like`匹配
      - `%`：匹配任意字子串
      - `_`：匹配任意一个字符
      - `\`：转义字符
      ```
      select dept_name
      from department
      where building like `% Waston%`;
      ```
  - select子句中的属性说明
    - `*`可以在select子句中表示“所有的属性”
  - 排序元组的显式次序
    - `order by`让查询结果中元组按排列顺序显式
    - 默认是升序`asc`
    - 降序`desc`
    ```
    select *
    from instructor
    order by salary desc, name asc;
    ```
  - where子句谓语
    - `between ... and ...` 一个值是小于或等于
    - `not` 取反
    - `(...)`表示元组
- 集合运算
  - `union` 集合并，自动去重，`union all`保留所有重复
    ```
    (select course_id from section where semester = 'Fall' and year = 2009）
    union
    (select course_id from secton where semester = 'Spring' and year = 2010);
    ```
  - `interset` 集合交，自动去重，`interset all`保留所有重复
    ```
    (select course_id from section where semester = 'Fall' and year = 2009）
    interset all
    (select course_id from secton where semester = 'Spring' and year = 2010);
    ```
  - `except` 集合差，从第一个输入中输出所有不出现在第二个输入中的元组，`except all`保留所有重复
    ```
    (select course_id from section where semester = 'Fall' and year = 2009）
    except
    (select course_id from secton where semester = 'Spring' and year = 2010);
    ```
- 空值 `null`
  - 空值与算术运算的结果为空值
  - 空值与比较运算是`unknown`（既不是谓语`is null`，也不是`is not null`）
  - 空值与关系运算
    - and：`true and unknown`结果是`unknown`，`false and unknown`结果是`false`，`unknown and unknown`结果是`unknown`
    - or：`true or unknown`结果是`true`，`false or unknown`结果是`unknown`，`unknown or unknown`结果是`unknown`
    - not：`not unknown`结果是`unknown`
- 聚集函数
  - 以值的一个集合（集或多重集）为输入，返回单个值的函数
    - `avg`：平均值，输入必须是数字集
    - `min`：最小值
    - `max`：最大值
    - `sum`：总和，输入必须是数字集
    - `count`：计数
  - 基本聚集
    ```
    select avg(salary) as avg_salary
    from instructor
    where dept_name = 'Comp. Sci.';

    select count(distinct ID)
    from teaches
    where semester = 'Spring' and year = 2010;
    ```
  - 分组聚集
    ```
    select dept_name, count(distnct ID) as instr_count
    from instructor natural join teaches
    where semester = 'Spring' and year = 2010
    group by dept_name;
    ```
  - having子句：在分组后才其作用
    ```
    select dept_name, avg(salary) as avg_salary
    from instructor
    group by dept_name
    having avg(salary) > 42000;
    ```
  - 对空值或布尔值的聚集
    - 除了`count(*)`外所有的聚集函数都忽略输入集合中的空值
- 嵌套子查询
  - 子查询嵌套在where子句中，通常用于对集合的成员资格、集合的比较以及集合的基数进行检查
  - 集合成员资格`in`或`not in`
    ```
    select distinct course_id
    from section
    where semester = 'Fall' and year = 2009 and
      course_id in (select course_id
                    from section
                    where semester = 'Spring' and year = 2010);
    ```
  - 集合的比较
    ```
    // 找出工资至少比Biology系某一个教师的工资高的教师名字
    select name
    from instructor
    where salary > some (select salary  // 至少比某一个要大
                         from instructor
                         where dept_name = 'Biology');
    ```
    - `< some`，`<= some``> some`，`>= some`，`<> some`, `< all`，`<= all``> all`，`>= all`，`<> all`
    - `> all`比所有的都大
  - 空关系测试
    - `exists`结构在zowie参数的子查询非空时返回`true`
    ```
    // 找出在2009年秋季学期和2010年春季学期同时开课的所有课程
    select course_id
    from section as S
    where semester = 'Fall' and year = 2009 and
        exists(select *
              from section as T
              where semester = 'Spring' and year = 2010 and S.course_id = T.course_id);
    ```
  - 重复元组存在性测试
    - `unique`测试一个子查询结果是否存在重复元组
    ```
    // 找出所有在2009年最多开设一次的课程
    select T.course_id
    from course as T
    where unique(select R.course_id
                from section as R
                where T.course_id = R.course_id and R.year = 2009);
    ```
  - from子句中的子查询
    ```
    // 找出系平均工资超过42000美元的那些系中教师的平均工资
    select dept_name, avg_salary
    from (select dept_name, avg(salary) as avg_salary
          from instructor
          group by dept_name)
    where avg_salary > 42000;
    ```
  - with子句
    - 提供临时关系的方法，这个定义只对包含with子句的查询有效
    ```
    // 找出所有工资总额大于所有系平均工资总额的系
    with dept_total (dept_name, value) as
        (select dept_name, sum(salary)
         from instructor
         group by dept_name),
         dept_total_avg (value) as 
        (select avg(val)
         from dept_total)
    select dept_name
    from dept_total, dept_total_avg
    where dept_total_avg >= dept_total_avg.value;
    ```
  - 标量子查询
    - 返回单个值的子查询
    ```
    // 所有的系以及拥有的教师的数量
    select dept_name,
        (select count(*)
         from instructor
         where department.dept_name = instructor.dept_name)
         as num_instructors
    from department;
    ```
- 数据库的修改
  - 删除 `delete from r where P;`，where子句可以为空，删除整个关系
    ```
    // 从instructor关系中删除与finance系教师相关的所有元组
    delete from instructor
    where dept_name = 'Finance';

    // 从instructor关系中删除所有这样的教师元组，他们在Waston大楼的系工作
    delete from instructor
    where dept_name in (select dept_name 
                        from department
                        where building = 'Waston');
    ```
  - 插入 `insert into r`
    ```
    insert into course
        value('CS-437', 'Database System', 'Comp. Sci.', 4);

    insert into instructor
        select ID, name, dept_name, 18000
        from student
        where dept_name = 'Music' and tot_cred > 144;
    ```
  - 更新 `update r set`
    ```
    // 给工资低于70000美元的教师涨工资
    update instructor
    set salary = salary * 1.05
    where salary < 70000;

    // 给工资超过100000美元的教师涨3%，其余涨5%
    update instructor
    set salare = case
              when salary <= 100000 then salary * 1.05
              else salary * 1.03
        end
    ```

## 中级SQL

- 连接表达式 `join...using`
  - 连接条件
    - `on`条件允许在参与连接的关系上设置通用的谓语，出现在连接表达式的末尾
      ```
      select *
      from student join takes on student.ID = takes.ID;
      ```
  - 外连接
    - 左外连接：只保留出现在左外连接运算之前（左边）的关系中的元组
    - 右外连接：只保留出现在左外连接运算之后（右边）的关系中的元组
    - 全外连接：只保留出现在两个关系中的元组
- 视图
  - 不是逻辑模型的一部分，但作为虚关系对用户可见的关系称为视图
  - 定义 `create view v as <query expression;>`
    ```
    create view physics_fall_2009 as 
      select course.course_id, sec_id, building, room_number
      from course, section
      where course.course_id = section.course_id
        and course.dept_name = 'Physics'
        and section.semester = 'Fall'
        and section.year = '2009';
    ```
  - 查询中使用视图
    ```
    select course_id
    from physics_fall_2009
    where building = 'Waston';
    ```
  - 更新视图 `insert int v`
- 事务
  - 由查询和（或）更新语句的序列组成
  - 下列SQL语句之一会结束一个事务
    - Commit work，提交当前事务，将该事务所做的更新在数据库中持久保存（Rollback也不能撤销）
    - Rollback work，回滚当前事务，撤销该事务中所有SQL语句对数据库的更新
  - 原子性
    - 一个事务或者在完成所有步骤后提交其行为，或者在不能成功完成其所有动作的情况下回滚其所有动作
- 完整性约束
  - 主码
  - not null
    - `name varchar(20) not null;`
  - unique
    - `unique(Aj1, Aj2, ..., Ajm);`属性Aj1，Aj2,。。，Aj形成一个候选码，即在关系中没有两个元组能在所有列出的属性上取值相同
  - check
    - 保证属性值满足指定的条件
    - `check (semester in ('Fall', 'Winter', 'Spring', 'Summer'));`
  - 外码，参照完整性
    `foreign key(dept_name) reference department;`
- 索引
  - 在关系的属性上所创建的一种数据结构，它允许数据库系统高效地找到关系中那些在索引属性上取给定值的元组，而不同扫描关系中的所有元组
- 授权（读取`select`、插入`insert`、更新`update`、删除`delete`）
  - 权限的授予`grant`
    ```
    grant <权限列表（select, insert, update, delete）>
    on <关系名或视图名>
    to <用户/角色列表>

    // 授予数据库用户Ami和Satoshi在department关系上的select权限
    grant select on department to Amit, Satoshi;
    // 授予Amit和Satoshi在department关系的budget属性上的更新权限
    grant update (budget) on department to Amit, Satoshi;
    ```
  - 权限的收回`revoke`
    ```
    revoke <权限列表（select, insert, update, delete）>
    on <关系名或视图名>
    from <用户/角色列表>

    revoke select on department from Amit, Satoshi;
    revoke update (budget) on department from Amit, Satoshi;
    ```
    - 默认级联收回（`cascade`）, 在句末添加`restrict`可以防止级联收回
- 触发器
  - 是一条语句，当对数据库做修改时，它自动被系统执行
  - 设置触发器的条件
    - 指明什么条件下执行触发器。它被分解成一个引起触发器被检测的事件和一个触发器执行必须满足的条件
    - 指明触发器执行是的动作