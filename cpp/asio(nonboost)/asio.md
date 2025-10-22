# asio简介
Asio提供了强大的工具来解决它们,并且不需要程序使用基于多线程和锁的并发模型。

Asio库为了C++系统编程而服务，特别是网络编程中它经常被用到。Asio追求以下目标：

- 跨平台性 它支持几种常见的操作系统，而且不同操作系统实现的行为相同。
- 高并发性 它应该帮助网络应用拥有处理成千上万并发事件的能力，每个操作系统都应该使用最优的高并发机制。
- 高收益性 它应该支持像scatter-gather这样的IO技术，允许程序进行尽量低粒度的文件操作。
- 从现有的优秀API中借鉴，比如BSD sockets 众所周知，BSD sockets被各种学术界借鉴，其他语言也使用了它的API风格。只要理由充分，我们就会从现有的优秀API中汲取养分。
- 易用性 该库应该是低门槛的，初学者使用工具包就可以使用，而不是框架，方法。初学者在看了一点基本说明后就能使用。使用者最后只需要理解那几个被用到的特定函数。
- 可扩展性 该库应该保证用户能够实现更高层的抽象，比如HTTP协议。
虽然Asio最初只是着重于网络，但是它的异步IO的思想已经扩展到了其他比如串口，文件句柄等领域。

# 基础api
## asio支持多范式的异步模型
- **回调:** 传统方式，如果嵌套过深容易出现回调地狱
- **Future/Promise:** 将异步操作结果包装成std::future通过get阻塞或wait_for非阻塞检查状态
- **协程:** 支持c++20无栈协程，以同步风格书写异步代码

## 支持的协议
- **TCP:** ip::tcp::socket/ip::tcp::acceptor(ipv4/ipv6)
- **UDP:** ip::udp::socket,支持广播多播
- **ICMP:** 支持ICMP消息的收发
- **串口:** 支持windows COM口,linux/macos:/dev/tty设备

## 高精度定时器
- **steady_timer:** 基于“稳定时钟”（Steady Clock），时钟频率不受系统时间调整影响（如用户修改系统时间、NTP同步），适用于需要精确间隔的场景（如定时任务、超时控制）。
- **system_timer:** 基于“系统时钟”（System Clock），时钟时间与系统当前时间一致，适用于需要与系统时间关联的场景（如定时在每天凌晨执行任务）。
- **high_resolution_timer** 基于“高精度时钟”（High-resolution Clock），提供最高精度的时间测量（通常可达纳秒级），适用于对时间精度要求极高的场景（如性能基准测试）。

## I/O执行策略
- 单线程执行: 单个io_context在一个线程中运行，适用于IO密集型
- 多线程执行: 多个线程同时调用io_context::run
- strand序列化: 通过io_context::strand确保多个异步任务串行执行，避免竞争
- 自定义执行器

## io_context
io_context的核心作用是维护一个事件队列，异步操作完成的时候对应的完成事件会被加入队列，io_context::run()会循环处理队列中的事件，直到队列为空，io_service是asio1.10.x及之前的类名，之后使用io_context

asio中提供了同步IO和异步IO两种IO函数
异步io：async_connect,async_xxx，例如 async_send,async_read_some
同步io: connect

所有的io操作都要经过io_service
最后退出之前调用io_service的run方法


# 例程
## tcp_client
``` cpp
#include <asio.hpp>
#include <asio/error_code.hpp>
#include <iostream>
#include <array>
#include <functional>

#include "common/Log.hpp"

int main()
{
  // 加载日志配置
  common::LoggerConfig config;
  common::loadLogConfig(config, "log.yaml");
  common::create_logger()->Init(config);

  asio::io_service ios; // 核心事件循环
  asio::ip::tcp::socket socket(ios);
  asio::error_code ec;
  auto server_endpoint = asio::ip::tcp::endpoint(asio::ip::make_address_v4("127.0.0.1"), 9527);

  LOG_INFO("connecting to server ...");
  socket.connect(server_endpoint, ec);
  if (ec)
  {
    LOG_ERR("connect failed: {}", ec.message());
    return -1;
  }
  LOG_INFO("connect success");

  std::array<char, 1024> recv_buf{}; // 可写缓冲区
  std::string input;

  // 启动初次异步读取
  std::function<void()> do_read;
  do_read = [&]() {
    socket.async_read_some(asio::buffer(recv_buf), [&](const std::error_code& r_ec, std::size_t len) {
      if (r_ec)
      {
        LOG_ERR("read failed: {}", r_ec.message());
        return;
      }
      LOG_INFO("read {} bytes: {}", len, std::string(recv_buf.data(), len));
      // 继续读取
      do_read();
    });
  };
  do_read();

  // 循环读取用户输入并发送
  while (std::getline(std::cin, input))
  {
    if (input.empty())
      break;
    socket.async_send(asio::buffer(input), [&](const std::error_code& s_ec, std::size_t bytes) {
      if (s_ec)
      {
        LOG_ERR("send failed: {}", s_ec.message());
      }
      else
      {
        LOG_INFO("send success {} bytes", bytes);
      }
    });
    // 运行事件循环一次处理已挂起操作（在真实程序里可能要独立线程或放在主循环外）
    ios.poll();
  }

  // 彻底跑完剩余事件
  ios.run();
  socket.close();
  return 0;
}
```

## tcp_server
``` cpp
#include <asio.hpp>
#include "common/Log.hpp"
#include <vector>
#include <memory>
#include <functional>

// 使用 std::enable_shared_from_this 来安全地从成员函数中获取 shared_ptr
class session : public std::enable_shared_from_this<session>
{
public:
    session(asio::ip::tcp::socket socket)
        : socket_(std::move(socket))
    {
    }

    void start()
    {
        do_read();
    }

private:
    void do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(asio::buffer(data_, max_length),
            [this, self](const asio::error_code& ec, std::size_t length)
            {
                if (!ec)
                {
                    do_write(length);
                }
            });
    }

    void do_write(std::size_t length)
    {
        auto self(shared_from_this());
        asio::async_write(socket_, asio::buffer(data_, length),
            [this, self](const asio::error_code& ec, std::size_t /*length*/)
            {
                if (!ec)
                {
                    do_read();
                }
            });
    }

    asio::ip::tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

void do_accept(asio::ip::tcp::acceptor& acceptor)
{
    acceptor.async_accept(
        [&acceptor](const asio::error_code& ec, asio::ip::tcp::socket socket)
        {
            if (!ec)
            {
                // 这里看起来session出了这个括号生命周期就结束了
                // 但是调用start之后会立马do_read,async_read_some的回调函数中捕获了自己本身
                // 此时session的生命周期就移交给了io_service了
                // read完之后有立马write，同上生命周期又交给io_service了所以 session不会析构掉而是和io_service生命周期绑定了
                std::make_shared<session>(std::move(socket))->start();
            }

            do_accept(acceptor);
        });
}


int main(){
// 加载日志配置
  common::LoggerConfig config;
  common::loadLogConfig(config, "log.yaml");
  common::create_logger()->Init(config);
  asio::io_context ioc;
  asio::ip::tcp::endpoint server_endpoint(asio::ip::make_address_v4("127.0.0.1"),9527);
  asio::ip::tcp::acceptor server(ioc,server_endpoint);
  
  do_accept(server);

  ioc.run();
}
```