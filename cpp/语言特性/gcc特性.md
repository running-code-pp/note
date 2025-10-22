
# [[gnu::may_alias]]
gunc++ 一个特有的属性，告诉编译器允许这个类型与其他不相关的类型进行别名访问，违反了标准c++的严格别名规则

```cpp
void foo(){
int x=42;
float* p = reinterpret_cast<float*>(&x);
std::cout<<*p<<std::endl;//这是未定义行为
}
```
**使用[[gnu::may_alias]]**
```cpp
typedef float __attribute__((__may_alias__)) float_alias;
using float_alias = float [[gnu::may_alias]];

void foo(){
 int x = 42;
    float_alias* p = reinterpret_cast<float_alias*>(&x); // 合法且安全
    std::cout << *p << std::endl; // 不再是未定义行为（在GCC下）
}

```


# label as values特性
goto label

对于goto跳转语句

gcc中允许通过&&获取label的地址，该地址是一个常量,这里的&&是单目运算符
```cpp
int main(){
void* labelPtr = && label;
goto *labelPtr;
printf("before label...");
label:
printf("after label...");
return 0;
}
```

