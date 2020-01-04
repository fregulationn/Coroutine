.global coswapctx
#if !defined( __APPLE__ ) && !defined( __FreeBSD__ )
.type  coswapctx, @function
#endif

coswapctx:

#if defined(__i386__)

//----- --------
// 32 bit
// | regs[0]: ret |
// | regs[1]: ebx |
// | regs[2]: ecx |
// | regs[3]: edx |
// | regs[4]: edi |
// | regs[5]: esi |
// | regs[6]: ebp |
// | regs[7]: eax |  = esp

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

#elif defined(__x86_64__)

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
    leaq (%rsp),%rax
    movq %rax, 104(%rsi)
    movq %rbx, 96(%rsi)         
    movq %rcx, 88(%rsi)
    movq %rdx, 80(%rsi)
    movq (%rax), %rax
    movq %rax, 72(%rsi)
    movq %rbp, 48(%rsi)
    movq %r8, 40(%rsi)
    movq %r9, 32(%rsi)
    movq %r12, 24(%rsi)
    movq %r13, 16(%rsi)
    movq %r14, 8(%rsi)
    movq %r15, 0(%rsi)
    # clear rax to indicate success
    xorq %rax, %rax

    movq 48(%rdi), %rbp
    movq 104(%rdi), %rsp
    movq (%rdi), %r15
    movq 8(%rdi), %r14
    movq 16(%rdi), %r13
    movq 24(%rdi), %r12
    movq 32(%rdi), %r9
    movq 40(%rdi), %r8
    movq 80(%rdi), %rdx
    movq 88(%rdi), %rcx
    movq 96(%rdi), %rbx
	leaq 8(%rsp), %rsp    //rsp是当前栈顶，8(%rsp)是跳过rip，指向调用coswapctx函数之前的栈顶
	pushq 72(%rdi)

    ret

#endif