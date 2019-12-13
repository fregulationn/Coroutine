#include "coroutine.h"
#include "co_queue.h"


void QUE_INIT(Queue_t **Q) {
    *Q = (Queue_t *)malloc(sizeof(Queue_t));
    (*Q)->head = NULL;
    (*Q)->tail = NULL;

}

void QUE_INSERT(Queue_t *Q, coroutine_t *co) {
    // 队列为空
    if(Q->head == NULL) {
        Q->head = (QueNode_t *)malloc(sizeof(QueNode_t));
        Q->tail = Q->head;
    } else {
        Q->tail->next = (QueNode_t *)malloc(sizeof(QueNode_t));
        Q->tail = Q->tail->next;
    }
    Q->tail->co = co;
    Q->tail->next = NULL;
}

int QUE_EMPTY(Queue_t *Q) {
    return Q->head == NULL;
}

coroutine_t* QUE_FIRST(Queue_t *Q) {
    return Q->head->co;
}

void QUE_REMOVE(Queue_t *Q) {
    QueNode_t *tmp = Q->head;
    if(Q->head == Q->tail) {
        Q->head = Q->tail = NULL;
    } else {
        Q->head = Q->head->next;
    }
    free(tmp);
    
}