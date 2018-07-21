#ifndef CORO_CORO_H
#define CORO_CORO_H

#include <functional>
//#define _XOPEN_SOURCE 600
#include <ucontext.h>
#include <unordered_map>
#include <memory>
#include <assert.h>
#include <list>
#include <queue>

enum Coro_Satus {
    CORO_DEATH,
    CORO_READY,
    CORO_RUNNING,
    CORO_SUSPEND
};

typedef std::function<void()> coroutine_func;

#define MAX_STACK_SIZE (1024 * 1024)
#define DEAFULT_NUM 16
#define CO_NUM_LIMIT 256


class coroutine {
public:
    coroutine(const coroutine_func &f, int id);

    ~coroutine();

    Coro_Satus &status() { return status_; }

    void set_status(Coro_Satus s);

    ucontext_t &ctx() { return ctx_; }

    char *stack() { return stack_; }

    int &id() { return id_; }


    void run();

private:
    int id_;
    ucontext_t ctx_;//context
    coroutine_func func_;//running function
    Coro_Satus status_;//running status
//    char stack[MAX_STACK_SIZE];// running stack
    char *stack_;

};

typedef std::shared_ptr<coroutine> Coroutine_ptr;

class Coro {
public:

//    Coro(int ss = MAX_STACK_SIZE);
    explicit Coro(int ss = MAX_STACK_SIZE, int cap = DEAFULT_NUM);

    ~Coro();

    void resume(int id);

    void yield();

    int create(const coroutine_func &f);

    int &co_num();

    Coroutine_ptr get_co(int id);

    Coro_Satus get_co_status(int id);

    int curr() { return curr_; };

private:
    ucontext_t mctx;//main context
//    coroutine *running_co;//running coroutine
//    std::set<coroutine *> coroutines;// store coroutines
    int curr_;//id of running coroutine
    int nco;//num of running coroutine
    int cap; //cap of Coro
//    std::vector<coroutine *> coroutines;
    std::unordered_map<int, Coroutine_ptr> coroutines;
    int stack_size;

    static void excute_fun(uint32_t low32, uint32_t hi32);
};

template<typename Type>
class Channel {
public:
    explicit Channel(std::shared_ptr<Coro> s, int id = -1) : taker_(id), schedule(std::move(s)) {

    }

    void consumer(int id) {
        taker_ = id;
    }

    void push(const Type &v);

    Type pop();

    void clear() {
        clear_(queue_);
    }

    size_t message_num() {
        return queue_.size();
    }

    bool is_empty() {
        return queue_.empty();
    }

private:
//    std::list<Type> list_;
    std::queue<Type> queue_;
    int taker_;
    std::shared_ptr<Coro> schedule;

    void clear_(std::queue<Type> &q){
        std::queue<Type> empty;
        swap(q,empty);
    }
};

template<typename Type>
Type Channel<Type>::pop() {
    if (taker_ == -1 ) {
        taker_ = schedule->curr();
    }

    auto co = schedule->get_co(taker_);
    //block
    while (queue_.empty()) {
        schedule->yield();
    }

    Type v = queue_.front();
    queue_.pop();

    return std::move(v);

}

template<typename Type>
void Channel<Type>::push(const Type &v) {
    queue_.push(v);
    if (taker_ != -1 && taker_ != schedule->curr()) {
        schedule->resume(taker_);
    }
}


#endif //CORO_CORO_H
