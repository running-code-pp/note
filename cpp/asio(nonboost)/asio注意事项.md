
- [asio中线程安全/不安全的操作](#asio中线程安全不安全的操作)
- [strand的注意事项](#strand的注意事项)
- [io\_context不会空转](#io_context不会空转)
- [asio::post 不直接支持带参数的任务](#asiopost-不直接支持带参数的任务)
- [io\_context post，defer和dispatch的行为差异](#io_context-postdefer和dispatch的行为差异)
- [定时器的精度与性能优化](#定时器的精度与性能优化)
    - [定时器精度的底层限制](#定时器精度的底层限制)
    - [高并发场景下的性能优化方案](#高并发场景下的性能优化方案)
  - [常见问题与避坑指南](#常见问题与避坑指南)
    - [问题1：io.run()未调用，定时器回调不执行](#问题1iorun未调用定时器回调不执行)
    - [问题2：定时器被重复async\_wait()，导致多次回调](#问题2定时器被重复async_wait导致多次回调)
    - [问题3：system\_timer受系统时间调整影响，定时逻辑异常](#问题3system_timer受系统时间调整影响定时逻辑异常)
- [asio使用c++20协程](#asio使用c20协程)

# asio中线程安全/不安全的操作
`**线程安全的操作（可多线程并发调用）**`

- io_context::post()/dispatch()/defer()：提交任务；
- io_context::stop()：停止事件循环；
- io_context::stopped()：检查是否已停止；
- 网络对象的取消操作：如socket::cancel()、timer::cancel()；
- 线程池相关操作：如thread_pool::join()。
- 
`**非线程安全的操作（禁止多线程并发调用）**`
- io_context::run()/run_one()/poll()/poll_one()：同一io_context可被多线程调用run()，但单线程内不可重入；
- 修改对象状态的操作：如socket::bind()/connect()、timer::expires_at()；
- 销毁Asio对象：如io_context、socket、strand的析构函数；
- 访问对象内部状态：如socket::available()、timer::expires_from_now()。

# strand的注意事项
`**尽量使用strand替代mutex**`

- 无阻塞等待：线程不会因Strand被锁定而阻塞，而是继续执行其他任务（如其他Strand的任务），提高CPU利用率；
- 批量执行任务：一次锁定后执行队列中所有任务，减少“锁定-解锁”的开销；
- 与异步模型深度集成：无需手动管理锁的生命周期，避免死锁风险。

`**post和bind_executor的对比**`
| 特性 | asio::post(strand, f) | asio::bind_executor(strand, f) |
|------|-----------------------|-------------------------------|
| 是否立即调度 | ✅ 是，立即提交任务 | ❌ 否，只是创建一个可调用对象 |
| 返回值 | void | 一个新的函数对象（包装器） |
| 调用时机 | 提交时立即调度 | 你手动调用返回的函数时才调度 |
| 用途 | 执行一次性任务 | 创建一个“线程安全”的回调，可多次调用 |
| 是否改变 f | 否 | 是，返回的是一个包装后的 f |

# io_context不会空转
如果没有待执行的任务，io_context不会阻塞而是直接退出

如果一个服务器需要长时间运行这显然是不行的，

work_guard是一个轻量级对象，通过与io_context绑定，向io_context发送“仍有工作待处理”的信号，即使任务队列为空，io_context::run()也会继续阻塞等待新任务。

```cpp
    asio::io_context io;
    // 1. 创建work_guard，绑定到io_context（此时io.run()不会因无任务而退出）
    auto work = asio::make_work_guard(io);
```

**`注意事项`**
- 不可拷贝，仅可移动：work_guard的拷贝构造函数和赋值运算符被删除，需通过移动语义传递（如auto work2 = std::move(work);）；
- 重置后不可恢复：调用work.reset()后，work_guard的“守护”功能永久失效，若需重新守护io_context，需重新创建work_guard；
- 避免内存泄漏：若work_guard未被重置且io_context的线程未退出，程序会因线程无法终止而内存泄漏（如服务器未正确关闭时）。


# asio::post 不直接支持带参数的任务
如果需要的话可以采用lambda+捕获列表原地初始化的方式
```cpp
int main()
{
  asio::post(
      *pool,
      [  a = 42, c = 'x'](){ 
        LOG_INFO("task with params: a={}, c={}, thread={}", a, c, std::this_thread::get_id()); 
      });
  logger->ShutDown();
  return 0;
}

```

# io_context post，defer和dispatch的行为差异
post和dispatch是提交处理程序的两种核心方式，区别在于是否“就地执行”：

post：无论当前线程是否在io_context的事件循环中，都将处理程序加入队列，等待后续调度执行（“延迟执行”）；
dispatch：若当前线程正在执行io_context的run()/run_one()，则直接在当前线程执行处理程序（“立即执行”,不会放进io_context的全局队列，而是立即就地同步执行），否则等同于post。

示例：post与dispatch的行为差异
```cpp
#include <asio.hpp>
#include <iostream>

void task(int id) {
    std::cout << "Task " << id << " executed in thread " << std::this_thread::get_id() << std::endl;
}

int main() {
    asio::io_context io;
    auto work = asio::make_work_guard(io); // 防止run()立即退出

    // 1. 在主线程（非io_context线程）调用post和dispatch
    std::cout << "Main thread id: " << std::this_thread::get_id() << std::endl;
    asio::post(io, []() { task(1); });       // 加入队列，由io线程执行
    asio::dispatch(io, []() { task(2); });   // 主线程非io线程，等同于post

    // 2. 启动io线程，在io线程中调用dispatch
    std::thread io_thread([&]() {
        asio::dispatch(io, []() { task(3); }); // 当前线程是io线程，直接执行
        io.run();
    });

    io_thread.join();
    return 0;
}

Main thread id: 0x100086e00
Task 3 executed in thread 0x16f9b3000  // dispatch在io线程中立即执行
Task 1 executed in thread 0x16f9b3000  // post的任务在io线程中调度执行
Task 2 executed in thread 0x16f9b3000  // 主线程非io线程，dispatch等同于post

```


# 定时器的精度与性能优化
在高并发或高精度场景中，定时器的精度和性能可能成为瓶颈。本节讲解Asio定时器的精度特性及优化方案。

### 定时器精度的底层限制
Asio定时器的精度并非“理论无限高”，而是受操作系统内核定时器实现和系统负载双重影响：

Windows：底层依赖WaitForSingleObjectEx或IOCP定时器，默认精度约10-15ms；若通过timeBeginPeriod调整系统时钟分辨率，可提升至1ms。
Linux：底层依赖epoll的EPOLLIN事件 + 内核定时器，默认精度约1ms；若使用CLOCK_MONOTONIC时钟（对应steady_timer），精度可稳定在1ms以内。
macOS：底层依赖kqueue的EVFILT_TIMER，精度约1ms。
精度测试代码：
```cpp
#include <iostream>
#include <asio.hpp>
#include <chrono>
#include <vector>

// 测试定时器精度：记录100次触发的实际间隔
void test_timer_precision() {
    asio::io_context io;
    asio::high_resolution_timer timer(io);
    std::vector<double> intervals;  // 存储每次触发的实际间隔（毫秒）
    auto last_trigger_time = std::chrono::high_resolution_clock::now();
    int trigger_count = 0;
    const int total_tests = 100;  // 测试100次
    const auto target_interval = 1ms;  // 目标间隔1ms

    // 递归触发定时器，记录实际间隔
    auto trigger_handler = [&]() {
        if (trigger_count < total_tests) {
            auto now = std::chrono::high_resolution_clock::now();
            // 计算实际间隔（转换为毫秒，保留3位小数）
            auto actual_interval = std::chrono::duration_cast<std::chrono::microseconds>(now - last_trigger_time).count() / 1000.0;
            intervals.push_back(actual_interval);
            last_trigger_time = now;
            trigger_count++;

            // 重置定时器，继续测试
            timer.expires_after(target_interval);
            timer.async_wait([&](const asio::error_code&) { trigger_handler(); });
        } else {
            // 测试完成，输出统计结果
            double avg = 0, min = 1e9, max = 0;
            for (auto& interval : intervals) {
                avg += interval;
                min = std::min(min, interval);
                max = std::max(max, interval);
            }
            avg /= total_tests;

            std::cout << "定时器精度测试结果（目标间隔：" << target_interval.count() << "ms）：" << std::endl;
            std::cout << "平均间隔：" << avg << "ms" << std::endl;
            std::cout << "最小间隔：" << min << "ms" << std::endl;
            std::cout << "最大间隔：" << max << "ms" << std::endl;
            std::cout << "误差范围：±" << std::max(avg - min, max - avg) << "ms" << std::endl;
        }
    };

    // 启动第一次测试
    timer.expires_after(target_interval);
    timer.async_wait([&](const asio::error_code&) { trigger_handler(); });
    io.run();
}

int main() {
    test_timer_precision();
    return 0;
}

典型输出（Linux系统）：

定时器精度测试结果（目标间隔：1ms）：
平均间隔：1.023ms
最小间隔：0.987ms
最大间隔：1.056ms
误差范围：±0.069ms

```


### 高并发场景下的性能优化方案
当系统中存在成千上万的定时器（如百万级并发连接的超时管理）时，直接使用Asio原生定时器可能导致io_context事件循环负载过高。需通过以下方案优化：

1. 定时器合并（Timer Coalescing）
对于相同周期的定时任务（如所有会话的30秒超时检查），无需为每个任务创建独立定时器，而是用一个“全局定时器”批量处理所有任务。

优化思路：

维护一个“任务列表”，存储所有需要在相同周期执行的任务。
创建一个全局steady_timer，按周期触发后，遍历任务列表执行所有任务。
代码示例：
```cpp
#include <iostream>
#include <asio.hpp>
#include <vector>
#include <functional>
#include <chrono>

using namespace std::chrono_literals;

// 定时器合并管理器：用一个全局定时器处理多个同周期任务
class CoalescedTimerManager {
public:
    CoalescedTimerManager(asio::io_context& io, std::chrono::milliseconds interval)
        : io_(io),
          global_timer_(io),
          interval_(interval),
          task_list_()  // 存储所有待执行的任务
    {
        // 启动全局定时器
        start_global_timer();
    }

    // 注册任务：将任务加入列表，无需创建独立定时器
    void register_task(std::function<void()> task) {
        task_list_.emplace_back(std::move(task));
        std::cout << "注册新任务，当前任务总数：" << task_list_.size() << std::endl;
    }

private:
    // 启动全局定时器，周期性触发任务执行
    void start_global_timer() {
        global_timer_.expires_after(interval_);
        global_timer_.async_wait(
            [this](const asio::error_code& ec) {
                if (!ec) {
                    // 批量执行所有任务
                    execute_all_tasks();
                    // 递归启动下一轮定时器
                    start_global_timer();
                }
            });
    }

    // 执行所有注册的任务
    void execute_all_tasks() {
        std::cout << "全局定时器触发，执行 " << task_list_.size() << " 个任务" << std::endl;
        // 遍历执行所有任务（注意：若任务可能被动态移除，需处理迭代器失效问题）
        for (const auto& task : task_list_) {
            task();  // 执行任务
        }
    }

private:
    asio::io_context& io_;
    asio::steady_timer global_timer_;  // 全局唯一定时器
    std::chrono::milliseconds interval_;  // 任务执行周期
    std::vector<std::function<void()>> task_list_;  // 待执行任务列表
};

// 测试：模拟100个同周期任务，通过合并定时器执行
int main() {
    asio::io_context io;
    // 创建合并定时器管理器，周期为2秒
    CoalescedTimerManager timer_manager(io, 2000ms);

    // 注册100个测试任务
    for (int i = 0; i < 100; ++i) {
        timer_manager.register_task([i]() {
            std::cout << "任务 " << i << " 执行（当前时间：" 
                      << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) 
                      << "）" << std::endl;
        });
    }

    // 运行事件循环
    io.run();
    return 0;
}
```
优化效果：将100个独立定时器的“100次异步等待”简化为“1次全局定时器等待”，大幅减少io_context的事件调度开销，降低CPU使用率（尤其在任务数量达数千/数万时效果显著）。

2. 使用定时器轮（Timer Wheel）数据结构
对于大量不同周期的定时器（如不同会话的超时时间：10秒、30秒、60秒），定时器轮是更高效的管理方案。其核心思想是“按时间粒度分层存储定时器”，避免每次检查所有定时器。

Asio本身未内置定时器轮，但可基于第三方库（如boost::asio::experimental::basic_waitable_timer）或自定义实现。以下是简化的定时器轮核心逻辑：
```cpp
#include <iostream>
#include <asio.hpp>
#include <vector>
#include <unordered_set>
#include <chrono>

using namespace std::chrono_literals;

// 简化的定时器轮实现（单层级，时间粒度1秒）
class TimerWheel {
public:
    TimerWheel(asio::io_context& io, std::size_t wheel_size = 60)  // 轮盘大小：60个槽（对应0-59秒）
        : io_(io),
          tick_timer_(io),
          current_slot_(0),
          wheel_(wheel_size)  // 每个槽存储该秒需要触发的定时器
    {
        // 启动“滴答”定时器（每1秒前进一个槽）
        start_tick();
    }

    // 添加定时器：延迟delay秒后触发
    void add_timer(std::chrono::seconds delay, std::function<void()> task) {
        if (delay.count() <= 0) {
            task();  // 延迟为0，立即执行
            return;
        }
        // 计算定时器应存入的槽位（当前槽 + 延迟）% 轮盘大小
        std::size_t target_slot = (current_slot_ + delay.count()) % wheel_.size();
        // 将任务存入目标槽位
        wheel_[target_slot].emplace(std::move(task));
        std::cout << "添加定时器：延迟 " << delay.count() << " 秒，存入槽位 " << target_slot << std::endl;
    }

private:
    // 每1秒触发一次“滴答”，前进槽位并执行当前槽的任务
    void start_tick() {
        tick_timer_.expires_after(1s);
        tick_timer_.async_wait(
            [this](const asio::error_code& ec) {
                if (!ec) {
                    // 执行当前槽位的所有任务
                    execute_current_slot_tasks();
                    // 前进到下一个槽位（循环）
                    current_slot_ = (current_slot_ + 1) % wheel_.size();
                    // 继续下一次滴答
                    start_tick();
                }
            });
    }

    // 执行当前槽位的所有任务
    void execute_current_slot_tasks() {
        auto& current_tasks = wheel_[current_slot_];
        if (!current_tasks.empty()) {
            std::cout << "滴答：当前槽位 " << current_slot_ << "，执行 " << current_tasks.size() << " 个任务" << std::endl;
            // 执行所有任务（执行后清空槽位）
            for (const auto& task : current_tasks) {
                task();
            }
            current_tasks.clear();
        }
    }

private:
    asio::io_context& io_;
    asio::steady_timer tick_timer_;  // 每1秒触发的“滴答”定时器
    std::size_t current_slot_;  // 当前指向的槽位
    std::vector<std::unordered_set<std::function<void()>>> wheel_;  // 定时器轮盘（槽位数组）
};

// 测试：添加不同延迟的定时器
int main() {
    asio::io_context io;
    TimerWheel timer_wheel(io, 60);  // 60个槽位，覆盖0-59秒

    // 添加不同延迟的定时器
    timer_wheel.add_timer(2s, []() { std::cout << "任务1：延迟2秒执行" << std::endl; });
    timer_wheel.add_timer(5s, []() { std::cout << "任务2：延迟5秒执行" << std::endl; });
    timer_wheel.add_timer(61s, []() { std::cout << "任务3：延迟61秒执行（跨轮盘1圈+1秒）" << std::endl; });
    timer_wheel.add_timer(3s, []() { std::cout << "任务4：延迟3秒执行" << std::endl; });

    // 运行事件循环
    io.run();
    return 0;
}
```
核心优势：

时间复杂度优化：添加定时器（O(1)）、执行定时器（O(1)，仅处理当前槽任务），远优于“遍历所有定时器检查超时”的O(n)复杂度。
内存高效：避免为每个定时器创建独立的steady_timer对象，通过“槽位+任务”的轻量级结构管理大量定时器。
3. 避免“过度精细”的定时器
除非业务有强制要求（如高频交易），否则应避免使用过小的定时间隔（如100微秒）。这类定时器会：

频繁触发回调，占用大量CPU时间；
受系统调度延迟影响，实际精度难以保证。
优化建议：

网络超时场景：最低使用10ms间隔（大部分网络场景无需更高精度）；
周期性任务场景：根据业务需求调整（如日志切割用1分钟，会话检查用10秒）；
若需高频触发，优先使用“批量处理”（如每1ms处理100个任务，而非100个1ms定时器）。
## 常见问题与避坑指南
在使用Asio定时器时，开发者常因对异步模型理解不深入而踩坑。以下是高频问题及解决方案：

### 问题1：io.run()未调用，定时器回调不执行
现象：创建定时器并调用async_wait()，但回调函数始终不触发。
原因：io_context::run()是事件循环的“引擎”，未调用时，io_context不会处理任何异步事件（包括定时器超时）。
解决方案：

确保在async_wait()后调用io.run()；
若需停止事件循环，可调用io.stop()或io.reset()（重置后可再次调用run()）。
```cpp
错误示例：

void wrong_example() {
    asio::io_context io;
    asio::steady_timer timer(io, 1s);
    timer.async_wait([](const asio::error_code&) {
        std::cout << "回调不会执行（未调用io.run()）" << std::endl;
    });
    // 遗漏 io.run()，事件循环未启动
}

正确示例：

void correct_example() {
    asio::io_context io;
    asio::steady_timer timer(io, 1s);
    timer.async_wait([](const asio::error_code&) {
        std::cout << "回调会执行（已调用io.run()）" << std::endl;
    });
    io.run();  // 必须调用，启动事件循环
}
```

### 问题2：定时器被重复async_wait()，导致多次回调
现象：同一个定时器对象被多次调用async_wait()，超时后回调函数执行多次。
原因：Asio定时器支持“多等待”——一个定时器可同时注册多个回调，超时后所有回调都会被触发。
解决方案：

若需“单次触发”，确保每次async_wait()前取消之前的等待（调用timer.cancel()）；
若需“周期性触发”，在回调中重置定时器并只调用一次async_wait()（避免递归时重复注册）。
```cpp
错误示例：

void wrong_multiple_wait() {
    asio::io_context io;
    asio::steady_timer timer(io, 1s);
    
    // 错误：同一个定时器注册2次回调，超时后会执行2次
    timer.async_wait([](const asio::error_code&) { std::cout << "回调1" << std::endl; });
    timer.async_wait([](const asio::error_code&) { std::cout << "回调2" << std::endl; });
    
    io.run();  // 输出：回调1 + 回调2（两次）
}

正确示例（单次触发）：

void correct_single_wait() {
    asio::io_context io;
    asio::steady_timer timer(io, 1s);
    
    // 正确：注册前取消之前的等待（若有）
    timer.cancel();  // 确保无残留等待
    timer.async_wait([](const asio::error_code&) { std::cout << "仅执行一次" << std::endl; });
    
    io.run();  // 输出：仅执行一次
}
```

### 问题3：system_timer受系统时间调整影响，定时逻辑异常
现象：使用system_timer时，若手动修改系统时间（如将时间调回1小时前），定时器可能延迟触发或重复触发。
原因：system_timer基于系统时钟（std::chrono::system_clock），系统时间的修改会直接影响其过期判断。
解决方案：

优先使用steady_timer（基于单调递增的steady_clock，不受系统时间调整影响）；
仅在“需与系统时间对齐”的场景（如每天凌晨3点备份）使用system_timer，且需处理时间回拨的异常情况。
对比示例：
```cpp
void compare_timers() {
    asio::io_context io;
    
    // system_timer：受系统时间影响
    asio::system_timer sys_timer(io, asio::chrono::system_clock::now() + 10s);
    sys_timer.async_wait([](const asio::error_code&) {
        std::cout << "system_timer 触发（若期间修改系统时间，可能异常）" << std::endl;
    });
    
    // steady_timer：不受系统时间影响
    asio::steady_timer steady_timer(io, 10s);
    steady_timer.async_wait([](const asio::error_code&) {
        std::cout << "steady_timer 触发（无论系统时间如何修改，10秒后必触发）" << std::endl;
    });
    
    io.run();
}
```

# asio使用c++20协程
asio1.14.0版本之后开始支持c++20的协程

在cmakelists.txt中添加
```text
# 定义ASIO_HAS_CO_AWAIT宏
target_compile_definitions(${PROJECT_NAME} PRIVATE ASIO_HAS_CO_AWAIT)
```