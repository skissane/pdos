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

        .globl  ___exita
        .align  2
___exita:
        stmfd   sp!,{lr}
        ldr     r0,[sp,#4]      @ rc
        mov     r7,#1           @ SYS_exit
        swi     0
        ldmia   sp!,{pc}

# int ___write(int fd, void *buf, int len);

        .globl  ___write
        .align  2
___write:
        stmfd   sp!,{lr}
        ldr     r2,[sp,#12]     @ len
        ldr     r1,[sp,#8]      @ buf
        ldr     r0,[sp,#4]      @ fd
        mov     r7,#4           @ SYS_write
        swi     0
wrtok:  ldmia   sp!,{pc}

# int ___read(int fd, void *buf, int len);

        .globl  ___read
        .align  2
___read:
        stmfd   sp!,{lr}
        ldr     r2,[sp,#12]     @ len
        ldr     r1,[sp,#8]      @ buf
        ldr     r0,[sp,#4]      @ fd
        mov     r7,#3           @ SYS_read
        swi     0
redok:  ldmia   sp!,{pc}
# int ___seek(int fd, int pos, int how);

        .globl  ___seek
        .align  2
___seek:
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

        .globl  ___open
        .align  2
___open:
        stmfd   sp!,{lr}
        mov     r2,#0x1A4       @ 0644
        ldr     r1,[sp,#8]      @ flags
        ldr     r0,[sp,#4]      @ path
        mov     r7,#5           @ SYS_open
        swi     0
opnok:  ldmia   sp!,{pc}

# int _close(int fd);

        .globl  ___close
        .align  2
___close:
        stmfd   sp!,{lr}
        ldr     r0,[sp,#4]      @ fd
        mov     r7,#6           @ SYS_close
        swi     0
clsok:  ldmia   sp!,{pc}

# int ___remove(char *path);

        .globl  ___remove
        .align  2
___remove:
        stmfd   sp!,{lr}
        ldr     r0,[sp,#4]      @ path
        mov     r7,#10          @ SYS_unlink
        swi     0
unlok:  ldmia   sp!,{pc}

# int ___rename(char *old, char *new);

        .globl  ___rename
        .align  2
___rename:
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

# This function is required by GCC but isn't used for anything
        .globl ___main
___main:
        movs    pc, lr

# unsigned integer divide
# inner loop code taken from http://me.henri.net/fp-div.html
# in:  r0 = num,  r1 = den
# out: r0 = quot, r1 = rem

        .globl  ___udivsi3
        .align  2
___udivsi3:   rsb     r2,r1,#0
        mov     r1,#0
        adds    r0,r0,r0
        .rept   32
        adcs    r1,r2,r1,lsl #1
        subcc   r1,r1,r2
        adcs    r0,r0,r0
        .endr
        mov     pc,lr

# signed integer divide
# in:  r0 = num,  r1 = den
# out: r0 = quot

        .globl  ___divsi3
        .align  2
___divsi3:
        stmfd   sp!,{lr}
        eor     r3,r0,r1        @ r3 = sign
#       asr     r3,r3,#31
        mov     r3,r3,ror #31
        cmp     r1,#0
        beq     divz
        rsbmi   r1,r1,#0
        cmp     r0,#0
        rsbmi   r0,r0,#0
        bl      ___udivsi3
        cmp     r3,#0
        rsbne   r0,r0,#0
        ldmia   sp!,{pc}
divz:   mov     r0,#8           @ SIGFPE
        stmfd   sp!,{r0}
        mov     r0,#1
        stmfd   sp!,{r0}
#        bl      Craise
        mov     r0,#0           @ if raise(SIGFPE) failed, return 0
        ldmia   sp!,{pc}

# signed integer modulo
# in:  r0 = num,  r1 = den
# out: r0 = rem

        .globl  ___modsi3
        .globl  ___umodsi3
        .align  2
___modsi3:
___umodsi3:
        stmfd   sp!,{lr}
#        asr     r4,r0,#31               @ r4 = sign
        mov     r4,r0,ror #31
        bl      ___divsi3
        mov     r0,r1
        cmp     r4,#0
        rsbne   r0,r0,#0
        ldmia   sp!,{pc}
