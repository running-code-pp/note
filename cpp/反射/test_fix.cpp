#include "reflect.h"
#include <iostream>

struct person
{
    ENABLE_REFLECT()
    REFLECTABLE(person, std::string, name)
    REFLECTABLE(person, int, age)
};

int main()
{
    // 先测试成员数量
    auto desc = reflection::getDescriptor<person>();
    std::cout << "成员数量: " << desc->getMembers().size() << std::endl;

    person p;
    p.name = "Alice";
    p.age = 25;

    std::cout << desc->dump(&p) << std::endl;
    return 0;
}
