/**
 * @Author: running-code-pp
 * @Date: 2025-10-28 15:54:40
 * @LastEditors: running-code-pp
 * @LastEditTime: 2025-10-28 18:01:54
 * @FilePath: \note\cpp\反射\example.cpp
 * @Description:
 * @Copyright: Copyright (c) 2025 by running-code-pp 3320996652@qq.com, All Rights Reserved.
 */
#include "reflect.h"
#include <iostream>

struct person
{
    ENABLE_REFLECT();
    REFLECTABLE(std::string, name);
    REFLECTABLE(int, age);
};

// 在某个cpp文件中定义静态成员并注册所有字段
DEFINE_REFLECTION(person,
                  REFLECT_MEMBER(person, std::string, name),
                  REFLECT_MEMBER(person, int, age));

int main()
{
    person p;
    p.name = "Alice";
    p.age = 25;

    auto desc = reflection::getDescriptor<person>();
    std::cout << desc->dump(&p) << std::endl;

    return 0;
}
