perf list 是 Linux 性能分析工具 perf 的一个子命令，用于列出当前系统支持的所有性能事件（performance events）。这些事件是 perf 进行性能分析的基础，你可以基于这些事件来监控和分析系统的 CPU、内存、缓存、分支预测、内核行为等。

📋 perf list 的基本用法
bash
编辑
perf list
这会列出所有可用的性能事件，包括硬件事件、软件事件、Tracepoint 事件等。

🔍 按类别过滤事件
你可以使用参数来只显示特定类型的事件：

命令	说明
perf list hw	列出硬件事件（Hardware Events）
perf list sw	列出软件事件（Software Events）
perf list cache	列出缓存事件（Cache Events）
perf list tracepoint	列出内核 tracepoint 事件（数量通常很多）
perf list pmu	列出PMU（Performance Monitoring Unit）事件
perf list <event_glob>	使用通配符搜索事件，例如 perf list *fault*
🧩 性能事件的三大类
1. Hardware Events（硬件事件）
由 CPU 的性能监控单元（PMU）直接支持，精度高。

常见事件：

cpu-cycles 或 cycles：CPU 周期
instructions：执行的指令数
cache-references：缓存引用
cache-misses：缓存未命中
branch-instructions：分支指令数
branch-misses：分支预测错误
bus-cycles：总线周期
💡 这些事件依赖于 CPU 架构（如 Intel、AMD、ARM）。

2. Software Events（软件事件）
由内核模拟或统计，与硬件无关。

常见事件：

task-clock：任务运行时间（ms）
context-switches：上下文切换次数
cpu-migrations：CPU 迁移次数（进程被迁移到其他 CPU）
page-faults：缺页异常
minor-faults / major-faults：轻微/严重缺页
alignment-faults：对齐错误
emulation-faults：模拟错误
dummy：空事件，用于测试
3. Tracepoint Events（跟踪点事件）
基于内核的 ftrace 机制，用于跟踪内核函数调用、系统调用、调度行为等。

示例：

sched:sched_switch：进程调度切换
syscalls:sys_enter_openat：进入 openat 系统调用
irq:irq_handler_entry：中断处理入口
kmem:kmalloc：内核内存分配
⚠️ 数量庞大（通常数百到上千个），建议用 perf list 'syscalls:*' 这样的方式过滤。

📌 使用示例
bash
编辑
# 列出所有事件
perf list

# 只看缓存相关的事件
perf list cache

# 查看与“fault”相关的事件（如缺页）
perf list *fault*

# 查看系统调用相关的 tracepoint
perf list 'syscalls:*'

# 查看分支预测相关的硬件事件
perf list branch*
🛠️ 实际应用场景
分析 CPU 利用率低但程序慢：
bash
编辑
perf stat -e cache-misses,cycles,instructions ./your_program
如果 cache-misses 很高，说明存在内存访问瓶颈。
分析上下文切换开销：
bash
编辑
perf stat -e context-switches,cpu-migrations ./your_program
分析系统调用开销：
bash
编辑
perf stat -e syscalls:sys_enter_write ./your_program
实时查看热点函数：
bash
编辑
perf top -e cycles:u  # 用户态 CPU 周期
perf top -e cycles:k  # 内核态 CPU 周期
⚠️ 注意事项
需要 root 权限 或 /proc/sys/kernel/perf_event_paranoid 设置为较低值才能访问某些事件。
某些事件依赖于 CPU 型号，不同机器支持的事件可能不同。
tracepoint 事件需要内核开启 CONFIG_FTRACE 和 CONFIG_PERF_EVENTS。
✅ 总结
perf list 是使用 perf 工具的第一步，它让你知道“可以测量什么”。

通过它，你可以选择合适的性能事件来定位程序的性能瓶颈，例如：

是 CPU 计算密集？ → 用 cycles, instructions
是 内存访问慢？ → 用 cache-misses, mem-loads
是 系统调用多？ → 用 syscalls:*
是 上下文切换频繁？ → 用 context-switches
掌握 perf list，你就掌握了性能分析的“武器库”清单。

```bash
 perf stat -e cache-references,cache-misses,cpu-clock,cycles,instructions,context-switches,L1-dcache-loads,L1-dcache-load-misses,L1-icache-loads,L1-icache-load-misses,dTLB-loads,dTLB-load-misses,iTLB-loads,iTLB-load-misses 
 ```