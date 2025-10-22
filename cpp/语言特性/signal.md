# 简介
程序信号处理函数:c++中在<csignal.h> c中<signal.h>
当进程接收到信号之后就会**立马中断**转头去执行注册的信号处理函数，结束之后返回被中断的地方，signal是一个全局函数是进程级别的和在哪个线程注册信号没有关系

# 使用
signal(int,void(*func)(int));
```
void crashCallBack(int sigNum)
{
    printf("crash signal triggered!\n");
}

signal(SIGSEGV,crashCallBack);
```

# 同步信号和异步信号
- 同步信号：触发信号的线程处理
- 异步信号：OS任意选择一个未被阻塞的线程处理
- 

# 标准库支持的信号

| 宏      | 英文全称                        | 说明                                                         | 类型   |
|---------|----------------------------------|--------------------------------------------------------------|--------|
| SIGABRT | Signal Abort                     | 程序异常终止                                                | 同步   |
| SIGFPE  | Signal Floating-Point Exception  | 算术运算出错，如除数为0或溢出（不一定是浮点数运算）         | 同步   |
| SIGILL  | Signal Illegal Instruction       | 非法指令                                                    | 同步   |
| SIGSEGV | Signal Segmentation Violation    | 非法访问存储器，如访问不存在的内存单元                      | 同步   |
| SIGTERM | Signal Terminate                 | 发送给本程序的终止请求信号                                  | 异步   |
| SIGINT  | Signal Interrupt                 | Ctrl+C 信号，中断程序执行                                   | 异步   |

# 预定义信号处理函数

| 宏       | 说明               |
|----------|--------------------|
| SIG_DFL  | 默认信号处理函数   |
| SIG_IGN  | 忽视信号           |

# 信号的执行机制

- 1、内核给进程所在的进程表项的信号域设置该信号位以向进程发送软中断信号
- 2、如果进程正在睡眠，那么要判断睡眠的优先级，如果睡眠在可被中断的优先级上则唤醒
- 3、进程接受到信号的时机是从内核态切换回用户态时，所以信号不会立即起作用
- 4、在用户态执行信号处理函数

