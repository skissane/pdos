# arm64 code for macOS

# Written by Paul Edwards
# Released to the public domain

# 64-bit ARM (AArch64) needs the stack 16-byte aligned

# macOS needs the syscall number in x16 and uses
# interrupt 0x80. Note that the syscall numbers are
# considered private, so you may need to recompile
# your software one day. This is not a problem for
# the intended purpose of creating a pseudo-bios for
# PDOS-generic. The pdos-generic applications do not
# require recompilation.

# Standard calling convention has parameters in
# x0-x7, and these are also used by the syscall,
# so minimal code is required.


# This is the new entry point. We simply capture
# the current status of the stack and pass it on
# to C code (linstart.c, not start.c) figure out
# the rest. Change of plans. argc and argv are
# already in x0 and x1 - the dynamic loader
# apparently gets first crack, so now we just
# call the C code without interference (except
# for the fact that the stack will be a bit
# deeper if we had more than 8 parameters of
# interest - but currently we don't)



        .align 4
        .globl ___pdpstart
___pdpstart:

#       mov x0, sp
        bl __start

# we shouldn't get here, but if we do, loop
# instead of doing something random
loop1:  b loop1





# int ___write(int fd, void *buf, int len);

        .align  4
        .globl  ___write
___write:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#4
#           @ SYS_write

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret




# void ___exita(int rc);

        .align  4
        .globl  ___exita
___exita:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#1
#           @ SYS_exit

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret




# void _longjmp(int rc);

        .align  4
        .globl  _longjmp
_longjmp:

# Not yet implemented or used - so just loop
# for when someone actually uses it
loop2:  b loop2




# int ___ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);

        .align  4
        .globl  ___ioctl
___ioctl:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#54
#           @ SYS_ioctl

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret




# int setjmp(jmp_buf env);

        .align  4
        .globl ___Ysetjmp
___Ysetjmp:

# Not yet implemented. Just return 0 until
# someone actually starts calling longjmp
        mov x0,#0
        ret





# int ___seek(int fd, int pos, int how);

        .align  4
        .globl  ___seek
___seek:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#199
#           @ SYS_lseek

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret





# int ___mprotect(const void *buf, size_t len, int prot);

        .align  4
        .globl  ___mprotect
___mprotect:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#74
#           @ SYS_mprotect

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret






# int ___read(int fd, void *buf, int len);

        .align  4
        .globl  ___read
___read:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#3
#           @ SYS_read

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret





# int ___close(int fd);

        .align  4
        .globl  ___close
___close:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#6
#           @ SYS_close

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret






# int ___open(char *path, int flags);

        .align  4
        .globl  ___open
___open:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#5
#           @ SYS_open

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret







# int ___rename(char *old, char *new);

        .align  4
        .globl  ___rename
___rename:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#128
#           @ SYS_rename

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret






# int ___remove(char *path);

        .align  4
        .globl  ___remove
___remove:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#10
#           @ SYS_unlink

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret






# int ___creat(char *path, int mode);

        .align  4
        .globl  ___creat
___creat:

# The syscall 8 has been taken away. Not sure what
# the replacement is. Perhaps an extended open?
# So just loop when someone actually uses it
loop3:  b loop3







# int __time(void);

        .align  4
        .globl ___time
___time:

# Not sure what to do with this. Just return 0
# for now.
        mov x0,#0
        ret







# int ___chdir(const char *filename);

        .align  4
        .globl  ___chdir
___chdir:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#10
#           @ SYS_chdir

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret






# int ___mkdir(const char *filename);

        .align  4
        .globl  ___mkdir
___mkdir:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#136
#           @ SYS_mkdir

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret






# int ___rmdir(const char *filename);

        .align  4
        .globl  ___rmdir
___rmdir:
        sub     sp,sp,#16
        str     x16, [sp, #0]
        mov     x16,#137
#           @ SYS_rmdir

        svc     #0x80

        ldr     x16, [sp, #0]
        add     sp,sp,#16
        ret





# There is a SYS_getdirentries of value 196,
# but I don't know how to use it

# int ___getdents(unsigned int fd, struct linux_dirent *dirent, int count);





# I don't know how to do a sleep, but we may not need
# to sleep anyway

# int ___nanosleep(unsigned int tval[2], unsigned int tval2[2]);
