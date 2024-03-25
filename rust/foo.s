	.text
	.def	@feat.00;
	.scl	3;
	.type	0;
	.endef
	.globl	@feat.00
.set @feat.00, 1
	.file	"foo.c5d6f560124b7ff1-cgu.0"
	.def	__ZN3foo3foo17h28e012b778f00b9eE;
	.scl	2;
	.type	32;
	.endef
	.globl	__ZN3foo3foo17h28e012b778f00b9eE
	.p2align	4, 0x90
__ZN3foo3foo17h28e012b778f00b9eE:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset %ebp, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register %ebp
	subl	$24, %esp
	leal	-24(%ebp), %eax
	movl	$L___unnamed_1, -24(%ebp)
	movl	$1, -20(%ebp)
	movl	$0, -8(%ebp)
	movl	$L___unnamed_2, -16(%ebp)
	movl	$0, -12(%ebp)
	pushl	%eax
	calll	__ZN3std2io5stdio6_print17h06e585553ed71412E
	addl	$28, %esp
	popl	%ebp
	retl
	.cfi_endproc

	.section	.rdata,"dr"
	.p2align	2, 0x0
L___unnamed_2:

L___unnamed_3:
	.ascii	"hello, world from foo\n"

	.p2align	2, 0x0
L___unnamed_1:
	.long	L___unnamed_3
	.asciz	"\026\000\000"

