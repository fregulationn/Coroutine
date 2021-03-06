#include <errno.h>
#include "coroutine.h"
#include "co_queue.h"
#include "co_tree.h"

int schedule_create(int stack_size) {

    int sched_stack_size = stack_size ? stack_size : CO_MAX_STACKSIZE;
    
    schedule_t *sched = (schedule_t *)malloc(sizeof(schedule_t));
    if(sched == NULL) {
        printf("malloc fail!\n");
        return -1;
    }

    assert(pthread_setspecific(global_sched_key, sched) == 0);

    sched->epollfd = coroutine_epoll_create();
    if(sched->epollfd == -1) {
        printf("Fail to create epoll\n");
        return -1;
    }

    sched->stack_size = sched_stack_size;
    sched->page_size = getpagesize();

    sched->num_new_events = 0;
    sched->current = NULL;
    sched->birth = coroutine_usec_now();
    sched->spawned_coroutines = 0;

    // create ready queue
    QUE_INIT(&sched->ready);
    // create dead queue
    QUE_INIT(&sched->dead);

#if (SCHED_HASH == 1)
    // create schedule table
    init_schedtab(&sched->table);
#else
    // create wait tree
    sched->wait_tree = create_rbtree();
#endif

    bzero(&sched->ctx, sizeof(sched->ctx));

    return 0;
}


void schedule_free(schedule_t *sched) {
    if(sched->epollfd > 0) {
        close(sched->epollfd);
    }
    
    free(sched);

    assert(pthread_setspecific(global_sched_key, NULL) == 0);
}


void schedule_running() {

    schedule_t *sched = get_schedule();
    if(sched == NULL) return;

    struct epoll_event ev;
    coroutine_t *co;
    int nready, i;
    int fd;
    uint32_t __events;

    while(1)
    {
        while(!QUE_EMPTY(sched->dead)) 
        {
            // 如果dead队列非空，说明有协程已经运行结束
            co = QUE_FIRST(sched->dead);
            // 从dead队列移除
            QUE_REMOVE(sched->dead);
            // 销毁
            coroutine_free(co);
        }

        while(!QUE_EMPTY(sched->ready)) 
        {
            // 如果ready队列非空，说明有加入的新协程
            co = QUE_FIRST(sched->ready);
            // 从ready队列移除
            QUE_REMOVE(sched->ready);
            // 运行新加入的协程
            coroutine_resume(co);
        }
        
        nready = coroutine_epoll_wait();
        for(i = 0; i < nready; i++) {
            ev = sched->eventlist[i];
            fd = ev.data.fd;
            __events = ev.events;

            if(__events & EPOLLHUP) {
                printf("Peer reset connection\n");

#if (SCHED_HASH == 1)
                // delete from sched table
                delfromTable(&sched->table, fd);
#else
                rbtree_delete(sched->wait_tree, fd);
#endif

                coroutine_epoll_ctl(EPOLL_CTL_DEL, fd, EPOLLHUP);
                // close this fd
                close(fd);  
                continue;
            }

#if (SCHED_HASH == 1)
            // search in sched table
            co = co_search(&sched->table, fd);
#else
            // 通过fd获取wait tree上的协程
            Node* ret = rbtree_search(sched->wait_tree, fd);
            co = ret->co;
#endif

            if(co != NULL) {
                // resume co
                coroutine_resume(co);
            }
        }

    }
    
}
