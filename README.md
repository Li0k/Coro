# Coro

A coroutine library for c++ only for study

use ucontext

provide a schedule Coro to handle the coroutine

provide some function

- create

  ```c++
  int create(const coroutine_func &f);
  ```

  create a coroutine and return its id

- resume

  ```c++
  void resume(int id);
  ```

  use id to resume a coroutine you want

- yield

  ```c++
  void yield();
  ```

  to yield a coroutine 

- other

  use marco to change property of Coro and coroutine

  - MAX_STACK_SIZE  

    set the max size of coroutine runnning stack 

  - DEFAULT_NUM

    set the default num of coroutine

  - CO_NUM_LIMIT

    set the max num of coroutine

otherwise Coro provide a Channel class to transfer some message from main thread to coroutine,but it only a toy

it provide some function

- pop

  ```c++
  template<typename Type>
  Type &Channel<Type>::pop()
  ```

- push

  ```c++
  template<typename Type>
  void Channel<Type>::push(const Type &v)
  ```



some example in test1.cpp and test2.cpp