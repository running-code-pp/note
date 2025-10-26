# 概述
C++ Standard 规定，对易失性变量的读取不能被缓存，易失性写入不能被延迟，并且易失性读取和写入不能相互移动。 这足以与硬件设备通信，这是 C++ Standard 中可变关键字的用途。

然而，该标准的保证不足以将易失性用于多线程。 C++ Standard 并没有阻止编译器相对于易失性读写重新排序非易失性读取和写入，也没有提到防止 CPU 重新排序。

**注意**
```
Visual C++ 2005 超越了标准 C++，为易失性变量访问定义了多线程友好的语义。 
从 Visual C++ 2005 开始，对易失性变量的读取被定义为具有读取-获取语义，对易失性变量的写入被定义为具有写入-释放语义。 
这意味着编译器不会重新排列任何读写操作；在 Windows 上，它也会确保 CPU 不会这样做。
```
编译器为了提高性能，会对代码进行优化。例如
```c
编辑
// 假设 flag 是一个全局变量
while (flag) {
    // 做一些事
}
```
如果编译器发现 flag 在循环中没有被修改，它可能认为 flag 的值不会变，于是优化成：

```c
if (flag) {
    while (1) {
        // 死循环
    }
}
```
但如果 flag 是由中断服务程序或另一个线程修改的，这个优化就会导致程序永远无法退出循环！

✅ volatile 如何解决这个问题？
加上 volatile 后：

```c
volatile int flag = 1;

while (flag) {
    // 每次都会从内存读取 flag 的最新值
}
```
✅ 编译器不会再做上述优化，每次循环都会从内存中重新读取 flag，确保能及时响应外部变化。

# 注意
仅防止编译器优化。
不保证原子性，不阻止 CPU 重排序。

将非 volatile 值强制转换为 volatile 类型没有效果。要使用 volatile 语义访问非 volatile 对象，必须将其地址强制转换为指向 volatile 的指针，然后通过该指针进行访问。

任何通过非 volatile 左值读写 volatile 限定类型的对象的尝试都会导致未定义行为。
```cpp
volatile int n = 1; // object of volatile-qualified type
int* p = (int*)&n;
int val = *p; // undefined behavior
```

volatile 限定结构或联合类型的成员会获得其所属类型的限定（无论是使用 . 运算符还是 -> 运算符访问）。
```cpp
struct s { int i; const int ci; } s;
// the type of s.i is int, the type of s.ci is const int
volatile struct s vs;
// the types of vs.i and vs.ci are volatile int and const volatile int
```

如果数组类型通过使用 typedef 声明为 volatile 类型限定符，则数组类型不是 volatile 限定的，但其元素类型是。

(直至 C23)
数组类型及其元素类型始终被认为是具有相同 volatile 限定的。
```cpp
(自 C23 起)
typedef int A[2][3];
volatile A a = {{4, 5, 6}, {7, 8, 9}}; // array of array of volatile int
int* pi = a[0]; // Error: a[0] has type volatile int*
void *unqual_ptr = a; // OK until C23; error since C23
// Notes: clang applies the rule in C++/C23 even in C89-C17 modes
```

如果函数类型通过使用 typedef 声明为 volatile 类型限定符，则行为是未定义的。

在函数声明中，关键字 volatile 可以出现在用于声明函数参数数组类型的方括号内。它限定了数组类型转换成的指针类型。

以下两个声明声明了相同的函数
```cpp
void f(double x[volatile], const double y[volatile]);
void f(double * volatile x, const double * volatile y);
```
(C99 起)
指向非 volatile 类型的指针可以隐式转换为指向相同或兼容类型的 volatile 限定版本的指针。反向转换需要强制转换表达式。
```cpp
int* p = 0;
volatile int* vp = p; // OK: adds qualifiers (int to volatile int)
p = vp; // Error: discards qualifiers (volatile int to int)
p = (int*)vp; // OK: cast
```