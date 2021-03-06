#ifndef _COROUTINE_QUEUE_H
#define _COROUTINE_QUEUE_H

struct coroutine;
typedef struct coroutine coroutine_t;

typedef struct QueNode {
    coroutine_t *co;
    struct QueNode *next;

}QueNode_t;

typedef struct Queue {
    QueNode_t *head;
    QueNode_t *tail;
    
}Queue_t;

// 添加队列
void QUE_INSERT(Queue_t *, coroutine_t *);
// 队列是否为空
int QUE_EMPTY(Queue_t *);
// 队列删除
void QUE_REMOVE(Queue_t *);
// 队首元素
coroutine_t* QUE_FIRST(Queue_t *);
// 队列初始化
void QUE_INIT(Queue_t **);

#endif