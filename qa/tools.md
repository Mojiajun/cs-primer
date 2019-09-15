# 开发工具问题
## GDB
### gdb调试的一些指令
### 介绍GDB和常用调试命令；
### 调试手段和流程？
### 用过的调试工具有哪些？（GDB等）
### 程序挂掉了，怎么调试？有什么工具？怎么做？
## Git
<img src='./imgs/tools-git-operations.png'>

### Git 配置
- git config --global/local user.name "your name"
- git config --global/local user.email "your@example.com"
- git config --global/local --unser

global 的配置保存在 ~/.gitconfig 中，而 local 的文件保存在 $project/.git/config 中

### Git 常用基本操作命令
- git init：初始化仓库
- git status：显式工作区和暂存区的当前状态
- git add file1 file2 ...：将工作区文件保存到暂存区
- git commit [file1 file2 ...] -m "commits"：提交暂存区的文件到仓库
- git commit [file1 file2 ...] -amend [-m "new commits"]：修改（添加文件或修改注释）最后一次提交
- git commit [file1 file2 ...] -a "commits"：将工作区的文件添加到仓库
- git rm file1 file2 ...：删除暂存区的文件
- git mv old new：移动（重命名）暂存区的文件
- git rm --cached file1 file2 ...：停止跟踪指定文件，但该文件保留在工作区
- git diff：显式暂存区的文件和工作区的文件的不同
- git diff #id1 #id2：两个提交之间的不同

### Git 远程仓库
- git remote add #name #url：添加远程仓库 #name（远端默认名字为 origin，可以指定为其他名字）
- git remote rename #old #name：远程仓库重命名
- git remote remove #name：删除远程仓库
- git clone #url：克隆一个远端仓库到本地
- git push #remote #local：初次把本地仓库 #local 推送到远端仓库 #remote（初次推送添加 -u 选项）
- git fetch #remote：拉取远端仓库到本地
- git pull #remote #local：拉取远端仓库到本地并合并到 HEAD

### Git 版本回退
- git log：显示提交日志
- git reflog：查看命令历史
- git reset --hard #id：撤销提交，返回到 #id 版本
- git reset --hard HEAD^：撤销提交，回退到上一个版本，HEAD^^ 表示上上个版本，HEAD~100 上 100 个版本
- git reset HEAD [file1 file2 ...]：撤销暂存区的修改，重新放回工作区
- git checkout -- [file1 file2 ...]：撤销工作区的修改（撤销后和暂存区的文件一致）
- git checkout #id/branch -- [file1 file2 ...]：恢复 #id 的文件到工作区

### Git 分支版本控制
- git branch：查看当前分支，列出所有分支，当前分支前加 *
- git branch #name：创建 #name 分支
- git branch -m #oldname #newname：分支重命名
- git branch -d #name：删除分支
- git checkout #name：切换到 #name 分支
- git checkout -b #name：创建并切换到 #name 分支
- git merge #name：合并指定分支到当前分支
- git rebase #name：（换基）将但前分支接到 #name 后面

### Git 中 fetch 和 pull 的区别
git fetch 就是把远程仓库的代码下载过来，但是不会自动将远程 origin 分支合并到本地 master 分支。而 git pull 就是把远程仓库的代码下载过来，同时自动将远程 origin 分支合并到本地 master 分支

### Git中 rebase 和 merge 的区别？
git merge 自动根据两个分支的最近共同祖先提交和这两个分支的最新提交进行三方合并，合并后并生成一个新的提交，新的提交表示本次合并修改的内容，两个分支的所有提交历史都会保留。
```
git checkout mywork
git merge orgin
```
<img src='./imgs/tools-git-merge.jpg'>

git rebase 将两个分支的提交历史进行合并，它的原理是首先找到这两个分支的最近共同祖先，然后对比当前分支相对于该祖先的历次提交，依次提取相应的修改并存为临时文件，然后将当前分支指向目标基底最新提交，最后将之前另存为临时文件的修改添加到基底后面生成新的提交
```
git checkout mywork
git rebase origin
```
<img src='./imgs/tools-git-rebase.jpg'>

## linux命令
### 知道哪些Linux命令
#### ps aux
显式系统中所有的进程（静态，某个时刻的快照），输出字段含义
- USER：该进程属于那个用户
- PID：进程 ID
- CPU：该进程占用 CPU 资源的百分比
- MEM：该进程占用物理内存的百分比
- VSZ：改进程使用的虚拟内存，单位是 KB
- RSS：占用非交换区的内存，单位是 KB
- TTY：该进程是在那个终端机运行
- STATE：进程目前的状态
  - D：不可中断
  - R：正在运行或可以运行
  - S：正在睡眠但是可以被信号唤醒
  - T：终止状态
  - Z：该进程是僵尸进程

#### top
实时显式系统中各个进程的资源占用，显式内容如下
- 第一行：当前时间、运行时间、登录用户数、系统负载（load average）
- 第二行显式进程数：进程总数（total）、正在运行的进程数（running）、睡眠的进程数量（sleeping）、停止的进程数量（stopped）、僵尸进程的数目（zombie）
- 第三行显式 CPU 占用相关：用户空间占用百分比（us）、内核空间占用百分比（sy）、用户空间改变过优先级的进程占用百分比（ni）、空闲百分比（id）、等待输入百分比（wa）、处理硬件中断百分比（hi）、处理软中断百分比（si）、用于虚拟 CPU 的百分比（st）
- 第四行显式内存相关：总大小（total）、空闲（free）、使用（used）、用作内核缓存的内存总量（buff/cache）
- 第五行显式交换空间相关：总大小（total）、空闲（free）、使用（used）、缓冲的交换区总量（avail Mem）
- 第六行显式：PID 进程号、用户 USER、优先级 PR、nice值 NI、虚拟内存用量 VIRT、物理内存用量 RES、共享内存用量 SHR、进程状态 S、CPU占用、物理内存占用 MEM、累计 CPU 占用时间 TIME+、命令行 COMMAND。

#### free
free显示系统中可用和已用物理和交换内存的总量，以及内核使用的缓冲区和高速缓存。显式的字段
- total：物理内存大小
- used：使用内存量，计算方式 total - free - buffers - cache
- free：空闲内存量
- shared：
- buff/cache：内核缓冲和 slab 或 page 的 cache 使用总量
- available：可用内存

### top 中的 load average？
top 中的 load average 显式的是最近 1 分钟、5 分钟、15 分钟的系统平均负载。按照经验，若数值小于 0.7 * #CPU，则系统工作正常；若超过这个值，甚至达到 CPU 核数的四五倍，则系统的负载就明显偏高。那么，我们不妨把这个CPU 想象成一座大桥，桥上只有一条车道，所有车辆都必须从这条车道上通过。系统负荷为 0，意味着大桥上一辆车也没有；系统负荷为0.5，意味着大桥一半的路段有车；系统负荷为 1.0，意味着大桥的所有路段都有车，也就是说大桥已经"满"了。但是必须注意的是，直到此时大桥还是能顺畅通行的。系统负荷为 1.7，意味着车辆太多了，大桥已经被占满了（100%），后面等着上桥的车辆为桥面车辆的 70%。以此类推，系统负荷 2.0，意味着等待上桥的车辆与桥面的车辆一样多；系统负荷 3.0，意味着等待上桥的车辆是桥面车辆的 2 倍。总之，当系统负荷大于1，后面的车辆就必须等待了；系统负荷越大，过桥就必须等得越久。CPU 的系统负荷，基本上等同于上面的类比。大桥的通行能力，就是 CPU 的最大工作量；桥梁上的车辆，就是一个个等待 CPU 处理的进程（process）。

### Linux了解不，df du区别，如何格式化磁盘，磁盘挂载过程，tcp丢包如何排查，docker是用什么实现的
### tcpdump抓包
### 说出LINUX命令：磁盘存储查询、内存使用查询、CPU信息查询

### 什么是内存泄露，怎么检查？
简单地说就是程序在堆空间上申请了一块内存空间，使用完毕后没有释放掉，且没有任何一个指针指向它，那么这块内存就泄露了。它的一般表现方式是程序运行时间越长，占用内存越多，最终用尽全部内存，整个系统崩溃。

首先需要有良好的编码习惯，尽量使用 C++ 的智能指针管理动态内存。然后借助一些工具分析代码中有无内存泄漏的可能，比如编译通过后，用 cppcheck 跑一遍，能解决不少代码中明显的内存泄漏。运行的时候用 valgrind 跑，可以检查运行时候存在的内存泄漏。

### 如何查看进程负载和内存泄漏？

### vim开发的相关工具？
### shell脚本中0,1,2的意思