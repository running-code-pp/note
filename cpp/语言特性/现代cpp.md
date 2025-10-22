
# 异常处理

## 异常类型
```
std::exception
 ├── std::logic_error                // 逻辑错误（程序可避免）
 │    ├── std::domain_error          // 参数超出函数定义域
 │    ├── std::invalid_argument      // 无效参数
 │    ├── std::length_error          // 超出长度限制
 │    └── std::out_of_range          // 访问越界
 │
 └── std::runtime_error              // 运行时错误（程序难以控制）
      ├── std::range_error           // 数值计算结果超出表示范围
      ├── std::overflow_error        // 上溢
      ├── std::underflow_error       // 下溢
      └── std::system_error (C++11)  // 系统级错误（如 POSIX 错误）
           ├── std::filesystem::filesystem_error (C++17)//文件系统操作失败（如路径不存在、权限不足）继承自 std::system_error
           └── std::future_error (C++11) //std::future, std::promise 相关操作失败
```

```
std::bad_alloc	<new>	内存分配失败（new 操作符抛出）
std::bad_cast	<typeinfo>	dynamic_cast 类型转换失败（用于引用）
std::bad_typeid	<typeinfo>	对 nullptr 指针使用 typeid
std::bad_exception	<exception>	用于 unexpected_handler 机制（已弃用）
std::bad_function_call	<functional>	调用空的 std::function
std::regex_error	<regex>	正则表达式语法错误或执行失败
std::bad_weak_ptr	<memory>	构造 std::shared_ptr 时使用了空的 std::weak_ptr
```

##  std::exception_ptr	
捕获和传递异常的智能指针