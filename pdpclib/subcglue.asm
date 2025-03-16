# Written by Paul Edwards
# Released to the public domain

# Windows calling convention passes the first 4
# parameters in rcx, rdx, r8 and r9. The rest are
# put on the stack.

# SubC puts everything on the stack.

# glue code for Win64

# Not sure if we need the subtraction of 40 from rsp, but
# that's what mingw64 generates.



.code64
.intel_syntax noprefix

.text


.globl	_puts
_puts:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	call	puts
	add	rsp, 40
	ret

.globl	___iob_func
___iob_func:
	sub	rsp, 40
	call	__iob_func
	add	rsp, 40
	ret

.globl	_malloc
_malloc:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	call	malloc
	add	rsp, 40
	ret

.globl	_ferror
_ferror:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	call	ferror
	add	rsp, 40
	ret

.globl	_fclose
_fclose:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	call	fclose
	add	rsp, 40
	ret

.globl	_strlen
_strlen:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	call	strlen
	add	rsp, 40
	ret

.globl	_strcpy
_strcpy:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	mov	rdx, 56[rsp]
	call	strcpy
	add	rsp, 40
	ret

.globl	_strcmp
_strcmp:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	mov	rdx, 56[rsp]
	call	strcmp
	add	rsp, 40
	ret

.globl	_fopen
_fopen:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	mov	rdx, 56[rsp]
	call	fopen
	add	rsp, 40
	ret

.globl	_fread
_fread:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	mov	rdx, 56[rsp]
	mov	r8, 64[rsp]
	mov	r9, 72[rsp]
	call	fread
	add	rsp, 40
	ret

.globl	_fwrite
_fwrite:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	mov	rdx, 56[rsp]
	mov	r8, 64[rsp]
	mov	r9, 72[rsp]
	call	fwrite
	add	rsp, 40
	ret

# Not sure why the 5th parameter goes into the top of the
# special 40-byte stack area. Maybe this is reserved for
# 5 more parameters
.globl	___getmainargs
___getmainargs:
	sub	rsp, 40
        mov     rcx, 80[rsp]
        movq    32[rsp], rcx
	mov	rcx, 48[rsp]
	mov	rdx, 56[rsp]
	mov	r8, 64[rsp]
	mov	r9, 72[rsp]
	call	__getmainargs
	add	rsp, 40
	ret

# We only have a maximum of 4 parameters supported at the
# moment. And theoretically this could access memory beyond
# what is allocated
.globl	_printf
_printf:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	mov	rdx, 56[rsp]
	mov	r8, 64[rsp]
	mov	r9, 72[rsp]
	call	printf
	add	rsp, 40
	ret

# This conflicts with another function called _exit,
# which means we can't currently exit properly - an
# attempt to exit will invoke the quick exit in start.c
# and give a random return code
#.globl	_exit
#_exit:
#	sub	rsp, 40
#	mov	rcx, 48[rsp]
#	call	exit
#	add	rsp, 40
#	ret

# But this likely works
.globl	___scexit
___scexit:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	call	exit
	add	rsp, 40
	ret
