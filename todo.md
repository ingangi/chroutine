## 1. yield by time

等待一个事件后再resume，或超时后resume

## 2. 如何实现当有io阻塞时自动yield

- hook一些glibc中的系统函数  比如socket的系列函数

参考: https://www.cnblogs.com/unnamedfish/p/8460441.html

## 3. 增加线程池调度