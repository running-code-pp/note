#include "reflect.h"
#include <iostream>

struct person
{
    ENABLE_REFLECT()
    REFLECTABLE(person, std::string, name)
    REFLECTABLE(person, int, age)
};

// 在某个cpp文件中定义静态成员
reflection::StructDescriptor person::Reflection("person", sizeof(person));

int main()
{
    // 打印成员数量来调试
    std::cout << "Members count: " << person::Reflection.getMembers().size() << std::endl;

    person p;
    p.name = "Alice";
    p.age = 25;

    auto desc = reflection::getDescriptor<person>();
    std::cout << desc->dump(&p) << std::endl;

    return 0;
}
