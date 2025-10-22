<!--
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-12 16:14:48
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-13 00:49:27
 * @FilePath: \note\cpp\asio(nonboost)\asio接口文档.md
 * @Description: 
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
-->
# io_context
```
asio的核心调度组件，本质是一个eventloop,负责管理所有异步操作的生命周期，从注册，等待事件触发，到最终调用

- 事件源：异步操作（如socket读写、定时器到期）的完成信号，由操作系统内核通过I/O多路复用机制（如Linux的epoll、Windows的IOCP、BSD的kqueue）通知io_context。

- 任务队列：io_context内部维护一个“完成处理程序队列”，当接收到内核的事件通知后，会将对应的回调函数（完成处理程序）加入队列，等待run()方法执行。

- 线程关联：io_context本身不绑定线程，需通过调用run()、run_one()、poll()等方法，让线程进入事件循环，处理队列中的任务。

```

| 方法 | 功能描述 | 线程安全？ |
|------|----------|------------|
| `run()` | 阻塞线程，持续处理队列中的完成处理程序，直到所有异步操作完成且无"工作守护"（见下文）。 | 否（同一io_context可被多线程调用run()，但单线程内不可重入） |
| `run_one()` | 阻塞线程，处理一个完成处理程序后返回，返回值为"是否处理了任务"（size_t）。 |  否 |
| `poll()` | 非阻塞，一次性处理当前队列中所有就绪的完成处理程序，立即返回处理的任务数。 |  否 |
| `poll_one()` | 非阻塞，处理一个就绪的完成处理程序（若存在），返回值为"是否处理了任务"。 | 否  |
| `stop()` | 立即终止事件循环，未处理的任务会被标记为"已取消"，后续run()调用会直接返回。 | 是 |
| `restart()` | 重置io_context的"停止状态"，使其可再次调用run()（适用于stop()后重启的场景）。 | 否 |
| `post(F&& f)` | 向io_context提交一个任务f（立即加入队列，不依赖外部事件） | 是 |
| `dispatch(F&& f)` | 若当前线程正在执行io_context的事件循环，则直接执行f；否则等同于post | 是 |


## 注意事项
`1、io_context中如果没有异步操作那么run()会立马返回，为了避免这种情况，需要一个守护对象asio::io_context::work，强制run()保持阻塞直到被销毁,io_context本身不是线程安全的（除表中标注的函数外），例如不能在一个线程中修改io_context的配置，同时在另一个线程中调用run()。`

```cpp
#include <asio.hpp>
#include <thread>

int main() {
  asio::io_context io;
  // 创建工作守护，防止run()立即返回
  asio::io_context::work work(io);

  // 启动线程执行事件循环
  std::thread t([&io]() { io.run(); });

  // 模拟业务逻辑（此处可添加异步操作）
  std::this_thread::sleep_for(std::chrono::seconds(3));

  // 销毁work（或调用io.stop()），让run()退出
  work.~work(); // 显式销毁（或通过智能指针管理生命周期）
  t.join();

  return 0;
}
```
`2、io_context的run()方法支持多线程调用，多个线程可以同时进入事件循环共同处理任务队列，asio会保证任务分发的线程安全`


# Completion Handler：异步操作结果处理器
就是异步操作完成之后执行的回调

`标准`
- 参数列表： 至少包含asio::error_code接受操作结果，如果涉及数据读写那么还要有std::size_t表示读写成功的字节数
- 返回值: void
- 可复制/移动: 需要支持复制或移动语义

`支持的形式`

- 普通函数： 适用于无状态依赖
- lambda表达式: 适用于逻辑内聚，需要捕获上下文，最常见
- 函数对象: 适用于逻辑复杂，需要维护状态

`线程安全`
- asio保证一个Completion Handler不会被多个线程同时执行
- Completion Handler的生命周期由asio管理，直到被执行或者操作取消，如果异步操作被取消，对应的完成处理程序仍然会被调用，但是error_code被设置为asio::error_code::operation_aborted
  
`异步安全`
异步操作发起之后，其以来的资源如缓冲去，系统资源的生命周期必须长于异步操作的生命周期，否则会造成非法访问导致崩溃

`异步操作之间的执行顺序`
有的时候我们需要规定几个异步操作的执行顺序，
比如 connect->read->write->disconnect,
为了保证顺序我们可以在上一个异步操作末尾调用下一个异步操作

# IO对象
asio对于os底层IO资源的封装，例如套接字，定时器，串口，从而提供统一的同步/异步接口

| I/O对象类型 | 功能描述 | 核心方法示例 |
|-------------|----------|-------------|
| `asio::steady_timer` | 基于单调时钟的定时器（不受系统时间调整影响），用于定时触发操作。 | `expires_after()`（设置过期时间）、`async_wait()`（异步等待） |
| `asio::ip::tcp::socket` | TCP协议套接字，用于面向连接的可靠数据传输。 | `connect()`/`async_connect()`、`read_some()`/`async_read_some()` |
| `asio::ip::udp::socket` | UDP协议套接字，用于无连接的不可靠数据传输（适用于广播、低延迟场景）。 | `send_to()`/`async_send_to()`、`receive_from()`/`async_receive_from()` |
| `asio::serial_port` | 串行端口对象，用于与串口设备（如传感器、嵌入式设备）通信。 | `open()`（打开端口）、`write_some()`/`async_write_some()` |
| `asio::ip::tcp::acceptor` | TCP服务器端 acceptor，用于监听并接受客户端连接请求。 | `bind()`（绑定地址端口）、`listen()`（开始监听）、`async_accept()`（异步接受） |


# asio::error_code
错误码，实际上就是std::error_code

**`核心类型`**
- asio::error_code：用于存储错误状态，包含错误值（value()）和错误类别（category()）。
- asio::system_error：可选的异常类型，可将error_code转换为异常（适用于需要异常处理的场景）。

**`错误码来源`**
- 操作系统错误：如“连接被拒绝”（asio::error::connection_refused）、“端口已占用”（asio::error::address_in_use）。
- Asio库错误：如“操作被取消”（asio::error::operation_aborted）、“缓冲区不足”（asio::error::message_size）。
- 用户自定义错误：通过扩展asio::error_category可定义应用特定的错误码。

# 缓冲区
Asio的缓冲区组件设计用于高效管理数据传输，避免不必要的内存复制，同时确保异步操作中数据的安全性。

`**核心设计原则**`
- 非所有权：缓冲区不管理数据的生命周期，仅提供对已有内存的“视图”（类似C++20的std::span）。
- 类型安全：区分“可修改缓冲区”（mutable_buffer）和“只读缓冲区”（const_buffer），防止意外修改常量数据。
- 拼接能力：支持将多个缓冲区拼接为一个“缓冲区序列”，适用于分散-聚集（scatter-gather）I/O操作。

`**主要缓冲区类型**`

| 缓冲区类型 | 功能描述 | 适用场景 |
|------------|----------|----------|
| `asio::const_buffer` | 只读缓冲区，包装const void*和大小，用于读取操作或发送常量数据。 | `async_send()`、`async_write()`发送常量数据 |
| `asio::mutable_buffer` | 可修改缓冲区，包装void*和大小，用于写入操作或接收数据。 | `async_receive()`、`async_read()`接收数据 |
| `asio::buffer()` | 通用工厂函数，根据输入数据自动创建const_buffer或mutable_buffer。 | 简化缓冲区创建，自动推断类型 |
| `asio::streambuf` | 基于流的缓冲区，类似std::stringstream，支持流式读写和动态内存管理。 | 处理变长数据（如HTTP报文、协议帧） |
| `asio::dynamic_buffer` | 适配器，将std::string或std::vector<char>包装为可动态扩展的缓冲区。 | 需自动扩容的场景（如读取未知长度的数据） |


# endpoint，网络地址的标准化表示

目标地址+端口,aiso为了频闭底层地址结构的差异(ipv4/v6)，
提供了endpoint抽象

**`核心类型`**
- asio::ip::tcp::endpoint: tcp协议栈 端点
- asio::ip::udp::endpoint: udp协议栈 端点
- asio::ip::address: ip地址封装支持v4/v6

```cpp
// 创建IPv4 TCP端点（127.0.0.1:8080）
asio::ip::tcp::endpoint endpoint_v4(
  asio::ip::make_address_v4("127.0.0.1"), 8080);

// 创建IPv6 TCP端点（::1:8080）
asio::ip::tcp::endpoint endpoint_v6(
  asio::ip::make_address_v6("::1"), 8080);

// 从端点获取地址和端口
std::cout << "Address: " << endpoint_v4.address() << std::endl; // 127.0.0.1
std::cout << "Port: " << endpoint_v4.port() << std::endl;       // 8080

// 检查地址类型
if (endpoint_v4.address().is_v4()) {
  std::cout << "IPv4 address" << std::endl;
}

```

# Resolvers 解析器
将域名解析为端点列表(ip+port)

- asio::ip::tcp::resolver: tcp协议解析器，返回asio::ip::tcp::endpoint
- asio::ip::udp::resolver: udp协议解析器，返回asio::ip::udp::endpoint

```cpp
void connectToServer(io_context& ioc, const std::string& host, const std::string& service)
{
  LOG_INFO("Starting DNS resolution for {}:{}", host, service);
  
  // 使用shared_ptr管理resolver，防止在异步操作完成前被销毁
  auto resolver = std::make_shared<asio::ip::tcp::resolver>(ioc);
  resolver->async_resolve(
      host,
      service,
      [resolver, &ioc](const asio::error_code& ec, asio::ip::tcp::resolver::results_type results)
      {
        if (!ec)
        {
          LOG_INFO("resolve success, found {} endpoints", results.size());
          for (const auto& entry : results)
          {
            LOG_INFO("endpoint: {}:{}", entry.endpoint().address().to_string(), entry.endpoint().port());
            
            // 使用shared_ptr管理socket，防止在异步操作完成前被销毁
            auto socket = std::make_shared<tcp_socket>(ioc);
            socket->async_connect(
                entry.endpoint(),
                [socket](const asio::error_code& ec)
                {
                  if (!ec)
                  {
                    LOG_INFO("connect to server success");
                    // 断开连接
                    socket->close();
                    LOG_INFO("shutdown socket success");
                  }
                  else
                  {
                    LOG_ERR("connect to server failed: {}", ec.message());
                  }
                });
            
            // 只连接第一个可用的endpoint
            break;
          }
        }
        else
        {
          LOG_ERR("resolve failed: {}", ec.message());
        }
      });
}
```


# execution contexts 执行上下文
io_context是最基础的执行上下文，asio提供了executor式的执行器

- asio::io_context: 基础执行上下文，基于eventloop,适用于IO密集型
- asio::thread_pool: 线程池执行上下文，适用于cpu密集型任务与IO任务混合的场景
- asio::system_executor: 系统默认执行器，直接在当前线程执行，没有调度

`asio::thread_pool`
```cpp
int main()
{
  // 初始化日志配置并保持其生命周期
  common::LoggerConfig config;
  common::loadLogConfig(config, "log.yaml");
  auto logger = common::create_logger();
  logger->Init(config);
  auto pool = std::make_shared<asio::thread_pool>(std::thread::hardware_concurrency() << 1);
  // 提交最简单的任务到线程池
  asio::post(
      *pool,
      []()
      {
        LOG_INFO("task running on thread {}", thread_id_str()); 
        std::this_thread::sleep_for(std::chrono::seconds(1));
      });
    // 提交带参数的任务到线程池,注意asio::post 不直接支持带参数的任务，如果需要的话可以采用lambda+捕获列表原地初始化的方式
  asio::post(
      *pool,
      [a = 42, c = 'x'](){ 
        LOG_INFO("task with params: a={}, c={}, thread={}", a, c, thread_id_str()); 
      });
  logger->ShutDown();
  // 等待所有任务结束关闭线程池
  pool->stop();
  return 0;
}
```

# 多线程和并发控制
## 多线程运行io_context
在多个线程调用io_context::run()
那么就会有多个线程去io_context的完成队列中取回调闭包执行

`优点`
```text
负载均衡: 多个线程同时处理任务队列
容错性: 单线程崩溃或者阻塞不影响其他线程
```

## Strand 异步操作的“序列化执行器”

Strand("执行链"),asio提供的一种轻量级同步机制，确保一组异步操作的完成回调按照顺序执行，避免竞态条件

Strand并不会创建一个单独的线程，他本质是执行器的包装，通过任务队列串行执行完成回调

不同的Strand管理的回调可以并行执行、

**`strand两种使用方式`**
- asio::io_context::strand绑定到io_context,适用于与IO操作相关的场景
- asio::strand<Executor>: 通过strand，绑定到任意执行器，例如thread_pool

`应用场景`
当多个异步操作需要访问同一个资源的时候比如计数器可以用Strand替代锁，减小开销

`示例`
```cpp
int global_counter = 0;

int main()
{
  // 注册信号
  std::signal(SIGINT, signal_handler);
  // 初始化日志配置并保持其生命周期
  common::LoggerConfig config;
  common::loadLogConfig(config, "log.yaml");
  auto logger = common::create_logger();
  logger->Init(config);
  auto iocPtr = std::make_shared<asio::io_context>();
  // 防止ioc立即退出，增加一个守护任务
  g_iocPtr = iocPtr;
  strand strand_(*iocPtr);
  for (int i = 0; i < 50; ++i)
  {
    asio::post(strand_,[i](){  // 捕获局部变量 i
    ++global_counter;  // 全局变量直接访问，无需捕获
    LOG_INFO("task {} incremented global_counter to {} on thread-{}",i,global_counter,thread_id_str());
    });
  }
  iocPtr->run();
  LOG_INFO("all task finished global_counter:{}",global_counter);
  logger->ShutDown();
  return 0;
}

```

# 所有权与生命周期管理

asio的核心在于异步，而异步伴随着携带的资源的生命周期/所有权的管理

## 共享所有权模式
通过std::shared_ptr,std::endable_shared_from_this实现对象的共享所有权，将对象的生命周期和异步操作绑定在一起

用法: 将异步操作需要访问的外部资源必须是std::shared_ptr<T>管理的动态资源，然后 通过shared_from_this()将所有权和异步操作共享

例如下面管理tcp会话的伪代码
```cpp
class Session:public std::enable_shared_from_this<Session>{
public:
void start(){
    do_read();
}
private:
void do_read(){
  auto self(shared_from_this());
      // 共享Session的所有权给async_read_some这个异步任务
  _socket.async_read_some(asio::dynamic_buffer,[self](error_code,bytes){
    .....
        // 紧接着执行do_read 又将所有权共享给do_write了
    do_write(bytes);
  }
}
void do_write(){
    auto self(shared_from_this());
    // 共享Session的所有权给async_write这个异步任务
  _socket.async_write(asio::dynamic_buffer,[self](error_code,bytes){
    .....
    // 紧接着执行do_read 又将所有权共享给do_read了
    do_read();
  }
}
private:
tcp_socket _socket;
std::vector<char> _buffer;
}
```

**`注意事项`**
防止循环引用


# 定时器

asio 提供了多种定时器类型，分别适用于不同的时间源和精度需求：

| 定时器类型 | 说明 | 适用场景 |
|------------|------|----------|
| `asio::steady_timer` | 基于单调时钟，不受系统时间调整影响 | 精确的时间间隔、超时控制 |
| `asio::system_timer` | 基于系统时钟（wall clock），受系统时间调整影响 | 需要与真实时间同步的场景，如定点任务 |
| `asio::high_resolution_timer` | 基于高精度时钟，提供纳秒级精度 | 需要极高时间精度的场景 |
| `asio::deadline_timer` | 兼容Boost.Asio的旧接口，基于系统时钟 | 兼容老代码 |

---

## system_timer 系统时钟定时器
与系统时间相关，受系统时间调整影响，适合定点任务。

```cpp
#include <asio.hpp>
#include <iostream>
#include <chrono>

int main() {
  asio::io_context ioc;
  asio::system_timer timer(ioc);
  timer.expires_after(std::chrono::seconds(2));
  timer.async_wait([](const asio::error_code& ec) {
    if (!ec) std::cout << "system_timer expired after 2 seconds\n";
  });
  ioc.run();
  return 0;
}
```

## high_resolution_timer 高精度定时器
提供纳秒级精度，适合对时间精度要求极高的场景。

```cpp
#include <asio.hpp>
#include <iostream>
#include <chrono>

int main() {
  asio::io_context ioc;
  asio::high_resolution_timer timer(ioc);
  timer.expires_after(std::chrono::nanoseconds(1000000)); // 1毫秒
  timer.async_wait([](const asio::error_code& ec) {
    if (!ec) std::cout << "high_resolution_timer expired after 1ms\n";
  });
  ioc.run();
  return 0;
}
```

## deadline_timer 兼容旧接口的定时器
主要用于兼容 Boost.Asio 旧代码，基于系统时钟。

```cpp
#include <boost/asio.hpp>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

int main() {
  boost::asio::io_context ioc;
  boost::asio::deadline_timer timer(ioc, boost::posix_time::seconds(2));
  timer.async_wait([](const boost::system::error_code& ec) {
    if (!ec) std::cout << "deadline_timer expired after 2 seconds\n";
  });
  ioc.run();
  return 0;
}
```

## steady_timer 稳定时钟计时器
和系统时间无关，只关心严格的时间间隔

`使用`
```cpp
#include <asio.hpp>
#include <iostream>
#include <chrono>

int main() {
    asio::io_context ioc;
    
    // 1. 基本异步等待
    asio::steady_timer timer1(ioc);
    timer1.expires_after(std::chrono::seconds(2));
    timer1.async_wait([](const asio::error_code& ec) {
        if (!ec) {
            std::cout << "Timer1 expired after 2 seconds\n";
        }
    });
    
    // 2. 重复定时器（在回调中重新设置）
    auto timer2 = std::make_shared<asio::steady_timer>(ioc);
    std::function<void()> repeat_timer;
    repeat_timer = [timer2, &repeat_timer]() {
        timer2->expires_after(std::chrono::milliseconds(500));
        timer2->async_wait([&repeat_timer](const asio::error_code& ec) {
            if (!ec) {
                std::cout << "Repeat timer tick\n";
                repeat_timer(); // 递归调用实现重复
            }
        });
    };
    repeat_timer(); // 启动重复定时器
    
    // 3. 可取消的定时器
    asio::steady_timer timer3(ioc);
    timer3.expires_after(std::chrono::seconds(5));
    timer3.async_wait([](const asio::error_code& ec) {
        if (ec == asio::error::operation_aborted) {
            std::cout << "Timer3 was cancelled\n";
        } else if (!ec) {
            std::cout << "Timer3 expired\n";
        }
    });
    
    // 1秒后取消timer3
    asio::steady_timer cancel_timer(ioc);
    cancel_timer.expires_after(std::chrono::seconds(1));
    cancel_timer.async_wait([&timer3](const asio::error_code& ec) {
        if (!ec) {
            timer3.cancel(); // 取消timer3
            std::cout << "Cancelled timer3\n";
        }
    });
    
    // 4. 同步等待（阻塞）
    std::thread sync_thread([&ioc]() {
        asio::steady_timer sync_timer(ioc);
        sync_timer.expires_after(std::chrono::seconds(1));
        sync_timer.wait(); // 同步等待
        std::cout << "Sync timer expired\n";
    });
    
    // 运行事件循环3秒后停止
    asio::steady_timer stop_timer(ioc);
    stop_timer.expires_after(std::chrono::seconds(3));
    stop_timer.async_wait([&ioc](const asio::error_code& ec) {
        if (!ec) {
            std::cout << "Stopping io_context\n";
            ioc.stop();
        }
    });
    
    ioc.run();
    sync_thread.join();
    
    return 0;
}
```
