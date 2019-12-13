#ifndef _COCTX_H
#define _COCTX_H

#include <stdlib.h>
#include <stdint.h>

typedef struct {
#if defined(__x86_64__)
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
    // void *esp; //
	// void *ebp;
	// void *eip;
	// void *edi;
	// void *esi;
	// void *ebx;
	// void *r1;
	// void *r2;
	// void *r3;
	// void *r4;
	// void *r5;

#elif defined(__i386__)
    void *eip
    void *ebx
    void *ecx
    void *edx
    void *edi
    void *esi
    void *ebp
    void *eax

#endif

} coctx_t;

struct coroutine;
typedef struct coroutine coroutine_t;

int coswapctx(coctx_t *new_ctx, coctx_t *cur_ctx);
int comakectx(coroutine_t *co);

#endif