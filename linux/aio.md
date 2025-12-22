# libaio简介
linux原生支持的异步io，类似于windows中的iocp，向内核的io时间队列中投递任务，然后从完成队列中中获取结果。和windows iocp的主要区别在于，aio只能支持存储文件的IO，不支持网络IO。
```bash
安装命令
# Ubuntu/Debian
sudo apt-get install libaio-dev

# CentOS/RHEL
sudo yum install libaio-devel
```

## api

#### io_context_t (上下文类型)

libaio 的上下文句柄，用于管理异步 I/O 操作。 属于linux的内核数据结构，没有对外暴露定义

```cpp
typedef struct io_context *io_context_t;
```

#### io_iocb_cmd (操作码枚举)

定义了 libaio 支持的 I/O 操作类型。

```cpp
typedef enum io_iocb_cmd {
    IO_CMD_PREAD = 0,    // 异步读操作
    IO_CMD_PWRITE = 1,   // 异步写操作
    IO_CMD_FSYNC = 2,    // 同步文件数据和元数据到磁盘
    IO_CMD_FDSYNC = 3,   // 同步文件数据到磁盘（不含元数据）
    IO_CMD_POLL = 5,     // 轮询操作
    IO_CMD_NOOP = 6,     // 空操作
    IO_CMD_PREADV = 7,   // 向量化异步读操作
    IO_CMD_PWRITEV = 8,  // 向量化异步写操作
} io_iocb_cmd_t;
```

#### iocb (I/O 控制块)

libaio 的核心数据结构，描述一个异步 I/O 请求。

```cpp
struct iocb {
    void *data;              // 用户自定义数据，在完成事件中返回
    unsigned key;            // 用于识别 I/O 请求的键
    unsigned aio_rw_flags;   // RWF_* 标志（这些值定义在linux/fs.h中
    short aio_lio_opcode;    // 操作类型（IO_CMD_PREAD/IO_CMD_PWRITE 等）
    short aio_reqprio;       // 请求优先级
    int aio_fildes;          // 文件描述符
    union {
        struct io_iocb_common c;     // 通用 I/O 操作
        struct io_iocb_vector v;     // 向量化 I/O 操作
        struct io_iocb_poll poll;    // 轮询操作
        struct io_iocb_sockaddr saddr; // socket 地址操作
    } u;
};
```

aio_rw_flags的可用值

| 标志（Flag） | 值（十六进制） | 内核版本 | 作用说明                                                                                                                                                 |
| ------------ | -------------- | -------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- |
| RWF_HIPRI    | 0x00000001     | ≥ 4.7    | 高优先级 I/O。请求高优先级处理（通常用于实时或延迟敏感场景）。需要 CAP_SYS_ADMIN 权限，且设备/文件系统需支持（如 NVMe）。                                |
| RWF_DSYNC    | 0x00000002     | ≥ 4.13   | 等效于 O_DSYNC。确保数据和必要元数据写入存储后才完成写操作。效果同 fdatasync()。                                                                         |
| RWF_SYNC     | 0x00000004     | ≥ 4.13   | 等效于 O_SYNC。确保数据和所有元数据（包括非必要）写入存储后才完成。效果同 fsync()。                                                                      |
| RWF_NOWAIT   | 0x00000008     | ≥ 5.1    | 非阻塞 I/O（不等待）。如果 I/O 会阻塞（如 page fault、disk wait），立即返回 -EAGAIN。要求文件以 O_DIRECT 打开，且支持异步 page-in（如 XFS/ext4 + DAX）。 |
| RWF_APPEND   | 0x00000010     | ≥ 5.6    | 追加写模式。自动将写入位置设为文件末尾（忽略 offset 字段）。类似 O_APPEND，但用于单次写操作。                                                            |



#### io_iocb_common (通用 I/O 参数)

用于普通读写操作的参数结构。

```cpp
struct io_iocb_common {
    void *buf;           // 数据缓冲区指针
    unsigned long nbytes; // 读写字节数
    long long offset;    // 文件偏移量
    unsigned flags;      // 标志位
    unsigned resfd;      // eventfd 文件描述符（用于完成通知）
};
```

#### io_iocb_vector (向量化 I/O 参数)

用于向量化（scatter/gather）I/O 操作。

```cpp
struct io_iocb_vector {
    const struct iovec *vec; // iovec 数组指针
    unsigned long nr;        // iovec 数组元素个数
    long long offset;        // 文件偏移量
};
```

#### io_event (完成事件)

从完成队列中获取的事件结构，表示一个已完成的 I/O 操作。

```cpp
struct io_event {
    void *data;          // 来自 iocb 的用户数据
    struct iocb *obj;    // 指向原始 iocb 的指针
    unsigned long res;   // I/O 操作结果（成功时为字节数，失败时为负的 errno）
    unsigned long res2;  // 辅助结果（通常为 0）
};
```

#### io_callback_t (回调函数类型)

```cpp
typedef void (*io_callback_t)(io_context_t ctx, struct iocb *iocb, long res, long res2);
```

### 系统调用

#### io_setup / io_queue_init

初始化异步 I/O 上下文。

```cpp
/* 系统调用版本 */
extern int io_setup(int maxevents, io_context_t *ctxp);

/* 库封装版本 */
extern int io_queue_init(int maxevents, io_context_t *ctxp);
```
- `maxevents`: 最大并发事件数
- `ctxp`: 输出参数，返回创建的上下文
- 返回值: 成功返回 0，失败返回负的 errno

#### io_destroy / io_queue_release

销毁异步 I/O 上下文，释放相关资源。

```cpp
extern int io_destroy(io_context_t ctx);
extern int io_queue_release(io_context_t ctx);
```

#### io_submit

提交一组异步 I/O 请求。

```cpp
extern int io_submit(io_context_t ctx, long nr, struct iocb *ios[]);
```
- `ctx`: I/O 上下文
- `nr`: 提交的请求数量
- `ios`: iocb 指针数组
- 返回值: 成功提交的请求数，失败返回负的 errno

#### io_cancel

尝试取消一个已提交的异步 I/O 请求。

```cpp
extern int io_cancel(io_context_t ctx, struct iocb *iocb, struct io_event *evt);
```
- `ctx`: I/O 上下文
- `iocb`: 要取消的请求
- `evt`: 如果取消成功，存放取消事件
- 返回值: 成功返回 0，失败返回负的 errno

#### io_getevents

从完成队列获取已完成的事件。

```cpp
extern int io_getevents(io_context_t ctx, long min_nr, long nr,
                        struct io_event *events, struct timespec *timeout);
```
- `ctx`: I/O 上下文
- `min_nr`: 最少等待的事件数
- `nr`: 最多获取的事件数
- `events`: 存放完成事件的数组
- `timeout`: 超时时间（NULL 表示无限等待）
- 返回值: 获取到的事件数，失败返回负的 errno

#### io_pgetevents

带信号掩码的 io_getevents 版本。

```cpp
extern int io_pgetevents(io_context_t ctx, long min_nr, long nr,
                         struct io_event *events, struct timespec *timeout,
                         sigset_t *sigmask);
```

#### io_queue_run

处理完成的事件并调用相应的回调函数。

```cpp
extern int io_queue_run(io_context_t ctx);
```

### 辅助函数

#### io_set_callback

设置 iocb 的完成回调函数。

```cpp
static inline void io_set_callback(struct iocb *iocb, io_callback_t cb);
```

#### io_prep_pread / io_prep_pwrite

准备异步读/写请求。

```cpp
static inline void io_prep_pread(struct iocb *iocb, int fd, void *buf,
                                 size_t count, long long offset);

static inline void io_prep_pwrite(struct iocb *iocb, int fd, void *buf,
                                  size_t count, long long offset);
```
- `iocb`: 要初始化的 iocb 结构
- `fd`: 文件描述符
- `buf`: 数据缓冲区
- `count`: 读写字节数
- `offset`: 文件偏移量

#### io_prep_preadv / io_prep_pwritev

准备向量化异步读/写请求。

```cpp
static inline void io_prep_preadv(struct iocb *iocb, int fd,
                                  const struct iovec *iov, int iovcnt,
                                  long long offset);

static inline void io_prep_pwritev(struct iocb *iocb, int fd,
                                   const struct iovec *iov, int iovcnt,
                                   long long offset);
```

#### io_prep_preadv2 / io_prep_pwritev2

带额外标志的向量化异步读/写请求。

```cpp
static inline void io_prep_preadv2(struct iocb *iocb, int fd,
                                   const struct iovec *iov, int iovcnt,
                                   long long offset, int flags);

static inline void io_prep_pwritev2(struct iocb *iocb, int fd,
                                    const struct iovec *iov, int iovcnt,
                                    long long offset, int flags);
```

#### io_prep_fsync / io_prep_fdsync

准备同步操作请求。

```cpp
static inline void io_prep_fsync(struct iocb *iocb, int fd);  // 同步数据和元数据
static inline void io_prep_fdsync(struct iocb *iocb, int fd); // 仅同步数据
```

#### io_fsync / io_fdsync

提交同步操作请求（带回调）。

```cpp
static inline int io_fsync(io_context_t ctx, struct iocb *iocb,
                           io_callback_t cb, int fd);

static inline int io_fdsync(io_context_t ctx, struct iocb *iocb,
                            io_callback_t cb, int fd);
```

#### io_prep_poll / io_poll

准备/提交轮询操作。

```cpp
static inline void io_prep_poll(struct iocb *iocb, int fd, int events);

static inline int io_poll(io_context_t ctx, struct iocb *iocb,
                          io_callback_t cb, int fd, int events);
```

#### io_set_eventfd

设置 eventfd 用于完成通知。

```cpp
static inline void io_set_eventfd(struct iocb *iocb, int eventfd);
```
- 当 I/O 完成时，会向指定的 eventfd 写入通知
- 可配合 epoll 使用实现事件驱动

# posix aio简介
基本流程:

通过线程模拟的异步io

## api
#### aiocb (aio控制块)

aio控制块,iocb，有点类似于iocp中的windows

```cpp
struct aiocb
{
  int aio_fildes;           // 要操作的文件的文件描述符
  int aio_lio_opcode;       // 要执行的操作类型 LIO_READ LIO_WRITE
  int aio_reqprio;          // 请求的优先级偏移量，通常为0，posix允许调整调度优先级
  volatile void *aio_buf;   // 指向用户缓冲区的指针，读取的时候存放数据，写入的时候提供数据
  size_t aio_nbytes;        // 缓冲区长度
  struct sigevent aio_sigevent; // IO完成之后通知的方式: 可以指定信号，线程回调
  
  // 内部成员，外部不要修改
  struct aiocb *__next_prio;
  int __abs_prio;
  int __policy;
  int __error_code;
  __ssize_t __return_value;

  // 文件偏移量
#ifndef __USE_FILE_OFFSET64
  __off_t aio_offset;       /* File offset.  */
  char __pad[sizeof (__off64_t) - sizeof (__off_t)];
#else
  __off64_t aio_offset;     /* File offset.  */
#endif
  char __glibc_reserved[32]; // 拓展字段，不能使用
};
```

#### aioinit (GNU扩展)

```cpp
#ifdef __USE_GNU
struct aioinit
{
  int aio_threads;      /* 最大线程数。 */
  int aio_num;          /* 预期同时发起的请求数。 */
  int aio_locks;        /* 未使用。 */
  int aio_usedba;       /* 未使用。 */
  int aio_debug;        /* 未使用。 */
  int aio_numusers;     /* 未使用。 */
  int aio_idle_time;    /* 空闲线程在终止前等待的秒数。 */
  int aio_reserved;     /* 保留字段。 */
};
#endif
```

### aio_cancel() 返回值枚举

```cpp
enum
{
  AIO_CANCELED,     /* 请求已成功取消。 */
#define AIO_CANCELED AIO_CANCELED
  AIO_NOTCANCELED,  /* 请求无法取消（可能已开始执行）。 */
#define AIO_NOTCANCELED AIO_NOTCANCELED
  AIO_ALLDONE       /* 所有请求已完成，无需取消。 */
#define AIO_ALLDONE AIO_ALLDONE
};
```

### aio_lio_opcode 操作码枚举

```cpp
enum
{
  LIO_READ,   /* 列表 I/O 中表示读操作。 */
#define LIO_READ LIO_READ
  LIO_WRITE,  /* 列表 I/O 中表示写操作。 */
#define LIO_WRITE LIO_WRITE
  LIO_NOP     /* 列表 I/O 中表示空操作（不执行 I/O）。 */
#define LIO_NOP LIO_NOP
};
```

### lio_listio() 同步选项枚举

```cpp
enum
{
  LIO_WAIT,    /* 调用阻塞，直到所有 I/O 完成。 */
#define LIO_WAIT LIO_WAIT
  LIO_NOWAIT   /* 调用立即返回，I/O 在后台进行。 */
#define LIO_NOWAIT LIO_NOWAIT
};
```

### aio_init (GNU扩展)

初始化 AIO（GNU 扩展），允许用户指定 AIO 实现的优化参数（如线程数、空闲超时等）。

```cpp
#ifdef __USE_GNU
extern void aio_init (const struct aioinit *__init) __THROW __nonnull ((1));
#endif
```

### 标准 AIO 函数

标准 AIO 函数（32 位偏移量模式），当未定义 __USE_FILE_OFFSET64 时使用以下接口。

```cpp
/* 提交一个异步读请求。 */
extern int aio_read (struct aiocb *__aiocbp) __THROW __nonnull ((1));

/* 提交一个异步写请求。 */
extern int aio_write (struct aiocb *__aiocbp) __THROW __nonnull ((1));

/* 提交一组异步 I/O 请求（列表 I/O）。 */
extern int lio_listio (int __mode,
                       struct aiocb *const __list[__restrict_arr],
                       int __nent, struct sigevent *__restrict __sig)
  __THROW __nonnull ((2));

/* 获取与 aiocb 关联的错误状态（0 表示成功，非 0 为 errno 值）。 */
extern int aio_error (const struct aiocb *__aiocbp) __THROW __nonnull ((1));

/* 获取异步 I/O 操作的返回值（类似 read/write 的返回值）。 */
extern __ssize_t aio_return (struct aiocb *__aiocbp) __THROW __nonnull ((1));

/* 尝试取消针对指定文件描述符的异步 I/O 请求。 */
extern int aio_cancel (int __fildes, struct aiocb *__aiocbp) __THROW;

/* 挂起调用线程，直到列表中至少一个 AIO 操作完成。
   此函数是取消点（cancellation point），因此不标记为 __THROW。 */
extern int aio_suspend (const struct aiocb *const __list[], int __nent,
                        const struct timespec *__restrict __timeout)
  __nonnull ((1));

/* 强制将与 aio_fildes 相关的所有异步写操作同步到存储设备。 */
extern int aio_fsync (int __operation, struct aiocb *__aiocbp)
  __THROW __nonnull ((2));
```

