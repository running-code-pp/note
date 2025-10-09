# 简介
程序信号处理函数:c++中在<csignal.h> c中<signal.h>
当进程接收到信号之后就会**立马中断**转头去执行注册的信号处理函数，结束之后返回被中断的地方

# 使用
signal(int,void(*func)(int));
```
void crashCallBack(int sigNum)
{
    printf("crash signal triggered!\n");
}

signal(SIGSEGV,crashCallBack);
```


# 信号

| 宏      | 英文全称                        | 说明                                                         |
|---------|----------------------------------|--------------------------------------------------------------|
| SIGABRT | Signal Abort                     | 程序异常终止                                                |
| SIGFPE  | Signal Floating-Point Exception  | 算术运算出错，如除数为0或溢出（不一定是浮点数运算）         |
| SIGILL  | Signal Illegal Instruction       | 非法指令                                                    |
| SIGSEGV | Signal Segmentation Violation    | 非法访问存储器，如访问不存在的内存单元                      |
| SIGTERM | Signal Terminate                 | 发送给本程序的终止请求信号                                  |

# 预定义信号处理函数

| 宏       | 说明               |
|----------|--------------------|
| SIG_DFL  | 默认信号处理函数   |
| SIG_IGN  | 忽视信号           |