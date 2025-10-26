/** 
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-25 20:06:28
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-25 20:07:45
 * @FilePath: \note\cpp\无锁编程\test_align_type_rw.cpp
 * @Description:  测试自然对齐的大小小于内存总线宽度的数据类型的读写原子性
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
 */


// 大多数现代cpu对于自然对齐的大小小于等于内存总线宽度的数据类型的读写操作都是原子的

#include<thread>
#include<sstream>
#include<string>

std::string this_thread_id() {
	std::ostringstream oss;
	oss << std::this_thread::get_id();
	return oss.str();
}
int global_counter = 0;

// 这里的操作是原子性的,最终的global_counter符合预期结果2000
void thread_func() {
	for(int i=0;i<1000;i++) {
		++global_counter;
		printf("thread-%s,global_counter:%d\n",this_thread_id().c_str(),global_counter);
	}
}

void test_global_int_rw() {
	std::jthread t1(thread_func);
	std::jthread t2(thread_func);
}

int main() {
    test_global_int_rw();
    return 0;
}