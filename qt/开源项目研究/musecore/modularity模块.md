# 编译器函数签名
msvc: __FUNC_SIG__
gcc/clang: __PRETTY_FUNCTION__



``` cpp
#include<iostream>
#ifndef FUNC_SIG
#if defined(_MSC_VER)
#define FUNC_SIG __FUNCSIG__
#else
#define FUNC_SIG __PRETTY_FUNCTION__
#endif
#endif

void func_test(int a){
  std::cout<<FUNC_SIG<<std::endl;
}

namespace test_ns{
 class class_test{
      public:
              int mem_func(int a){
                      std::cout<<FUNC_SIG<<std::endl;
                      return 0;
              }
 };
};// namespace test_ns

int main(){
        func_test(1);
        test_ns::class_test ct;
        ct.mem_func(0);
        return 0;
}
```
```
gcc/clang输出：
void func_test(int)
int test_ns::class_test::mem_func(int)

msvc输出:
void __cdecl func_test(int)
int __cdecl test_ns::class_test::mem_func(int)
```