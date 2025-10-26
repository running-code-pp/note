# 概述
所谓的memory order，其实就是限制编译器以及CPU对单线程当中的指令执行顺序进行重排的程度（此外还包括对cache的控制方法）。这种限制，决定了以atom操作为基准点（边界），对其之前的内存访问命令，以及之后的内存访问命令，能够在多大的范围内自由重排（或者反过来，需要施加多大的保序限制）。从而形成了6种模式。它本身与多线程无关，是限制的单一线程当中指令执行顺序。但是（合理的）指令执行顺序的重排在单线程环境下不会造成逻辑错误而在多线程环境下会，所以这个问题的目的是为了解决多线程环境下出现的问题。

# 理解
```cpp
1. Relaxed ordering: 
在单个线程内，所有原子操作是顺序进行的。
按照什么顺序？基本上就是代码顺序（sequenced-before）。
这就是唯一的限制了！两个来自不同线程的原子操作是什么顺序？两个字：任意。

2. Release -- acquire: 
来自不同线程的两个原子操作顺序不一定？那怎么能限制一下它们的顺序？
这就需要两个线程进行一下同步（synchronize-with）。
同步什么呢？同步对一个变量的读写操作。
线程 A 原子性地把值写入 x (release), 然后线程 B 原子性地读取 x 的值（acquire）. 这样线程 B 保证读取到 x 的最新值。注意 release -- acquire 有个副作用：
线程 A 中所有发生在 release x 之前的写操作，对在线程 B acquire x 之后的任何读操作都可见！
本来 A, B 间读写操作顺序不定。这么一同步，在 x 这个点前后，
A, B 线程之间有了个顺序关系，称作 inter-thread happens-before.

3. Release -- consume: 
只想同步一个 x 的读写操作，结果把 release 之前的写操作都顺带同步了？
如果我想避免这个额外开销怎么办？用 release -- consume 。同步还是一样的同步，这回副作用弱了点：
在线程 B acquire x 之后的读操作中，有一些是依赖于 x 的值的读操作。
管这些依赖于 x 的读操作叫 赖B读. 同理在线程 A 里面, release x 也有一些它所依赖的其他写操作，这些写操作自然发生在 release x 之前了。管这些写操作叫 赖A写. 
现在这个副作用就是，只有 赖B读 能看见 赖A写. （卧槽真累）有人问了，说什么叫数据依赖（carries dependency）？其实这玩意儿巨简单：S1. c = a + b;
S2. e = c + d;S2 数据依赖于 S1，因为它需要 c 的值。

4. Sequential consistency: 
理解了前面的几个，顺序一致性就最好理解了。Release -- acquire 就同步一个 x，
顺序一致就是对所有的变量的所有原子操作都同步。
这么一来，我擦，所有的原子操作就跟由一个线程顺序执行似的。
```

## 内存序类型表格

| 内存序 | 中文名称 | 描述 |
|--------|----------|------|
| memory_order_relaxed | 松散顺序 | 最弱的顺序保证，仅保证原子性 |
| memory_order_consume | 消费顺序 | 数据依赖顺序（很少用） |
| memory_order_acquire | 获取顺序 | 读操作，防止后续操作重排到它前面 |
| memory_order_release | 释放顺序 | 写操作，防止前面操作重排到它后面 |
| memory_order_acq_rel | 获取-释放顺序 | 同时具有 acquire 和 release 语义 |
| memory_order_seq_cst | 顺序一致性 | 最强的顺序保证，默认选项 |

## 注意
```cpp
    memory_order_consume memory_order_acquire memory_order_acq_rel:这三种内存序不能用于store操作
    load操作是六种都可以
```
## 内存序和内存屏障的关系
```
C++ memory_order（语言层）
       ↓
[编译器根据目标架构决定]
       ↓
内存屏障指令（硬件层）
       ↓
CPU 执行正确的内存顺序
```