.global coswapctx
#if !defined( __APPLE__ ) && !defined( __FreeBSD__ )
.type  coswapctx, @function
#endif

coswapctx:

#ifdef __i386__

    .text
    .p2align 2,,3
    movl 4(%esp), %eax
    movl %esp,  28(%eax)
    movl %ebp, 24(%eax)
    movl %esi, 20(%eax)
    movl %edi, 16(%eax)
    movl %edx, 12(%eax)
    movl %ecx, 8(%eax)
    movl %ebx, 4(%eax)

    movl 8(%esp), %eax
    movl 4(%eax), %ebx
    movl 8(%eax), %ecx
    movl 12(%eax), %edx
    movl 16(%eax), %edi
    movl 20(%eax), %esi
    movl 24(%eax), %ebp
    movl 28(%eax), %esp

    ret

#elif __x86_64__

//-------------
// 64 bit
// low | regs[0]: r15 |
//     | regs[1]: r14 |
//     | regs[2]: r13 |
//     | regs[3]: r12 |
//     | regs[4]: r9  |
//     | regs[5]: r8  |
//     | regs[6]: rbp |
//     | regs[7]: rdi |
//     | regs[8]: rsi |
//     | regs[9]: ret |  //ret func addr
//     | regs[10]: rdx |
//     | regs[11]: rcx |
//     | regs[12]: rbx |
// hig | regs[13]: rsp |

    .text
    .p2align 4,,15
    movq %rsp, 104(%rsi)        # very hard to understand
    movq %rbx, 96(%rsi)         
    movq %rcx, 88(%rsi)
    movq %rdx, 80(%rsi)
    movq (%rax), 72(%rsi)
    movq %rsi, 64(%rsi)
    movq %rdi, 56(%rsi)
    movq %rbp, 48(%rsi)
    movq %r8, 40(%rsi)
    movq %r9, 32(%rsi)
    movq %r12, 24(%rsi)
    movq %r13, 16(%rsi)
    movq %r14, 8(%rsi)
    movq %r15, 0(%rsi)
    # clear rax to indicate success
    xorq %rax, %rax

    movq 48(%rsi), %rbp
    movq 104(%rsi), %rsp
    movq (%rsi), %r15
    movq 8(%rsi), %r14
    movq 16(%rsi), %r13
    movq 24(%rsi), %r12
    movq 32(%rsi), %r9
    movq 40(%rsi), %r8
    movq 56(%rsi), %rdi
    movq 80(%rsi), %rdx
    movq 88(%rsi), %rcx
    movq 96(%rsi), %rbx
	leaq 8(%rsp), %rsp
	pushq 72(%rsi)
    movq 64(%rsi), %rsi

    ret

#endif