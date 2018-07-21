#include <iostream>
#include "Coro.h"


coroutine::coroutine(const coroutine_func &f, int id)
        : func_(f),
          status_(CORO_READY),
          id_(id),
          stack_(nullptr) {
//    stack_ = new char[MAX_STACK_SIZE];
};
//coroutine::coroutine(const coroutine_func &f)
//        : func(f), status(CORO_READY) {}


coroutine::~coroutine() = default;

void coroutine::set_status(Coro_Satus s) {
    status_ = s;
}

void coroutine::run() {
    assert(func_ != nullptr);
    func_();
}

Coro::Coro(int ss, int cap)
        : stack_size(ss),
          cap(cap), curr_(-1) {

}

Coro::~Coro() = default;

void Coro::excute_fun(uint32_t low32, uint32_t hi32) {
    uintptr_t ptr = static_cast<uintptr_t>(low32) | (static_cast<uintptr_t>(hi32) << 32);
    Coro *schedule = reinterpret_cast<Coro *>(ptr);
    auto id = schedule->curr_;
    auto it = schedule->coroutines.find(id);
    assert(it != schedule->coroutines.end());
    auto co = it->second;
    co->run();
    schedule->coroutines[schedule->curr_] = nullptr;
    co->set_status(CORO_DEATH);
    schedule->curr_ = -1;
}

void Coro::resume(int id) {
    assert(id >= 0 && id < cap);
    auto it = coroutines.find(id);
    assert(it != coroutines.end());
    assert(curr_ == -1);

    auto new_co = it->second;

    assert(new_co != nullptr);
    auto status = new_co->status();

    switch (status) {
        case CORO_READY: {
            getcontext(&new_co->ctx());
            auto s = new_co->stack();
            s = new char[stack_size];
//            memset(new_co->stack_,0,1024*1024);
            auto &new_co_ctx = new_co->ctx();
            new_co_ctx.uc_stack.ss_sp = s;
            new_co_ctx.uc_stack.ss_size = static_cast<size_t>(stack_size);
            new_co_ctx.uc_link = &mctx;
            curr_ = id;
            new_co->set_status(CORO_RUNNING);

            uintptr_t ptr = reinterpret_cast<uintptr_t>(this);
            makecontext(&new_co->ctx(), reinterpret_cast<void (*)()>(excute_fun), 2, static_cast<uint32_t>(ptr),
                        static_cast<uint32_t>((ptr >> 32)));
            swapcontext(&mctx, &new_co_ctx);
            break;
        }
        case CORO_SUSPEND: {
            curr_ = id;
            new_co->status() = CORO_RUNNING;
            swapcontext(&mctx, &new_co->ctx());
            break;
        }
        default:
            assert(0);
    }

}

void Coro::yield() {
    assert(curr_ != -1);
    auto co = coroutines[curr_];
    assert(co->status() != CORO_DEATH);

    co->status() = CORO_SUSPEND;
    curr_ = -1;
    swapcontext(&co->ctx(), &mctx);
}

int Coro::create(const coroutine_func &f) {
    if (co_num() >= cap && co_num() < CO_NUM_LIMIT) {
        int id = cap;
        auto new_co = std::make_shared<coroutine>(f, id);
        coroutines.insert({id, new_co});
        cap *= 2;
        auto &nco = co_num();
        nco++;
        return id;
    } else {
        int id = -1;

        for (int i = 0; i < cap; ++i) {
            id = (i + nco) % cap;
            auto it = coroutines.find(id);
            if (it == coroutines.end()) {
                Coroutine_ptr new_co = std::make_shared<coroutine>(f, id);
//            Coroutine_ptr new_co(new coroutine(f, id));
//            coroutines[id] = new_co;
                coroutines.insert({id, new_co});
                return id;
            }
        }
    }
}

int &Coro::co_num() {
    return nco;
}

Coroutine_ptr Coro::get_co(int id) {
    assert(id >= 0 && id < cap);
    auto it = coroutines.find(id);
    if (it == coroutines.end())
        return nullptr;

    auto co = it->second;
    if (co != nullptr) {
        return co;
    }
    return nullptr;
}

Coro_Satus Coro::get_co_status(int id) {
    auto co = get_co(id);
    if (co == nullptr) {
        return CORO_DEATH;
    }

    return co->status();
}


