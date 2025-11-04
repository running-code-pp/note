# 概述
ADL 指的是 Argument-Dependent Lookup（参数相关查找），也称为“Koenig 查找”。

它的作用是：当你调用一个函数时，如果该函数的参数类型属于某个命名空间，编译器会自动在该命名空间中查找是否有匹配的函数定义，而不仅仅在当前作用域查找。
```cpp
namespace foo {
    struct Bar {};
    void func(Bar) { /* ... */ }
}
namespace foo {
    struct Bar {};
    void func(Bar) { /* ... */ }
}

foo::Bar b;
func(b); // 虽然没有 using namespace foo，但编译器会自动在 foo 里查找 func
foo::Bar b;
func(b); // 虽然没有 using namespace foo，但编译器会自动在 foo 里查找 func
```

ADL 让 C++ 的函数调用更加灵活，尤其在模板和泛型编程中非常重要。