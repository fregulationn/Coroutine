#ifndef _COROUTINE_H
#define _COROUTINE_H

#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <assert.h>
#include <sys/eventfd.h>
#include <sys/time.h>

#include "coctx.h"
#include "co_queue.h"
#include "co_tree.h"

// 协程执行结束
#define CO_STATUS_DEAD 0
// 刚创建协程状态
#define CO_STATUS_NEW 1
// 协程创建后未resume，或yield后处的状态
#define CO_STATUS_READY 2
// 协程当前正在运行
#define CO_STATUS_RUNNING 3
// 当前协程resume了其它协程，此时处于这个状态
#define CO_STATUS_WAIT 4

#define DEFAULT_COROUTINE 1024
#define CO_MAX_EVENTS 1024
#define CO_MAX_STACKSIZE (16 * 1024)

struct schedule;
typedef struct schedule_t;
typedef void (*co_func)(void *);

typedef struct coroutine {
    coctx_t ctx;
    co_func func;
    void *arg;
    size_t stack_size;
    // 调度器
    schedule_t *sched;
    // 定义创建的时间
    uint64_t birth;
    // 定义栈
    void *stack;
    // 协程当前状态
    int status;
    // socket fd
    int fd;
    // 等待的事件
    uint32_t events;

}coroutine_t;


typedef struct schedule {
    coctx_t ctx;
    uint64_t birth;
    size_t stack_size;
    int page_size;
    coroutine_t *current;

    int epollfd;
    struct epoll_event eventlist[CO_MAX_EVENTS];
    int num_new_events;

    uint64_t default_timeout;

    struct Queue_t *ready;
    struct Queue_t *dead;
    struct RBRoot *wait_tree;

}schedule_t;

extern pthread_key_t global_sched_key;
static inline schedule_t *get_schedule() {
    return pthread_getspecific(global_sched_key);
}

static inline uint64_t coroutine_usec_now(void) {
	struct timeval t1 = {0, 0};
	gettimeofday(&t1, NULL);

	return t1.tv_sec * 1000000 + t1.tv_usec;
}

// 打开一个调度器，每个线程一个：stsize为栈大小，传0为默认
int schedule_create(int stsize);
// 关闭调度器
void schedule_free(schedule_t *);
// 新建协程
int coroutine_create(coroutine_t **, co_func, void *arg);
// 启动协程
int coroutine_resume(coroutine_t *);
// 将CPU的控制交还给调度器
int coroutine_yield(coroutine_t *);
// 释放协程
void coroutine_free(coroutine_t *);


#endif