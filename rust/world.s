	.text
	.def	@feat.00;
	.scl	3;
	.type	0;
	.endef
	.globl	@feat.00
.set @feat.00, 1
	.file	"world.ba7340611aee4726-cgu.0"
	.def	__ZN3std10sys_common9backtrace28__rust_begin_short_backtrace17h745d6abc782b1824E;
	.scl	3;
	.type	32;
	.endef
	.p2align	4, 0x90
__ZN3std10sys_common9backtrace28__rust_begin_short_backtrace17h745d6abc782b1824E:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset %ebp, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register %ebp
	calll	*%ecx
	#APP
	#NO_APP
	popl	%ebp
	retl
	.cfi_endproc

	.def	__ZN3std2rt10lang_start17hda88e62891e90dd1E;
	.scl	2;
	.type	32;
	.endef
	.globl	__ZN3std2rt10lang_start17hda88e62891e90dd1E
	.p2align	4, 0x90
__ZN3std2rt10lang_start17hda88e62891e90dd1E:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset %ebp, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register %ebp
	pushl	%esi
	pushl	%eax
	.cfi_offset %esi, -12
	movl	8(%ebp), %edx
	movl	12(%ebp), %eax
	movl	16(%ebp), %ecx
	leal	-8(%ebp), %esi
	movl	%edx, -8(%ebp)
	movl	20(%ebp), %edx
	pushl	%edx
	pushl	%ecx
	pushl	%eax
	pushl	$L___unnamed_1
	pushl	%esi
	calll	__ZN3std2rt19lang_start_internal17hfefa8592b280fcd4E
	addl	$24, %esp
	popl	%esi
	popl	%ebp
	retl
	.cfi_endproc

	.def	__ZN3std2rt10lang_start28_$u7b$$u7b$closure$u7d$$u7d$17hfbebd4d1dbc367daE;
	.scl	3;
	.type	32;
	.endef
	.p2align	4, 0x90
__ZN3std2rt10lang_start28_$u7b$$u7b$closure$u7d$$u7d$17hfbebd4d1dbc367daE:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset %ebp, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register %ebp
	movl	8(%ebp), %eax
	movl	(%eax), %ecx
	calll	__ZN3std10sys_common9backtrace28__rust_begin_short_backtrace17h745d6abc782b1824E
	xorl	%eax, %eax
	popl	%ebp
	retl
	.cfi_endproc

	.def	__ZN4core3ops8function6FnOnce40call_once$u7b$$u7b$vtable.shim$u7d$$u7d$17h9ff0ae12a260ad5eE;
	.scl	3;
	.type	32;
	.endef
	.p2align	4, 0x90
__ZN4core3ops8function6FnOnce40call_once$u7b$$u7b$vtable.shim$u7d$$u7d$17h9ff0ae12a260ad5eE:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset %ebp, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register %ebp
	movl	8(%ebp), %eax
	movl	(%eax), %ecx
	calll	__ZN3std10sys_common9backtrace28__rust_begin_short_backtrace17h745d6abc782b1824E
	xorl	%eax, %eax
	popl	%ebp
	retl
	.cfi_endproc

	.def	__ZN4core3ptr85drop_in_place$LT$std..rt..lang_start$LT$$LP$$RP$$GT$..$u7b$$u7b$closure$u7d$$u7d$$GT$17hfcb3696546bba262E;
	.scl	3;
	.type	32;
	.endef
	.p2align	4, 0x90
__ZN4core3ptr85drop_in_place$LT$std..rt..lang_start$LT$$LP$$RP$$GT$..$u7b$$u7b$closure$u7d$$u7d$$GT$17hfcb3696546bba262E:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset %ebp, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register %ebp
	popl	%ebp
	retl
	.cfi_endproc

	.def	__ZN5world4main17hcbe94aecf2a4248fE;
	.scl	3;
	.type	32;
	.endef
	.p2align	4, 0x90
__ZN5world4main17hcbe94aecf2a4248fE:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset %ebp, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register %ebp
	subl	$24, %esp
	leal	-24(%ebp), %eax
	movl	$L___unnamed_2, -24(%ebp)
	movl	$1, -20(%ebp)
	movl	$0, -8(%ebp)
	movl	$L___unnamed_3, -16(%ebp)
	movl	$0, -12(%ebp)
	pushl	%eax
	calll	__ZN3std2io5stdio6_print17h06e585553ed71412E
	addl	$28, %esp
	popl	%ebp
	jmp	__ZN3foo3foo17h28e012b778f00b9eE
	.cfi_endproc

	.def	_main;
	.scl	2;
	.type	32;
	.endef
	.globl	_main
	.p2align	4, 0x90
_main:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset %ebp, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register %ebp
	pushl	%edi
	pushl	%esi
	pushl	%eax
	.cfi_offset %esi, -16
	.cfi_offset %edi, -12
	movl	8(%ebp), %esi
	movl	12(%ebp), %edi
	calll	___main
	movl	$__ZN5world4main17hcbe94aecf2a4248fE, -12(%ebp)
	leal	-12(%ebp), %eax
	pushl	$0
	pushl	%edi
	pushl	%esi
	pushl	$L___unnamed_1
	pushl	%eax
	calll	__ZN3std2rt19lang_start_internal17hfefa8592b280fcd4E
	addl	$24, %esp
	popl	%esi
	popl	%edi
	popl	%ebp
	retl
	.cfi_endproc

	.section	.rdata,"dr"
	.p2align	2, 0x0
L___unnamed_1:
	.long	__ZN4core3ptr85drop_in_place$LT$std..rt..lang_start$LT$$LP$$RP$$GT$..$u7b$$u7b$closure$u7d$$u7d$$GT$17hfcb3696546bba262E
	.asciz	"\004\000\000\000\004\000\000"
	.long	__ZN4core3ops8function6FnOnce40call_once$u7b$$u7b$vtable.shim$u7d$$u7d$17h9ff0ae12a260ad5eE
	.long	__ZN3std2rt10lang_start28_$u7b$$u7b$closure$u7d$$u7d$17hfbebd4d1dbc367daE
	.long	__ZN3std2rt10lang_start28_$u7b$$u7b$closure$u7d$$u7d$17hfbebd4d1dbc367daE

	.p2align	2, 0x0
L___unnamed_3:

L___unnamed_4:
	.ascii	"hello, world from rust\n"

	.p2align	2, 0x0
L___unnamed_2:
	.long	L___unnamed_4
	.asciz	"\027\000\000"

	.section	.drectve,"yni"
	.ascii	" -exclude-symbols:_ZN3std2rt10lang_start17hda88e62891e90dd1E"
