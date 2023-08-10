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


# Three parameters are:
# 1. Function to call.
# 2. First parameter to that function
# 3. Second parmater to that function

# It is intended that this should be generalized into a
# function that takes a count of number of parameters and
# then the function and parameters

.globl	_puts
_puts:
	sub	rsp, 40
	mov	rcx, 48[rsp]
	call	puts
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


