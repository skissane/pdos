; This code was written by Paul Edwards
; Released to the public domain

; ARM64 assembler in Visual Studio syntax

; 64-bit ARM (AArch64) needs the stack 16-byte aligned


 if :def:LINUX
 else
; setl LINUX 0
 endif


;.code
; guessing alignment
 area .text, code, readonly, align=4

; int setjmp(jmp_buf env);

        export  __Ysetjmp
__Ysetjmp proc

        str     x1,[x0,#52*2]
        mov     x1,x0
        str     x2,[x1,#40*2]
        str     x3,[x1,#44*2]
        str     x12,[x1,#48*2]
        mov     x2,sp
        str     x2,[x1]
        str     x11,[x1,#4*2]
;   @ fp
        str     lr,[x1,#8*2]
;    @ r14
        str     x4,[x1,#12*2]
        str     x5,[x1,#16*2]
        str     x6,[x1,#20*2]
        str     x7,[x1,#24*2]
        str     x8,[x1,#28*2]
        str     x9,[x1,#32*2]
;   @ rfp
        str     x10,[x1,#36*2]
;  @ sl

        str     x13,[x1,#14*4*2]
        str     x14,[x1,#15*4*2]
        str     x15,[x1,#16*4*2]
        str     x16,[x1,#17*4*2]
        str     x17,[x1,#18*4*2]
        str     x18,[x1,#19*4*2]
        str     x19,[x1,#20*4*2]
        str     x20,[x1,#21*4*2]
        str     x21,[x1,#22*4*2]
        str     x22,[x1,#23*4*2]
        str     x23,[x1,#24*4*2]
        str     x24,[x1,#25*4*2]
        str     x25,[x1,#26*4*2]
        str     x26,[x1,#27*4*2]
        str     x27,[x1,#28*4*2]
        str     x28,[x1,#29*4*2]
        str     x29,[x1,#30*4*2]
        str     x30,[x1,#31*4*2]

; This doesn't assemble
; The distance 32 appears to be too far
; There is no sign that gcc is generating references to x31
; so it is probably fine to do nothing. No sign of references
; to x17 either.
;        str     x31,[x1,#32*4*2]

        mov     x0, #0
        ret
        endp



; void longjmp(jmp_buf env, int v);

        export  longjmp
longjmp proc

        mov     x2,x0
        mov     x0,x1

        cbnz    x0,notzero
        cmp     x0,#0
;        moveq   x0,#1
        mov     x0,#1
notzero

        mov     x1,x2

;        ldr     sp,[x1]
        ldr     x2,[x1]
        mov     sp,x2

        ldr     x11,[x1,#4*2]
;      @ fp
        ldr     lr,[x1,#8*2]
        str     lr,[sp]
        ldr     x4,[x1,#12*2]
        ldr     x5,[x1,#16*2]
        ldr     x6,[x1,#20*2]
        ldr     x7,[x1,#24*2]
        ldr     x8,[x1,#28*2]
        ldr     x9,[x1,#32*2]
        ldr     x10,[x1,#36*2]
        ldr     x2,[x1,#40*2]
        ldr     x3,[x1,#44*2]
        ldr     x12,[x1,#48*2]
;  @ ip

        ldr     x13,[x1,#14*4*2]
        ldr     x14,[x1,#15*4*2]
        ldr     x15,[x1,#16*4*2]
        ldr     x16,[x1,#17*4*2]
        ldr     x17,[x1,#18*4*2]
        ldr     x18,[x1,#19*4*2]
        ldr     x19,[x1,#20*4*2]
        ldr     x20,[x1,#21*4*2]
        ldr     x21,[x1,#22*4*2]
        ldr     x22,[x1,#23*4*2]
        ldr     x23,[x1,#24*4*2]
        ldr     x24,[x1,#25*4*2]
        ldr     x25,[x1,#26*4*2]
        ldr     x26,[x1,#27*4*2]
        ldr     x27,[x1,#28*4*2]
        ldr     x28,[x1,#29*4*2]
        ldr     x29,[x1,#30*4*2]
        ldr     x30,[x1,#31*4*2]
;        ldr     x31,[x1,#32*4*2]

        ldr     x1,[x1,#52*2]
;        mov     pc,lr
        ret
        endp




 if :def:LINUX


; int ___write(int fd, void *buf, int len);

        export  __write
__write proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#64
;           @ SYS_write

        svc     #0

        ldr     x8, [sp, #0]
        add     sp,sp,#16
        ret
        endp



; AArch64 requires the use of renameat instead

; int ___rename(char *old, char *new);

        export  __rename
__rename proc
        sub     sp,sp,#32
        str     x8, [sp, #0]
        str     x1, [sp, #8]
        str     x2, [sp, #16]
        str     x3, [sp, #24]
        mov     x3, x1
        mov     x2, #-100
; AT_FDCWD
        mov     x1, x0
        mov     x0, #-100
; AT_FDCWD
        mov     x8,#38
;           @ SYS_renameat

        svc     #0

        ldr     x3, [sp, #24]
        ldr     x2, [sp, #16]
        ldr     x1, [sp, #8]
        ldr     x8, [sp, #0]
        add     sp,sp,#32
        ret
        endp



; int _close(int fd);

        export  __close
__close proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#57
;           @ SYS_close

        svc     #0

        ldr     x8, [sp, #0]
        add     sp,sp,#16
        ret
        endp



; int ___seek(int fd, int pos, int how);

        export  __seek
__seek  proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#62
;           @ SYS_lseek

        svc     #0

        ldr     x8, [sp, #0]
        add     sp,sp,#16
        ret
        endp



; AArch64 requires the use of unlinkat

; int ___remove(char *path);

        export  __remove
__remove proc
        sub     sp,sp,#32
        str     x8, [sp, #0]
        str     x1, [sp, #8]
        str     x2, [sp, #16]
        mov     x2,#0
; flags (not defining AT_REMOVEDIR)
        mov     x1, x0
        mov     x0, #-100
; AT_FDCWD
        mov     x8,#35
;           @ SYS_unlinkat

        svc     #0

        ldr     x2, [sp, #16]
        ldr     x1, [sp, #8]
        ldr     x8, [sp, #0]
        add     sp,sp,#32
        ret
        endp




; AArch64 requires the use of openat instead of open

; int _open(char *path, int flags);

        export  __open
__open  proc
        sub     sp,sp,#32
        str     x8, [sp, #0]
        str     x1, [sp, #8]
        str     x2, [sp, #16]
        str     x3, [sp, #24]
        mov     x3,#0x1A4
;        @ 0644
        mov     x2, x1
        mov     x1, x0
        mov     x0, #-100
; AT_FDCWD
        mov     x8,#56
;           @ SYS_openat

        svc     #0

        ldr     x3, [sp, #24]
        ldr     x2, [sp, #16]
        ldr     x1, [sp, #8]
        ldr     x8, [sp, #0]
        add     sp,sp,#32
        ret
        endp



; int ___ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);

        export  __ioctl
__ioctl proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#29
;           @ SYS_ioctl

        svc     #0

        ldr     x8, [sp, #0]
        add     sp,sp,#16
        ret
        endp



; int ___getpid(void);

        export  __getpid
__getpid proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#172
;           @ SYS_getpid

        svc     #0

        ldr     x8, [sp, #0]
        add     sp,sp,#16
        ret
        endp



; int ___read(int fd, void *buf, int len);

        export  __read
__read  proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#63
;           @ SYS_read

        svc     #0

        ldr     x8, [sp, #0]
        add     sp,sp,#16
        ret
        endp





; void _exita(int rc);

        export  __exita
__exita proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#93
;           @ SYS_exit

        svc     #0
        ldr     x8,[sp, #0]
        add     sp,sp,#16
        ret
        endp




; int __time(void);

        export  __time
__time  proc
; Unusual amount of storage used
        sub     sp,sp,#32
; Note that this store is unusual
        str     x8, [sp, #16]
        str     x1, [sp, #24]
; this is CLOCK_REALTIME
        mov     x0, #0
; this is a struct timespec
; it contains number of seconds followed by
; number of nanoseconds - both 64-bit values
        mov     x1, sp
        mov     x8,#113
;           @ SYS_clock_gettime

        svc     #0
        ldr     x1, [sp, #24]
        ldr     x8,[sp, #16]
; we are only interested in the number of seconds
        ldr     x0, [sp, #0]
; Unusual stack correction
        add     sp,sp,#32
        ret
        endp




; int ___clone(...);

        export  __clone
__clone proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#220
;           @ SYS_clone

        svc     #0

        ldr     x8, [sp, #0]
        add     sp,sp,#16
        ret
        endp




; int ___waitid(...);

        export  __waitid
__waitid proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#95
;           @ SYS_waitid

        svc     #0

        ldr     x8, [sp, #0]
        add     sp,sp,#16
        ret
        endp




; int ___execve(...);

        export  __execve
__execve proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#221
;           @ SYS_execve

        svc     #0

        ldr     x8, [sp, #0]
        add     sp,sp,#16
        ret
        endp




; int ___mmap(...);

        export  __mmap
__mmap  proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#222
;           @ SYS_mmap

        svc     #0

        ldr     x8, [sp, #0]
        add     sp,sp,#16
        ret
        endp



; int ___munmap(...);

        export  __munmap
__munmap proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#215
;           @ SYS_munmap

        svc     #0

        ldr     x8, [sp, #0]
        add     sp,sp,#16
        ret
        endp



; mremap is 216


; int ___chdir(const char *filename);

        export  __chdir
__chdir proc
        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#49
;           @ SYS_chdir

        svc     #0
        ldr     x8,[sp, #0]
        add     sp,sp,#16
        ret
        endp




; AArch64 uses mkdirat

; int ___mkdir(const char *filename, int mode);

        export  __mkdir
__mkdir proc
        sub     sp,sp,#32
        str     x8, [sp, #0]
        str     x1, [sp, #8]
        str     x2, [sp, #16]
        mov     x2, x1
        mov     x1, x0
        mov     x0, #-100
; AT_FDCWD
        mov     x8,#34
;           @ SYS_mkdirat

        svc     #0

        ldr     x2, [sp, #16]
        ldr     x1, [sp, #8]
        ldr     x8, [sp, #0]
        add     sp,sp,#32
        ret
        endp




; AArch64 requires unlinkat with AT_REMOVEDIR

; int ___rmdir(const char *filename);

        export  __rmdir
__rmdir proc
        sub     sp,sp,#32
        str     x8, [sp, #0]
        str     x1, [sp, #8]
        str     x2, [sp, #16]
        mov     x2,#0x200
; flags (setting AT_REMOVEDIR)
        mov     x1, x0
        mov     x0, #-100
; AT_FDCWD
        mov     x8,#35
;           @ SYS_unlinkat

        svc     #0

        ldr     x2, [sp, #16]
        ldr     x1, [sp, #8]
        ldr     x8, [sp, #0]
        add     sp,sp,#32
        ret
        endp




; AArch64 doesn't have getdents - only getdents64

; int ___getdents64(unsigned int fd, struct linux_dirent64 *dirent, int count);

        export  __getdents64
__getdents64 proc

        sub     sp,sp,#16
        str     x8, [sp, #0]
        mov     x8,#61
;           @ SYS_getdents64

        svc     #0

        ldr     x8, [sp, #0]
        add     sp,sp,#16
        ret
        endp



; LINUX
 endif



        export  __chkstk
__chkstk proc
        ret
        endp



        end
