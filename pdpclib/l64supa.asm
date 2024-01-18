# l64supa.asm - support code for C programs for x64 Linux
#
# This program written by Paul Edwards
# Released to the public domain


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

.if STACKPARM
push %rbp
mov %rsp, %rbp
push %rdi
push %rsi
push %rdx
.endif

# function code 1 = write
movq $1, %rax

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


.globl ___read
___read:
.globl __read
__read:
push %rbp
mov %rsp, %rbp
push %rbx
push %rcx
push %rdx

# function code 3 = read
movq $3, %rax
# handle
movq 8(%rbp), %rbx
# data pointer
movq 12(%rbp), %rcx
# length
movq 16(%rbp), %rdx
int $0x80
pop %rdx
pop %rcx
pop %rbx
pop %rbp
ret



.globl ___open
___open:
.globl __open
__open:
push %rbp
mov %rsp, %rbp
push %rbx
push %rcx
push %rdx

# function code 5 = open
movq $5, %rax
# filename
movq 8(%rbp), %rbx
# flag
movq 12(%rbp), %rcx
# mode
movq 16(%rbp), %rdx
int $0x80
pop %rdx
pop %rcx
pop %rbx
pop %rbp
ret



.globl __seek
__seek:
.globl ___seek
___seek:
push %rbp
mov %rsp, %rbp
push %rbx
push %rcx
push %rdx

# function code 19 = lseek
movq $19, %rax
# handle
movq 8(%rbp), %rbx
# offset
movq 12(%rbp), %rcx
# whence
movq 16(%rbp), %rdx
int $0x80
pop %rdx
pop %rcx
pop %rbx
pop %rbp
ret



.globl ___rename
___rename:
.globl __rename
__rename:
push %rbp
mov %rsp, %rbp
push %rbx
push %rcx

# function code 38 = rename
movq $38, %rax
# old file
movq 8(%rbp), %rbx
# new file
movq 12(%rbp), %rcx
int $0x80
pop %rcx
pop %rbx
pop %rbp


.globl ___remove
___remove:
.globl __remove
__remove:
push %rbp
mov %rsp, %rbp
push %rbx
# function code 10 = unlink
movq $10, %rax
# filename
movq 8(%rbp), %rbx
int $0x80
pop %rbx
pop %rbp
ret


.globl ___close
___close:
.globl __close
__close:
push %rbp
mov %rsp, %rbp
push %rbx
# function code 6 = close
movq $6, %rax
# handle
movq 8(%rbp), %rbx
int $0x80
pop %rbx
pop %rbp
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
push %rbp
mov %rsp, %rbp
push %rbx
# function code 13 = retrieve current time
movq $13, %rax
# pointer to time_t
movq 8(%rbp), %rbx
int $0x80
pop %rbx
pop %rbp
ret


.globl ___ioctl
___ioctl:
.globl __ioctl
__ioctl:
push %rbp
mov %rsp, %rbp
push %rbx
push %rcx
push %rdx
# function code 54 = ioctl
movq $54, %rax
# file descriptor
movq 8(%rbp), %rbx
# command
movq 12(%rbp), %rcx
# parameter
movq 16(%rbp), %rdx
int $0x80
pop %rdx
pop %rcx
pop %rbx
pop %rbp
ret


.globl ___chdir
___chdir:
.globl __chdir
__chdir:
push %rbp
mov %rsp, %rbp
push %rbx
# function code 12 = chdir
movq $12, %rax
# filename (directory name)
movq 8(%rbp), %rbx
int $0x80
pop %rbx
pop %rbp
ret


.globl ___rmdir
___rmdir:
.globl __rmdir
__rmdir:
push %rbp
mov %rsp, %rbp
push %rbx
# function code 40 = rmdir
movq $40, %rax
# pathname
movq 8(%rbp), %rbx
int $0x80
pop %rbx
pop %rbp
ret


.globl ___mkdir
___mkdir:
.globl __mkdir
__mkdir:
push %rbp
mov %rsp, %rbp
push %rbx
push %rcx
# function code 39 = mkdir
movq $39, %rax
# pathname
movq 8(%rbp), %rbx
# mode
movq 12(%rbp), %rcx
int $0x80
pop %rcx
pop %rbx
pop %rbp
ret


.globl ___getdents
___getdents:
.globl __getdents
__getdents:
push %rbp
mov %rsp, %rbp
push %rbx
push %rcx
push %rdx
# function code 141 = getdents
movq $141, %rax
# file descriptor
movq 8(%rbp), %rbx
# dirent
movq 12(%rbp), %rcx
# count
movq 16(%rbp), %rdx
int $0x80
pop %rdx
pop %rcx
pop %rbx
pop %rbp
ret


.globl ___mprotect
___mprotect:
.globl __mprotect
__mprotect:
push %rbp
mov %rsp, %rbp
push %rbx
push %rcx
push %rdx
# function code 125 = mprotect
movq $125, %rax
# start
movq 8(%rbp), %rbx
# len
movq 12(%rbp), %rcx
# prot
movq 16(%rbp), %rdx
int $0x80
pop %rdx
pop %rcx
pop %rbx
pop %rbp
ret
