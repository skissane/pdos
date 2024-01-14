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
        .type  __write, %function
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
        .type  __read, %function
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
        .type  __seek, %function
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
        .type  __creat, %function
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
        .type  __open, %function
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
        .type  __close, %function
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
        .type  __remove, %function
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
        .type  __rename, %function
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
        .type  __time, %function
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
        .type  __mprotect, %function
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



# int ___getdents(unsigned int fd, struct linux_dirent *dirent, int count);

        .globl  __getdents
        .globl  ___getdents
        .type  __getdents, %function
        .align  2
__getdents:
___getdents:
        stmfd   sp!,{r2,r7,lr}
.if STACKPARM
        ldr     r2,[sp,#20]     @ count
        ldr     r1,[sp,#16]      @ dirent
        ldr     r0,[sp,#12]      @ fd
.endif
        mov     r7,#141          @ SYS_getdents
        swi     0
gdok:   ldmia   sp!,{r2,r7,pc}




# int ___ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);

        .globl  __ioctl
        .globl  ___ioctl
        .type  __ioctl, %function
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

# int ___chdir(const char *filename);

        .globl  __chdir
        .globl  ___chdir
        .type  __chdir, %function
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
        .type  __mkdir, %function
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
        .type  __rmdir, %function
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


# This function is being defined, even for non-Linux,
# as a hopefully temporary workaround for executing
# code in BSS on ARM

# int ___nanosleep(unsigned int tval[2], unsigned int tval2[2]);

        .globl  __nanosleep
        .globl  ___nanosleep
        .type  __nanosleep, %function
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
