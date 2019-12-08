#ifndef _COCTX_H
#define _COCTX_H

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    void *r15;
    void *r14;
    void *r13;
    void *r12;
    void *r9;
    void *r8;
    void *rbp;
    void *rdi;
    void *rsi;
    void *rip;
    void *rdx;
    void *rcx;
    void *rbx;
    void *rsp;
} coctx_t;

struct coroutine;
typedef struct coroutine coroutine_t;

int coswapctx(coctx_t *new_ctx, coctx_t *cur_ctx);
int comakectx(coroutine_t *co);

#endif