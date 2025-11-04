<!--
 * @Author: running-code-pp
 * @Date: 2025-10-29 16:00:43
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-11-02 22:01:34
 * @FilePath: \note\工程实践\进程cpu占用过高.md
 * @Description: 
 * @Copyright: Copyright (c) 2025 by running-code-pp 3320996652@qq.com, All Rights Reserved. 
-->
# 如果linux中一个服务进程突然cpu很高，怎么排查问题
top 查看cpu占用的情况，找出造成cpu占用过高的进程，并记录pid

top -H -p pid 查看进程中所有线程的cpu资源使用情况

top - 15:59:42 up 3 days,  2:23,  1 user,  load average: 0.00, 0.00, 0.00
Threads:  12 total,   0 running,  12 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.3 us,  0.2 sy,  0.0 ni, 99.2 id,  0.0 wa,  0.2 hi,  0.2 si,  0.0 st
MiB Mem :   1735.7 total,    359.6 free,    300.8 used,   1075.3 buff/cache
MiB Swap:      0.0 total,      0.0 free,      0.0 used.   1250.4 avail Mem 

    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND                                                     
   1064 root      20   0  126776  22928  12464 S   0.3   1.3   4:41.62 AliYunDun                                                   
   1069 root      20   0  126776  22928  12464 S   0.3   1.3   4:48.75 AliYunDun                                                   
   1060 root      20   0  126776  22928  12464 S   0.0   1.3   0:50.60 AliYunDun                                                   
   1061 root      20   0  126776  22928  12464 S   0.0   1.3   0:22.55 AliYunDun                                                   
   1062 root      20   0  126776  22928  12464 S   0.0   1.3   0:23.82 AliYunDun                                                   
   1063 root      20   0  126776  22928  12464 S   0.0   1.3   1:04.08 AliYunDun                                                   
   1065 root      20   0  126776  22928  12464 S   0.0   1.3   0:05.71 AliYunDun                                                   
   1066 root      20   0  126776  22928  12464 S   0.0   1.3   0:03.59 AliYunDun                                                   
   1067 root      20   0  126776  22928  12464 S   0.0   1.3   1:29.65 AliYunDun                                                   
   1068 root      20   0  126776  22928  12464 S   0.0   1.3   0:00.84 AliYunDun                                                   
   1070 root      20   0  126776  22928  12464 S   0.0   1.3   0:04.87 AliYunDun                                                   
   1071 root      20   0  126776  22928  12464 S   0.0   1.3   0:02.40 AliYunDun                                                   


top 命令显示所有的进程的cpu占用

shift+h 切换到线程cpu视图,但是这里没有显示线程id
所以要先找出pid

然后查看线程的堆栈

# 方法1：使用 pstack（简单快速）
pstack <进程PID>

# 方法2：使用 gdb（功能强大，可交互）
gdb -p <进程PID> -batch -ex "thread apply all bt"

# 方法3：查看 /proc（无需额外工具）
cat /proc/<进程PID>/task/*/stack