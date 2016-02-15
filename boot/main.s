	.file	"main.c"
	.text
	.globl	bootmain
	.type	bootmain, @function
bootmain:
.LFB33:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	$0, %edx
	movl	$4096, %esi
	movl	$65536, %edi
	call	readseg
	movl	$65536, %eax
	movl	(%rax), %eax
	cmpl	$1179403647, %eax
	je	.L2
	jmp	.L3
.L2:
	movl	$65536, %eax
	movl	28(%rax), %eax
	movl	%eax, %eax
	addq	$65536, %rax
	movq	%rax, -16(%rbp)
	movl	$65536, %eax
	movzwl	44(%rax), %eax
	movzwl	%ax, %eax
	salq	$5, %rax
	movq	%rax, %rdx
	movq	-16(%rbp), %rax
	addq	%rdx, %rax
	movq	%rax, -8(%rbp)
	jmp	.L4
.L5:
	movq	-16(%rbp), %rax
	movl	4(%rax), %edx
	movq	-16(%rbp), %rax
	movl	20(%rax), %ecx
	movq	-16(%rbp), %rax
	movl	12(%rax), %eax
	movl	%ecx, %esi
	movl	%eax, %edi
	call	readseg
	addq	$32, -16(%rbp)
.L4:
	movq	-16(%rbp), %rax
	cmpq	-8(%rbp), %rax
	jb	.L5
	movl	$65536, %eax
	movl	24(%rax), %eax
	movl	%eax, %eax
	call	*%rax
.L3:
	movl	$35328, -24(%rbp)
	movw	$-30208, -28(%rbp)
	movzwl	-28(%rbp), %eax
	movl	-24(%rbp), %edx
#APP
# 113 "../inc/x86.h" 1
	outw %ax,%dx
# 0 "" 2
#NO_APP
	movl	$35328, -20(%rbp)
	movw	$-29184, -26(%rbp)
	movzwl	-26(%rbp), %eax
	movl	-20(%rbp), %edx
#APP
# 113 "../inc/x86.h" 1
	outw %ax,%dx
# 0 "" 2
#NO_APP
.L6:
	jmp	.L6
	.cfi_endproc
.LFE33:
	.size	bootmain, .-bootmain
	.globl	readseg
	.type	readseg, @function
readseg:
.LFB34:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movl	%esi, -24(%rbp)
	movl	%edx, -28(%rbp)
	movl	-20(%rbp), %edx
	movl	-24(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -4(%rbp)
	andl	$-512, -20(%rbp)
	movl	-28(%rbp), %eax
	shrl	$9, %eax
	addl	$1, %eax
	movl	%eax, -28(%rbp)
	jmp	.L8
.L9:
	movl	-20(%rbp), %eax
	movq	%rax, %rdx
	movl	-28(%rbp), %eax
	movl	%eax, %esi
	movq	%rdx, %rdi
	call	readsect
	addl	$512, -20(%rbp)
	addl	$1, -28(%rbp)
.L8:
	movl	-20(%rbp), %eax
	cmpl	-4(%rbp), %eax
	jb	.L9
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE34:
	.size	readseg, .-readseg
	.globl	waitdisk
	.type	waitdisk, @function
waitdisk:
.LFB35:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	nop
.L12:
	movl	$503, -4(%rbp)
	movl	-4(%rbp), %eax
	movl	%eax, %edx
#APP
# 48 "../inc/x86.h" 1
	inb %dx,%al
# 0 "" 2
#NO_APP
	movb	%al, -5(%rbp)
	movzbl	-5(%rbp), %eax
	movzbl	%al, %eax
	andl	$192, %eax
	cmpl	$64, %eax
	jne	.L12
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE35:
	.size	waitdisk, .-waitdisk
	.globl	readsect
	.type	readsect, @function
readsect:
.LFB36:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$64, %rsp
	movq	%rdi, -56(%rbp)
	movl	%esi, -60(%rbp)
	call	waitdisk
	movl	$498, -40(%rbp)
	movb	$1, -46(%rbp)
	movzbl	-46(%rbp), %eax
	movl	-40(%rbp), %edx
#APP
# 98 "../inc/x86.h" 1
	outb %al,%dx
# 0 "" 2
#NO_APP
	movl	-60(%rbp), %eax
	movzbl	%al, %eax
	movl	$499, -36(%rbp)
	movb	%al, -45(%rbp)
	movzbl	-45(%rbp), %eax
	movl	-36(%rbp), %edx
#APP
# 98 "../inc/x86.h" 1
	outb %al,%dx
# 0 "" 2
#NO_APP
	movl	-60(%rbp), %eax
	shrl	$8, %eax
	movzbl	%al, %eax
	movl	$500, -32(%rbp)
	movb	%al, -44(%rbp)
	movzbl	-44(%rbp), %eax
	movl	-32(%rbp), %edx
#APP
# 98 "../inc/x86.h" 1
	outb %al,%dx
# 0 "" 2
#NO_APP
	movl	-60(%rbp), %eax
	shrl	$16, %eax
	movzbl	%al, %eax
	movl	$501, -28(%rbp)
	movb	%al, -43(%rbp)
	movzbl	-43(%rbp), %eax
	movl	-28(%rbp), %edx
#APP
# 98 "../inc/x86.h" 1
	outb %al,%dx
# 0 "" 2
#NO_APP
	movl	-60(%rbp), %eax
	shrl	$24, %eax
	orl	$-32, %eax
	movzbl	%al, %eax
	movl	$502, -24(%rbp)
	movb	%al, -42(%rbp)
	movzbl	-42(%rbp), %eax
	movl	-24(%rbp), %edx
#APP
# 98 "../inc/x86.h" 1
	outb %al,%dx
# 0 "" 2
#NO_APP
	movl	$503, -20(%rbp)
	movb	$32, -41(%rbp)
	movzbl	-41(%rbp), %eax
	movl	-20(%rbp), %edx
#APP
# 98 "../inc/x86.h" 1
	outb %al,%dx
# 0 "" 2
#NO_APP
	call	waitdisk
	movl	$496, -16(%rbp)
	movq	-56(%rbp), %rax
	movq	%rax, -8(%rbp)
	movl	$128, -12(%rbp)
	movl	-16(%rbp), %edx
	movq	-8(%rbp), %rcx
	movl	-12(%rbp), %eax
	movq	%rcx, %rsi
	movq	%rsi, %rdi
	movl	%eax, %ecx
#APP
# 89 "../inc/x86.h" 1
	cld
	repne
	insl
# 0 "" 2
#NO_APP
	movl	%ecx, %eax
	movq	%rdi, %rsi
	movq	%rsi, -8(%rbp)
	movl	%eax, -12(%rbp)
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE36:
	.size	readsect, .-readsect
	.ident	"GCC: (Ubuntu 4.9.2-10ubuntu13) 4.9.2"
	.section	.note.GNU-stack,"",@progbits
