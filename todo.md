# Upcoming

## chroutine in main thread

## write to channel with select

## schedule 2.0
### A. move chroutines to other thread if current thread is blocked for some time
### B. Dynamic allocation when thread resources are insufficient
(This feature is so important that it should be prioritized so that we can handle blocking threads as well.)
> unhooked syscalls (read/write files etc.)

> third party APIs (mysql client etc.)

## share chroutine stack

# Low priority

## mysql client

## redis client

## Optimize spin-lock strategy

//## hook more: https://www.cnblogs.com/unnamedfish/p/8460441.html