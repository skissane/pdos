# This code was taken from the public domain SubC
# Modified by Paul Edwards
# All changes remain public domain

# Calling conventions: r0,r1,r2,stack, return in r0
#                      64-bit values in r0/r1, r2/r3, never in r1/r2
#                      (observe register alignment!)
# System call: r7=call#, arguments r0,r1,r2,r3,r4,r5
#              carry indicates error,
#              return/error value in r0

# https://github.com/aligrudi/neatlibc/blob/master/arm/syscall.s
# https://gist.github.com/yamnikov-oleg/454f48c3c45b735631f2
# https://syscalls.w3challs.com/?arch=arm_strong

        .data
        .align  2
# int setjmp(jmp_buf env);

        .globl  __setjmp
        .align  2
__setjmp:
        ldr     r1,[sp]         @ env
        mov     r2,sp
        add     r2,r2,#4
        str     sp,[r1]
        str     r11,[r1,#4]
        str     lr,[r1,#8]
        mov     r0,#0
        mov     pc,lr

# void longjmp(jmp_buf env, int v);

        .globl  __longjmp
        .align  2
__longjmp:
        ldr     r0,[sp,#4]      @ v
        cmp     r0,#0
        moveq   r0,#1
        ldr     r1,[sp]         @ env
        ldr     sp,[r1]
        ldr     r11,[r1,#4]
        ldr     lr,[r1,#8]
        mov     pc,lr

# void _exita(int rc);

        .globl  __exita
        .align  2
__exita:
        stmfd   sp!,{lr}
        ldr     r0,[sp,#4]      @ rc
        mov     r7,#1           @ SYS_exit
        swi     0
        ldmia   sp!,{pc}

# int __write(int fd, void *buf, int len);

        .globl  __write
        .align  2
__write:
        stmfd   sp!,{lr}
        ldr     r2,[sp,#12]     @ len
        ldr     r1,[sp,#8]      @ buf
        ldr     r0,[sp,#4]      @ fd
        mov     r7,#4           @ SYS_write
        swi     0
wrtok:  ldmia   sp!,{pc}

# int __read(int fd, void *buf, int len);

        .globl  __read
        .align  2
__read:
        stmfd   sp!,{lr}
        ldr     r2,[sp,#12]     @ len
        ldr     r1,[sp,#8]      @ buf
        ldr     r0,[sp,#4]      @ fd
        mov     r7,#3           @ SYS_read
        swi     0
redok:  ldmia   sp!,{pc}
# int __seek(int fd, int pos, int how);

        .globl  __lseek
        .align  2
__lseek:
        stmfd   sp!,{lr}
        ldr     r2,[sp,#12]     @ how
        ldr     r1,[sp,#8]      @ off_t
        ldr     r0,[sp,#4]      @ fd
        mov     r7,#19
        swi     0
lskok:  
        ldmia   sp!,{pc}

# int __creat(char *path, int mode);

        .globl  __creat
        .align  2
__creat:
        stmfd   sp!,{lr}
        ldr     r1,[sp,#8]      @ mode
        ldr     r0,[sp,#4]      @ path
        mov     r7,#8           @ SYS_creat
        swi     0
crtok:  ldmia   sp!,{pc}

# int _open(char *path, int flags);

        .globl  __open
        .align  2
__open:
        stmfd   sp!,{lr}
        mov     r2,#0x1A4       @ 0644
        ldr     r1,[sp,#8]      @ flags
        ldr     r0,[sp,#4]      @ path
        mov     r7,#5           @ SYS_open
        swi     0
opnok:  ldmia   sp!,{pc}

# int _close(int fd);

        .globl  __close
        .align  2
__close:
        stmfd   sp!,{lr}
        ldr     r0,[sp,#4]      @ fd
        mov     r7,#6           @ SYS_close
        swi     0
clsok:  ldmia   sp!,{pc}

# int __remove(char *path);

        .globl  __remove
        .align  2
__remove:
        stmfd   sp!,{lr}
        ldr     r0,[sp,#4]      @ path
        mov     r7,#10          @ SYS_unlink
        swi     0
unlok:  ldmia   sp!,{pc}

# int __rename(char *old, char *new);

        .globl  __rename
        .align  2
__rename:
        stmfd   sp!,{lr}
        ldr     r1,[sp,#8]      @ new
        ldr     r0,[sp,#4]      @ old
        mov     r7,#0x26        @ SYS_rename
        swi     0
renok:  ldmia   sp!,{pc}

# int __time(void);

        .globl  __time
        .align  2
__time:
        stmfd   sp!,{lr}
        sub     sp,sp,#16       @ struct timespec
        mov     r1,sp
        mov     r0,#0           @ CLOCK_REALTIME
        ldr     r7,=0x107       @ SYS_clock_gettime
        swi     0
timok:  ldr     r0,[sp]
        add     sp,sp,#16
        ldmia   sp!,{pc}
