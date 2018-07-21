#include <stdio.h>

#include "Coro.h"

using namespace std;

void func1(void *arg) {
    puts("1");
    puts("11");
    puts("111");
    puts("1111");

}

void context_test() {
//    char stack[1024 * 128];
    char *stack;
    ucontext_t child, main;

    getcontext(&child); //获取当前上下文
    stack = new char[1024 * 128];
    child.uc_stack.ss_sp = stack;//指定栈空间
    child.uc_stack.ss_size = 1024 * 128;//指定栈空间大小
    child.uc_stack.ss_flags = 0;
    child.uc_link = &main;//设置后继上下文

    makecontext(&child, (void (*)(void)) func1, 0);//修改上下文指向func1函数

    swapcontext(&main, &child);//切换到child上下文，保存当前上下文到main
    puts("main");//如果设置了后继上下文，func1函数指向完后会返回此处
}

std::shared_ptr<Coro> coro(new Coro);

Channel<int> chan(coro, 0);

void func11() {
    puts("1");
    puts("11");
    puts("111");
    puts("1111");

}
//static Coro schedule;
#include <iostream>

void func22() {
    puts("22");
    puts("22");
    int i = chan.pop();
    std::cout << "chan get: " << i << std::endl;
//    coro->yield()

    i = chan.pop();
    std::cout << "chan get: " << i << std::endl;
    puts("22");
    puts("22");
}

void func33() {
    puts("3333");
    puts("3333");
    coro->yield();
//    chan.push(111);
    puts("3333");
    puts("3333");

}


int main() {
    auto co1 = coro->create(std::bind(&func22));
    auto co2 = coro->create(std::bind(&func33));

    coro->resume(co1);
    chan.push(111);
    coro->resume(co2);
    chan.push(555);
    return 0;
}
