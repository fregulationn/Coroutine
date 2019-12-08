#include "coroutine.h"

int coroutine_create(coroutine_t **new_co, co_func fn, void *arg) {
    
    schedule_t *sched = get_sched();

    if(sched == NULL) {
        schedule_create(0);

        sched = get_schedule();
        if(sched == NULL) {
            printf("Fail to create scheduler\n");
            return -1;
        }
    }

    coroutine_t *co = (coroutine_t *)malloc(sizeof(coroutine_t));
    if(co == NULL) {
        printf("Fail to create new coroutine\n");
        return -1;
    }

    int ret = posix_memalign(&co->stack, getpagesize(), sched->stack_size);
	if (ret) {
		printf("Failed to allocate stack for new coroutine\n");
		free(co);
		return -1;
	}

    co->sched = sched;
    co->status = CO_STATUS_NEW;
    co->func = fn;
    co->arg = arg;
    co->stack_size = sched->stack_size;
    co->fd = -1;
    co->events = 0;
    co->birth = coroutine_usec_now();

    *new_co = co;
    
    // add to ready queue
    QUE_INSERT(sched->ready, co);

    return 0;
}

int coroutine_yield(coroutine_t *co) {

    coswapctx(&co->sched->ctx, &co->ctx);

    return 0;
}

int coroutine_resume(coroutine_t *co) {
    if(co->status == CO_STATUS_NEW) {
        // 新创建的协程，需要先创建环境
        comakectx(co);
    }

    schedule_t *sched = get_schedule();
    sched->current = co;
    // swap context
    coswapctx(&co->ctx, &sched->ctx);
    // when come back
    sched->current = NULL;

    return 0;
}

void coroutine_free(coroutine_t *co) {
    if(co == NULL) return;

    if(co->stack) {
        free(co->stack);
        co->stack = NULL;
    }

    if(co) {
        free(co);
    }

}