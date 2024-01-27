# l64supb.asm - support code for C programs for x64 Linux
# For code compiled with the cc64 compiler that follows the
# Microsoft Win64 calling convention
#
# This program written by Paul Edwards
# Released to the public domain


.code64


.globl ___setj
___setj:
.globl __setj
__setj:
mov $0, %rax
ret

mov 4(%rsp), %rax
push %rbx
mov %rsp, %rbx
mov %rbx, 20(%rax) #esp

mov %rbp, %rbx
mov %rbx, 24(%rax)

mov %rcx, 4(%rax)
mov %rdx, 8(%rax)
mov %rdi, 12(%rax)
mov %esi, 16(%rax)

mov 4(%rsp), %rbx    # return address
mov %rbx, 28(%rax)   # return address

pop %rbx
mov %rbx,0(%rax)
mov $0, %rax

ret



.globl ___longj
___longj:
.globl __longj
__longj:
fred: jmp fred
mov $0, %rax
ret

mov 4(%rsp), %rax
mov 20(%rax), %rbp
mov %rbp, %rsp

pop %rbx            # position of old ebx
pop %rbx            # position of old return address

mov 28(%rax), %rbx  # return address
push %rbx

mov 24(%rax), %rbx
mov %rbx, %rbp

mov 0(%rax), %rbx
mov 4(%rax), %rcx
mov 8(%rax), %rdx
mov 12(%rax), %rdi
mov 16(%rax), %esi

mov 60(%rax), %rax    # return value

ret



.globl ___write
___write:
.globl __write
__write:

push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
push %rdx

# function code 1 = write
movq $1, %rax

# handle
xor %rdi,%rdi
mov %ecx, %edi
# data pointer
mov %rdx, %rsi
# length
xor %rdx,%rdx
mov %edx, %edx

syscall

pop %rdx
pop %rsi
pop %rdi
pop %rbp

ret



.globl ___read
___read:
.globl __read
__read:

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
push %rdx
.endif

# function code 0 = read
movq $0, %rax

.if STACKPARM
# handle
movq 16(%rbp), %rdi
# data pointer
movq 24(%rbp), %rsi
# length
movq 32(%rbp), %rdx
.endif

syscall

.if STACKPARM
pop %rdx
pop %rsi
pop %rdi
pop %rbp
.endif

ret




.globl ___open
___open:
.globl __open
__open:

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
push %rdx
.endif

# function code 2 = open
movq $2, %rax

.if STACKPARM
# filename
movq 16(%rbp), %rdi
# flag
movq 24(%rbp), %rsi
# mode
movq 32(%rbp), %rdx
.endif

syscall

.if STACKPARM
pop %rdx
pop %rsi
pop %rdi
pop %rbp
.endif

ret



.globl __seek
__seek:
.globl ___seek
___seek:

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
push %rdx
.endif

# function code 8 = lseek
movq $8, %rax

.if STACKPARM
# handle
movq 16(%rbp), %rdi
# offset
movq 24(%rbp), %rsi
# whence
movq 32(%rbp), %rdx
.endif

syscall

.if STACKPARM
pop %rdx
pop %rsi
pop %rdi
pop %rbp
.endif

ret



.globl ___rename
___rename:
.globl __rename
__rename:

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
.endif

# function code 82 = rename
movq $82, %rax

.if STACKPARM
# old file
movq 16(%rbp), %rdi
# new file
movq 24(%rbp), %rsi
.endif

syscall

.if STACKPARM
pop %rsi
pop %rdi
pop %rbp
.endif

ret


.globl ___remove
___remove:
.globl __remove
__remove:

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
.endif

# function code 87 = unlink
movq $87, %rax

.if STACKPARM
# filename
movq 16(%rbp), %rdi
.endif

syscall

.if STACKPARM
pop %rdi
pop %rbp
.endif

ret



.globl ___close
___close:
.globl __close
__close:

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
.endif

# function code 3 = close
movq $3, %rax

.if STACKPARM
# handle
movq 16(%rbp), %rdi
.endif

syscall

.if STACKPARM
pop %rdi
pop %rbp
.endif

ret




.globl ___exita
___exita:
.globl __exita
__exita:
# exit/terminate

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
movq 16(%rbp), %rdi
.endif

movq $60, %rax

syscall

.if STACKPARM
pop %rdi
pop %rbp
.endif

ret


.globl ___time
___time:
.globl __time
__time:

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
.endif

# function code 201 = retrieve current time
movq $201, %rax

.if STACKPARM
# pointer to time_t
movq 16(%rbp), %rdi
.endif

syscall

.if STACKPARM
pop %rdi
pop %rbp
.endif

ret




.globl ___ioctl
___ioctl:
.globl __ioctl
__ioctl:

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
push %rdx
.endif

# function code 16 = ioctl
movq $16, %rax

.if STACKPARM
# file descriptor
movq 16(%rbp), %rdi
# command
movq 24(%rbp), %rsi
# parameter
movq 32(%rbp), %rdx
.endif

syscall

.if STACKPARM
pop %rdx
pop %rsi
pop %rdi
pop %rbp
.endif

ret



.globl ___chdir
___chdir:
.globl __chdir
__chdir:

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
.endif

# function code 80 = chdir
movq $80, %rax

.if STACKPARM
# filename (directory name)
movq 16(%rbp), %rdi
.endif

syscall

.if STACKPARM
pop %rdi
pop %rbp
.endif

ret




.globl ___rmdir
___rmdir:
.globl __rmdir
__rmdir:
.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
.endif

# function code 84 = rmdir
movq $84, %rax

.if STACKPARM
# pathname
movq 8(%rbp), %rdi
.endif

syscall

.if STACKPARM
pop %rdi
pop %rbp
.endif

ret




.globl ___mkdir
___mkdir:
.globl __mkdir
__mkdir:

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
.endif

# function code 83 = mkdir
movq $83, %rax

.if STACKPARM
# pathname
movq 16(%rbp), %rdi
# mode
movq 24(%rbp), %rsi
.endif

syscall

.if STACKPARM
pop %rsi
pop %rdi
pop %rbp
.endif

ret




.globl ___getdents
___getdents:
.globl __getdents
__getdents:

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
push %rdx
.endif

# function code 78 = getdents
movq $78, %rax

.if STACKPARM
# file descriptor
movq 16(%rbp), %rdi
# dirent
movq 24(%rbp), %rsi
# count
movq 32(%rbp), %rdx
.endif

syscall

.if STACKPARM
pop %rdx
pop %rsi
pop %rdi
pop %rbp
.endif

ret




.globl ___mprotect
___mprotect:
.globl __mprotect
__mprotect:

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
push %rdx
.endif

# function code 10 = mprotect
movq $10, %rax

.if STACKPARM
# start
movq 16(%rbp), %rdi
# len
movq 24(%rbp), %rsi
# prot
movq 32(%rbp), %rdx
.endif

syscall

.if STACKPARM
pop %rdx
pop %rsi
pop %rdi
pop %rbp
.endif

ret
