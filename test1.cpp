//
// Created by liiiyu on 7/21/18.
//

#include "Coro.h"
#include <iostream>

std::shared_ptr<Coro>coro(new Coro);

void func22() {
    puts("22");
    puts("22");
    coro->yield();
    puts("22");
    puts("22");
}

void func33() {
    puts("3333");
    puts("3333");
    coro->yield();
    puts("3333");
    puts("3333");
}

int main() {
    auto co1 = coro->create(std::bind(&func22));
    auto co2 = coro->create(std::bind(&func33));

    while (coro->get_co(co1) != nullptr && coro->get_co(co2) != nullptr){
        coro->resume(co1);
        coro->resume(co2);
    }
    return 0;
}