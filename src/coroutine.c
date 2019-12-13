#include "coroutine.h"

pthread_key_t global_sched_key;
static pthread_once_t sched_key_once = PTHREAD_ONCE_INIT;

static void coroutine_sched_key_destructor(void *data) {
	free(data);
}

static void coroutine_sched_key_creator(void) {
	assert(pthread_key_create(&global_sched_key, coroutine_sched_key_destructor) == 0);
	assert(pthread_setspecific(global_sched_key, NULL) == 0);

	return;
}

int coroutine_create(coroutine_t **new_co, co_func fn, void *arg) {

    assert(pthread_once(&sched_key_once, coroutine_sched_key_creator) == 0);
    schedule_t *sched = get_schedule();

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
    printf("create---co's address: %p, sched's address: %p\n", co, sched);
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
    if(sched->current != NULL)
    printf("current---co's address: %p, sched's address: %p\n", sched->current, sched->current->sched);
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
    printf("resume---co's address: %p, sched's address: %p\n", co, co->sched);
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