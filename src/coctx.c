#include "coctx.h"
#include "coroutine.h"
#include "co_queue.h"

static void _exec(void *lt) {
#if defined(__lvm__) && defined(__x86_64__)
	__asm__("movq 16(%%rbp), %[lt]" : [lt] "=r" (lt));
#endif

    coroutine_t *co = (coroutine_t *)lt;
    co->func(co->arg);
    // 协程执行完毕
    co->status = CO_STATUS_DEAD;
    // 加入到dead队列
    QUE_INSERT(co->sched->dead, co);
    // 切回调度器
    coroutine_yield(co);
}

int comakectx(coroutine_t *co) {
    char *sp = co->stack + co->stack_size;
    // 16位对齐
    sp = (char *)((unsigned long)sp & -16LL);
    bzero(&co->ctx, sizeof(co->ctx));

    // 定义函数的入口地址
    void **entry = (void **)sp;
    *entry = (void *)_exec;

    // 保存函数入口地址
    co->ctx.rip = entry;
    // 函数参数
    co->ctx.rdi = (void *)co;
    // 栈指针
    co->ctx.rsp = sp;

    return 0;
}