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

# ARM (a.out) and ERM (ELF) have been changed to put all
# parameters on the stack, to match SubC
# PRM (PE) is locked in to the UEFI calling convention,
# which appears to use registers.

# Note that r2 (and presumably other registers) need to be
# preserved (for gccarm I think)


.ifndef STACKPARM
.set STACKPARM,0
.endif

.ifndef SUBC
.set SUBC,0
.endif

.ifndef LINUX
.set LINUX,0
.endif

.ifndef ELF
.set ELF,0
.endif

.ifndef THUMBSTUB
.set THUMBSTUB,0
.endif

.ifndef NEEDENT
.set NEEDENT,0
.endif


.globl ___gccmain
.globl ____gccmain
___gccmain:
____gccmain:
mov pc,lr



.if !THUMBSTUB
# I haven't seen direct evidence that r2 and r3
# need to be preserved in setjmp/longjmp, but
# doing so anyway. ditto r12 and r1.

        .text
        .align  2
# int setjmp(jmp_buf env);

        .globl  __Ysetjmp
        .globl  ___Ysetjmp
.if ELF
        .type  __Ysetjmp, %function
.endif
        .align  2
__Ysetjmp:
___Ysetjmp:

.if STACKPARM
        ldr     r0,[sp]         @ env
.endif
        str     r1,[r0,#52]
        mov     r1,r0
        str     r2,[r1,#40]
        str     r3,[r1,#44]
        str     r12,[r1,#48]  @ ip
        mov     r2,sp
#        add     r2,r2,#4
#        str     sp,[r1]
        str     r2,[r1]
        str     r11,[r1,#4]   @ fp
        str     lr,[r1,#8]    @ r14
        str     r4,[r1,#12]
        str     r5,[r1,#16]
        str     r6,[r1,#20]
        str     r7,[r1,#24]
        str     r8,[r1,#28]
        str     r9,[r1,#32]   @ rfp
        str     r10,[r1,#36]  @ sl
        mov     r0,#0
        mov     pc,lr

# void longjmp(jmp_buf env, int v);

        .globl  longjmp
        .globl  _longjmp
.if ELF
        .type longjmp, %function
.endif
        .align  2
longjmp:
_longjmp:
.if STACKPARM
        ldr     r0,[sp]      @ env
        ldr     r1,[sp,#4]   @ v
.endif
        mov     r2,r0
        mov     r0,r1

        cmp     r0,#0
        moveq   r0,#1

        mov     r1,r2

        ldr     sp,[r1]
        ldr     r11,[r1,#4]      @ fp
        ldr     lr,[r1,#8]
        ldr     r4,[r1,#12]
        ldr     r5,[r1,#16]
        ldr     r6,[r1,#20]
        ldr     r7,[r1,#24]
        ldr     r8,[r1,#28]
        ldr     r9,[r1,#32]
        ldr     r10,[r1,#36]
        ldr     r2,[r1,#40]
        ldr     r3,[r1,#44]
        ldr     r12,[r1,#48]  @ ip
        ldr     r1,[r1,#52]
        mov     pc,lr

.endif

# There is no such thing as 16-bit ARM
# ARM was originally 32-bit
# 16-bit is actually a 32-bit enhancement - providing more
# compact instructions. This is called Thumb mode. In Thumb
# mode, some instructions are 32-bit, but most are 16-bit I think
# Windows 11 32-bit will enter an application in plain ARM mode.
# But the Visual C compiler generates the more efficient Thumb
# mode instructions, with no apparent option to restrict it to the
# old ARM32. So we need a stub to switch to Thumb mode so that we
# can call Microsoft-generated code. This stub is not suitable for
# calling gccprm-generated code, which is old ARM32 (currently).

# In order to get into Thumb mode, you simply add 1 to the target
# address, and that low bit is ignored. All instructions are on a
# 16 or 32-bit boundary, so the 1 would normally not be valid.
# That's how it is possible to use it as a flag.
# Some linkers will add the 1 automatically when they see that a
# function being called is Thumb. But we are using a simpler
# linker (pdld) which doesn't do that, and this code would be
# inconsistent with a linker that does it automatically. We should
# probably do an "or" instead of an add so that it works with
# either.


.if NEEDENT


# Note that although we don't really need to save r0, I am
# saving it anyway as a means to keep the stack 8-byte aligned
# as otherwise when we go to call msvcrt.dll we can't print
# doubles

# It seems that when msvcrt.dll calls an ARM entry point, the
# stack is not 8-byte aligned, so we are adjusting it here.
# We should really check to see if it is already properly aligned

.globl __pdpent
__pdpent:
        stmfd   sp!,{r0,lr}
        sub     sp, sp, #4
#        ldr     r0,=mainCRTStartup
# Activate Thumb mode by adding 1 (should probaby use OR instead,
# in case the above ldr has already compensated for that)
#        add     r0, r0, #1
# mainCRTStartup returns by moving lr into pc, and will restore
# our original ARM mode (not Thumb) due to that, I think
        bl      mainCRTStartup
        add     sp, sp, #4
        ldmia   sp!,{r0,pc}

.endif



.if LINUX
# void _exita(int rc);

        .globl  __exita
        .globl  ___exita
.if ELF
        .type  __exita, %function
.endif
        .align  2
__exita:
___exita:
        stmfd   sp!,{lr}
.if STACKPARM
        ldr     r0,[sp,#4]      @ rc
.endif
        mov     r7,#1           @ SYS_exit
        swi     0
        ldmia   sp!,{pc}

# int ___write(int fd, void *buf, int len);

        .globl  __write
        .globl  ___write
.if ELF
        .type  __write, %function
.endif
        .align  2
__write:
___write:
        stmfd   sp!,{r2,r7,lr}
.if STACKPARM
        ldr     r2,[sp,#20]     @ len
        ldr     r1,[sp,#16]      @ buf
        ldr     r0,[sp,#12]      @ fd
.endif
        mov     r7,#4           @ SYS_write
        swi     0
wrtok:  ldmia   sp!,{r2,r7,pc}

# int ___read(int fd, void *buf, int len);

        .globl  __read
        .globl  ___read
.if ELF
        .type  __read, %function
.endif
        .align  2
__read:
___read:
        stmfd   sp!,{r2,r7,lr}
.if STACKPARM
        ldr     r2,[sp,#20]     @ len
        ldr     r1,[sp,#16]      @ buf
        ldr     r0,[sp,#12]      @ fd
.endif
        mov     r7,#3           @ SYS_read
        swi     0
redok:  ldmia   sp!,{r2,r7,pc}

# int ___seek(int fd, int pos, int how);

        .globl  __seek
        .globl  ___seek
.if ELF
        .type  __seek, %function
.endif
        .align  2
__seek:
___seek:
        stmfd   sp!,{r2,r7,lr}
.if STACKPARM
        ldr     r2,[sp,#20]      @ how
        ldr     r1,[sp,#16]      @ off_t
        ldr     r0,[sp,#12]      @ fd
.endif
        mov     r7,#19     @ SYS_lseek
        swi     0
lskok:  
        ldmia   sp!,{r2,r7,pc}

# int __creat(char *path, int mode);

        .globl  __creat
        .globl  ___creat
.if ELF
        .type  __creat, %function
.endif
        .align  2
__creat:
___creat:
        stmfd   sp!,{r7,lr}
#        ldr     r1,[sp,#12]      @ mode
        mov     r1,#0x1A4       @ 0644
.if STACKPARM
        ldr     r0,[sp,#8]      @ path
.endif
        mov     r7,#8           @ SYS_creat
        swi     0
crtok:  ldmia   sp!,{r7,pc}

# int _open(char *path, int flags);

        .globl  __open
        .globl  ___open
.if ELF
        .type  __open, %function
.endif
        .align  2
__open:
___open:
        stmfd   sp!,{r2,r7,lr}
        mov     r2,#0x1A4       @ 0644
.if STACKPARM
        ldr     r1,[sp,#16]     @ flags
        ldr     r0,[sp,#12]      @ path
.endif
        mov     r7,#5           @ SYS_open
        swi     0
opnok:  ldmia   sp!,{r2,r7,pc}

# int _close(int fd);

        .globl  __close
        .globl  ___close
.if ELF
        .type  __close, %function
.endif
        .align  2
__close:
___close:
        stmfd   sp!,{r7,lr}
.if STACKPARM
        ldr     r0,[sp,#8]      @ fd
.endif
        mov     r7,#6           @ SYS_close
        swi     0
clsok:  ldmia   sp!,{r7,pc}

# int ___remove(char *path);

        .globl  __remove
        .globl  ___remove
.if ELF
        .type  __remove, %function
.endif
        .align  2
__remove:
___remove:
        stmfd   sp!,{r7,lr}
.if STACKPARM
        ldr     r0,[sp,#8]      @ path
.endif
        mov     r7,#10          @ SYS_unlink
        swi     0
unlok:  ldmia   sp!,{r7,pc}

# int ___rename(char *old, char *new);

        .globl  __rename
        .globl  ___rename
.if ELF
        .type  __rename, %function
.endif
        .align  2
__rename:
___rename:
        stmfd   sp!,{r7,lr}
.if STACKPARM
        ldr     r1,[sp,#12]     @ new
        ldr     r0,[sp,#8]      @ old
.endif
        mov     r7,#0x26        @ SYS_rename
        swi     0
renok:  ldmia   sp!,{r7,pc}

# int __time(void);

        .globl  __time
        .globl  ___time
.if ELF
        .type  __time, %function
.endif
        .align  2
__time:
___time:
        stmfd   sp!,{r7,lr}
        sub     sp,sp,#16       @ struct timespec
        mov     r1,sp
        mov     r0,#0           @ CLOCK_REALTIME
        ldr     r7,=0x107       @ SYS_clock_gettime
        swi     0
timok:  ldr     r0,[sp]
        add     sp,sp,#16
        ldmia   sp!,{r7,pc}

# int ___mprotect(const void *buf, size_t len, int prot);

        .globl  __mprotect
        .globl  ___mprotect
.if ELF
        .type  __mprotect, %function
.endif
        .align  2
__mprotect:
___mprotect:
        stmfd   sp!,{r2,r7,lr}
.if STACKPARM
        ldr     r2,[sp,#20]     @ prot
        ldr     r1,[sp,#16]      @ len
        ldr     r0,[sp,#12]      @ buf
.endif
        mov     r7,#125          @ SYS_mprotect
        swi     0
mpok:   ldmia   sp!,{r2,r7,pc}



# count is size of buffer

# int ___getdents(unsigned int fd, struct linux_dirent *dirent, int count);

        .globl  __getdents
        .globl  ___getdents
.if ELF
        .type  __getdents, %function
.endif
        .align  2
__getdents:
___getdents:

# Now that we're using stack parameters, we likely
# need to preserve r1 too (ditto for other functions)

        stmfd   sp!,{r1,r2,r7,lr}
.if STACKPARM
        ldr     r2,[sp,#24]      @ count
        ldr     r1,[sp,#20]      @ dirent
        ldr     r0,[sp,#16]      @ fd
.endif
        mov     r7,#141          @ SYS_getdents
        swi     0
gdok:   ldmia   sp!,{r1,r2,r7,pc}




# int ___ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);

        .globl  __ioctl
        .globl  ___ioctl
.if ELF
        .type  __ioctl, %function
.endif
        .align  2
__ioctl:
___ioctl:
        stmfd   sp!,{r2,r7,lr}
.if STACKPARM
        ldr     r2,[sp,#20]      @ arg
        ldr     r1,[sp,#16]      @ cmd
        ldr     r0,[sp,#12]      @ fd
.endif
        mov     r7,#54           @ SYS_ioctl
        swi     0
iocok:  ldmia   sp!,{r2,r7,pc}




# int ___getpid(void);

        .globl  __getpid
        .globl  ___getpid
.if ELF
        .type  __getpid, %function
.endif
        .align  2
__getpid:
___getpid:
        stmfd   sp!,{r7,lr}
        mov     r7,#20           @ SYS_getpid
        swi     0
pidok:  ldmia   sp!,{r7,pc}




# int ___clone(...);

        .globl  __clone
        .globl  ___clone
.if ELF
        .type  __clone, %function
.endif
        .align  2
__clone:
___clone:
        stmfd   sp!,{r2,r3,r4,r7,lr}
.if STACKPARM
        ldr     r4,[sp,#36]      @ p4
        ldr     r3,[sp,#32]      @ p3
        ldr     r2,[sp,#28]      @ p2
        ldr     r1,[sp,#24]      @ p1
        ldr     r0,[sp,#20]      @ p0
.endif
        mov     r7,#120          @ SYS_clone
        swi     0
        ldmia   sp!,{r2,r3,r4,r7,pc}




# int ___waitid(...);

        .globl  __waitid
        .globl  ___waitid
.if ELF
        .type  __waitid, %function
.endif
        .align  2
__waitid:
___waitid:
        stmfd   sp!,{r2,r3,r4,r7,lr}
.if STACKPARM
        ldr     r4,[sp,#36]      @ p4
        ldr     r3,[sp,#32]      @ p3
        ldr     r2,[sp,#28]      @ p2
        ldr     r1,[sp,#24]      @ p1
        ldr     r0,[sp,#20]      @ p0
.endif
        mov     r7,#280          @ SYS_waitid
        swi     0
        ldmia   sp!,{r2,r3,r4,r7,pc}




# int ___execve(...)

        .globl  __execve
        .globl  ___execve
.if ELF
        .type  __execve, %function
.endif
        .align  2
__execve:
___execve:
        stmfd   sp!,{r2,r7,lr}
.if STACKPARM
        ldr     r2,[sp,#20]
        ldr     r1,[sp,#16]
        ldr     r0,[sp,#12]
.endif
        mov     r7,#11           @ SYS_execve
        swi     0
        ldmia   sp!,{r2,r7,pc}




# int ___mmap(...);

        .globl  __mmap
        .globl  ___mmap
.if ELF
        .type  __mmap, %function
.endif
        .align  2
__mmap:
___mmap:
        stmfd   sp!,{r2,r3,r4,r5,r6,r7,lr}
.if STACKPARM
        ldr     r6,[sp,#52]      @ p6
        ldr     r5,[sp,#48]      @ p5
        ldr     r4,[sp,#44]      @ p4
        ldr     r3,[sp,#40]      @ p3
        ldr     r2,[sp,#36]      @ p2
        ldr     r1,[sp,#32]      @ p1
        ldr     r0,[sp,#28]      @ p0
.endif
        mov     r7,#192          @ SYS_mmap2
        swi     0
        ldmia   sp!,{r2,r3,r4,r5,r6,r7,pc}


# int ___munmap(char *addr, int len);

        .globl  __munmap
        .globl  ___munmap
.if ELF
        .type  __munmap, %function
.endif
        .align  2
__munmap:
___munmap:
        stmfd   sp!,{r7,lr}
.if STACKPARM
        ldr     r1,[sp,#12]     @ addr
        ldr     r0,[sp,#8]      @ len
.endif
        mov     r7,#91          @ SYS_munmap
        swi     0
        ldmia   sp!,{r7,pc}



# mremap 163




# int ___chdir(const char *filename);

        .globl  __chdir
        .globl  ___chdir
.if ELF
        .type  __chdir, %function
.endif
        .align  2
__chdir:
___chdir:
        stmfd   sp!,{r7,lr}
.if STACKPARM
        ldr     r0,[sp,#8]      @ filename
.endif
        mov     r7,#12           @ SYS_chdir
        swi     0
cdok:  ldmia   sp!,{r7,pc}

# int ___mkdir(const char *pathname, unsigned int mode);

        .globl  __mkdir
        .globl  ___mkdir
.if ELF
        .type  __mkdir, %function
.endif
        .align  2
__mkdir:
___mkdir:
        stmfd   sp!,{r7,lr}
.if STACKPARM
        ldr     r1,[sp,#12]      @ mode
        ldr     r0,[sp,#8]      @ pathname
.endif
        mov     r7,#39           @ SYS_mkdir
        swi     0
mdok:  ldmia   sp!,{r7,pc}

# int ___rmdir(const char *pathname);

        .globl  __rmdir
        .globl  ___rmdir
.if ELF
        .type  __rmdir, %function
.endif
        .align  2
__rmdir:
___rmdir:
        stmfd   sp!,{r7,lr}
.if STACKPARM
        ldr     r0,[sp,#8]      @ pathname
.endif
        mov     r7,#40           @ SYS_rmdir
        swi     0
rdok:  ldmia   sp!,{r7,pc}

.endif


# This function is required by GCC but isn't used for anything
        .globl __main
        .globl ___main
__main:
___main:
        mov    pc, lr


# unsigned integer divide
# inner loop code taken from http://me.henri.net/fp-div.html
# in:  r0 = num,  r1 = den
# out: r0 = quot, r1 = rem

.if !THUMBSTUB

        .globl  __rt_udiv
        .globl  __udivsi3
        .globl  ___udivsi3
.if ELF
        .type  __udivsi3, %function
.endif
        .globl  __aeabi_uidiv
.if ELF
        .type  __aeabi_uidiv, %function
.endif
        .align  2
__rt_udiv:
__udivsi3:
___udivsi3:
__aeabi_uidiv:

        stmfd   sp!,{r2,lr}

.if STACKPARM
        ldr     r0,[sp,#8]
        ldr     r1,[sp,#12]
.endif

        rsb     r2,r1,#0
        mov     r1,#0
        adds    r0,r0,r0
        .rept   32
        adcs    r1,r2,r1,lsl #1
        subcc   r1,r1,r2
        adcs    r0,r0,r0
        .endr
#        mov     pc,lr
        ldmia   sp!,{r2,pc}



# This is the traditional function that expects
# register parameters, not on the stack - for
# use even in a stack parameter environment
# We may wish to eliminate this function in
# a stack environment though

        .globl  __udivsi3_trad
        .globl  ___udivsi3_trad
.if ELF
        .type  __udivsi3_trad, %function
.endif
        .globl  __aeabi_uidiv_trad
.if ELF
        .type  __aeabi_uidiv_trad, %function
.endif
        .align  2
__udivsi3_trad:
___udivsi3_trad:
__aeabi_uidiv_trad:
        rsb     r2,r1,#0
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

        .globl  __rt_sdiv
        .globl  __divsi3
        .globl  ___divsi3
.if ELF
        .type  __divsi3, %function
.endif
        .globl  __aeabi_idiv
.if ELF
        .type  __aeabi_idiv, %function
.endif
        .align  2
__rt_sdiv:
__divsi3:
___divsi3:
__aeabi_idiv:
# Need to preserve r2 because the
# function we may call, doesn't
        stmfd   sp!,{r2,r3,lr}
.if STACKPARM
        ldr     r0,[sp,#12]
        ldr     r1,[sp,#16]
.endif
        eor     r3,r0,r1        @ r3 = sign
#       asr     r3,r3,#31
        mov     r3,r3,asr#31
        cmp     r1,#0
        beq     divz
        rsbmi   r1,r1,#0
        cmp     r0,#0
        rsbmi   r0,r0,#0
        bl      ___udivsi3_trad
        cmp     r3,#0
        rsbne   r0,r0,#0
        ldmia   sp!,{r2,r3,pc}
divz:   mov     r0,#8           @ SIGFPE
        stmfd   sp!,{r0}
        mov     r0,#1
        stmfd   sp!,{r0}
#        bl      Craise
        mov     r0,#0           @ if raise(SIGFPE) failed, return 0
        ldmia   sp!,{r2,r3,pc}



# This is the traditional function that expects
# register parameters, not on the stack - for
# use even in a stack parameter environment
# We may wish to eliminate this function in
# a stack environment though

        .globl  __divsi3_trad
        .globl  ___divsi3_trad
.if ELF
        .type  __divsi3_trad, %function
.endif
        .globl  __aeabi_idiv_trad
.if ELF
        .type  __aeabi_idiv_trad, %function
.endif
        .align  2
__divsi3_trad:
___divsi3_trad:
__aeabi_idiv_trad:
        stmfd   sp!,{lr}
        eor     r3,r0,r1        @ r3 = sign
#       asr     r3,r3,#31
        mov     r3,r3,asr#31
        cmp     r1,#0
        beq     divz_trad
        rsbmi   r1,r1,#0
        cmp     r0,#0
        rsbmi   r0,r0,#0
        bl      ___udivsi3_trad
        cmp     r3,#0
        rsbne   r0,r0,#0
        ldmia   sp!,{pc}
divz_trad:
        mov     r0,#8           @ SIGFPE
        stmfd   sp!,{r0}
        mov     r0,#1
        stmfd   sp!,{r0}
#        bl      Craise
        mov     r0,#0           @ if raise(SIGFPE) failed, return 0
        ldmia   sp!,{pc}



# signed integer modulo
# in:  r0 = num,  r1 = den
# out: r0 = rem

        .globl  __modsi3
        .globl  ___modsi3
.if ELF
        .type  __modsi3, %function
.endif
        .globl  __aeabi_idivmod
.if ELF
        .type  __aeabi_idivmod, %function
.endif
        .align  2
__modsi3:
___modsi3:
__aeabi_idivmod:
# Need to preserve r3, because the function
# we call doesn't
        stmfd   sp!,{r3,r4,lr}
.if STACKPARM
        ldr     r0,[sp,#12]
        ldr     r1,[sp,#16]
.endif
#        asr     r4,r0,#31               @ r4 = sign
        mov     r4,r0,asr#31
        bl      ___divsi3_trad
        mov     r0,r1
        cmp     r4,#0
        rsbne   r0,r0,#0
        ldmia   sp!,{r3,r4,pc}

# unsigned integer modulo
# in:  r0 = num,  r1 = den
# out: r0 = rem

        .globl  __umodsi3
        .globl  ___umodsi3
.if ELF
        .type  __umodsi3, %function
.endif
        .globl  __aeabi_uidivmod
.if ELF
        .type  __aeabi_uidivmod, %function
.endif
        .align  2
__umodsi3:
___umodsi3:
__aeabi_uidivmod:
# Need to preserve r2, because the function
# we call  doesn't
        stmfd   sp!,{r2,lr}
.if STACKPARM
        ldr     r0,[sp,#8]
        ldr     r1,[sp,#12]
.endif
        bl      ___udivsi3_trad
        mov     r0,r1
        ldmia   sp!,{r2,pc}

.endif



# This is a call to memcpy where the parameters are
# 4-byte aligned, but we don't care about that.
# Commented out for now because it is no longer being called.
#        .globl __aeabi_memcpy4
#        .type __aeabi_memcpy4, %function
#__aeabi_memcpy4:
#        str    lr, [sp, #-4]!
#        ldr    lr, [sp], #4
#        b      _memcpy


# This is a memcpy call, but it doesn't need to provide a
# return value. We don't care about that.
# Commented out for now as it is no longer being called.
#        .globl __aeabi_memcpy
#        .type __aeabi_memcpy, %function
#__aeabi_memcpy:
#        str    lr, [sp, #-4]!
#        ldr    lr, [sp], #4
#        b      _memcpy


.if SUBC
# Support routines for SubC

# unsigned integer divide
# inner loop code taken from http://me.henri.net/fp-div.html
# in:  r0 = num,  r1 = den
# out: r0 = quot, r1 = rem

	.globl	udiv
	.align	2
udiv:	rsb     r2,r1,#0
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

	.globl	sdiv
	.align	2
sdiv:	stmfd	sp!,{lr}
	eor	r3,r0,r1	@ r3 = sign
	mov	r3,r3,asr #31
	cmp	r1,#0
	beq	divz2
	rsbmi	r1,r1,#0
	cmp	r0,#0
	rsbmi	r0,r0,#0
	bl	udiv
	cmp	r3,#0
	rsbne	r0,r0,#0
	ldmfd	sp!,{pc}
divz2:	mov	r0,#8		@ SIGFPE
	stmfd	sp!,{r0}
	mov	r0,#1
	stmfd	sp!,{r0}
#	bl	Craise
	mov	r0,#0		@ if raise(SIGFPE) failed, return 0
	ldmfd	sp!,{pc}

# signed integer modulo
# in:  r0 = num,  r1 = den
# out: r0 = rem

	.globl	srem
	.align	2
srem:	stmfd	sp!,{lr}
	mov	r4,r0,asr #31		@ r4 = sign
	bl	sdiv
	mov	r0,r1
	cmp	r4,#0
	rsbne	r0,r0,#0
	ldmfd	sp!,{pc}

# internal switch(expr) routine
# r1 = switch table, r0 = expr

	.globl	switch
	.align	2
switch:	ldr	r2,[r1]		@ # of non-default cases
	add	r1,r1,#4	@ first case
next:	ldr	r3,[r1]
	cmp	r0,r3
	beq	match
	add	r1,r1,#8
	subs	r2,r2,#1
	bne	next
	ldr	r0,[r1]
	blx	r0
match:	add	r1,r1,#4
	ldr	r0,[r1]
	blx	r0

.endif



# This function is being defined, even for non-Linux,
# as a hopefully temporary workaround for executing
# code in BSS on ARM

# int ___nanosleep(unsigned int tval[2], unsigned int tval2[2]);

        .globl  __nanosleep
        .globl  ___nanosleep
.if ELF
        .type  __nanosleep, %function
.endif
        .align  2
__nanosleep:
___nanosleep:
        stmfd   sp!,{r7,lr}
.if STACKPARM
        ldr     r1,[sp,#12]      @ time struct
        ldr     r0,[sp,#8]       @ time struct
.endif
        mov     r7,#162          @ SYS_nanosleep
        swi     0
nsok:   ldmia   sp!,{r7,pc}



# Trying cacheflush instead
#
# Some documentation says that the parameters are
#
# start pointer, size and flags as follows:
# ICACHE = 1
# DCACHE = 2
# BCACHE = 3 (I | D)
#
# However, other documentation - that actually worked -
# says that the parameters are start pointer, end pointer,
# and 0. So that is what is shown below and used.

        .globl  __cacheflush
        .globl  ___cacheflush
.if ELF
        .type  __cacheflush, %function
.endif
        .align  2
__cacheflush:
___cacheflush:
        stmfd   sp!,{r2,r7,lr}
.if STACKPARM
        ldr     r2,[sp,#20]      @ flag
        ldr     r1,[sp,#16]      @ end
        ldr     r0,[sp,#12]      @ start
.endif
# Can't use this directly
#        mov     r7,#0xf0002     @ SYS_cacheflush
        mov     r7, #0xf0000
        add     r7,r7,#2
        swi     0
        ldmia   sp!,{r2,r7,pc}




# These floating point routines are all dummied to
# hopefully allow gccarm to run when built with
# -msoft-float given that -mhard-float (default)
# is not working on real hardware such as a
# Pinebook Pro for unknown reasons (ldfd is an
# illegal instruction).

# Note that I suspect that the problem is that
# floating point hasn't been initialized for some
# reason - which is the normal default. But I am
# not in a position to initialize floating point
# on some environments like smartphones, as I don't
# have privilege, which I assume I need. And even in
# privileged environments, I wasn't able to get it
# initialized - the CPACR seemed to be unable to be
# read.

# Fortunately I was able to get some public domain
# floating point routines from the Plasma project.
# Unfortunately those are only single precision.
# Fortunately I was able to configure gccarm so that
# "double" and "long double" were single precision.
# So the remaining stubs should never be exercised.

# I should probably delete them to failsafe this code,
# but hopefully someone will provide double precision
# earlier than that, or at least the ability to
# convert between float and double so that I can do
# double precision as single precision and then return
# the compiler to its former glory.

#.globl ___addsf3
#.globl ____addsf33
#___addsf3:
#____addsf3:

#.globl ___adddf3
#.globl ____adddf33
#___adddf3:
#____adddf3:

#.globl ___subsf3
#.globl ____subsf33
#___subsf3:
#____subsf3:

.globl ___subdf3
.globl ____subdf33
___subdf3:
____subdf3:

#.globl ___mulsf3
#.globl ____mulsf33
#___mulsf3:
#____mulsf3:

.globl ___muldf3
.globl ____muldf3
___muldf3:
____muldf3:

.globl ___divdf3
.globl ____divdf3
___divdf3:
____divsd3:

#.globl ___divsf3
#.globl ____divsf33
#___divsf3:
#____divsf3:

#.globl ___floatsisf
#.globl ____floatsisf
#___floatsisf:
#____floatsisf:

#.globl ___gtsf2
#.globl ____gtsf2
#___gtsf2:
#____gtsf2:

.globl ___gtdf2
.globl ____gtdf2
___gtdf2:
____gtdf2:

#.globl ___ltsf2
#.globl ____ltsf2
#___ltsf2:
#____ltsf2:

.globl ___ltdf2
.globl ____ltdf2
___ltdf2:
____ltdf2:

#.globl ___nesf2
#.globl ____nesf2
#___nesf2:
#____nesf2:

#.globl ___eqsf2
#.globl ____eqsf2
#___eqsf2:
#____eqsf2:

.globl ___eqdf2
.globl ____eqdf2
___eqdf2:
____eqdf2:

#.globl ___fixsfsi
#.globl ____fixsfsi
#___fixsfsi:
#____fixsfsi:

#.globl ___fixdfsi
#.globl ____fixdfsi
#___fixdfsi:
#____fixdfsi:

.globl ___extendsfdf2
.globl ____extendsfdf2
___extendsfdf2:
____extendsfdf2:

.globl ___floatsidf
.globl ____floatsidf
___floatsidf:
____floatsidf:

.globl ___truncdfsf2
.globl ____truncdfsf2
___truncdfsf2:
____truncdfsf2:

.globl ___negdf2
.globl ____negdf2
___negdf2:
____negdf2:

.globl ___nedf2
.globl ____nedf2
___nedf2:
____nedf2:

.globl ___ledf2
.globl ____ledf2
___ledf2:
____ledf2:

.globl ___gedf2
.globl ____gedf2
___gedf2:
____gedf2:

mov pc,lr
