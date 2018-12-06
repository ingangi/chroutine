# todo list

## 1. yield by time, sync with son chroutine

等待一个事件后再resume，或超时后resume

## 2. 当有io阻塞时自动yield

- hook一些glibc中的系统函数  比如socket的系列函数

参考: https://www.cnblogs.com/unnamedfish/p/8460441.html

## 3. add thread pool

## 4. add channel between father and son chroutine

## 5. rewrite the tcp APIs with epoll and offer sync-look APIs for net programing

## 6. share chroutine stack