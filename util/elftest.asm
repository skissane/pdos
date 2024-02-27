# Simple ELF program for Linux x86

# Written by Paul Edwards
# Released the public domain

#ase86 -o elftest.o elftest.asm
#lde86 -s --emit-relocs -e myentry -o elftest.exe elftest.o
#
# If you are using Linux:
#chmod 755 elftest.exe (may be required)
#qemu-i386 elftest.exe (note - user, not system)
#
# If you are using PDOS/386:
# mcopy -i z:pdos.vhd --offset 128 elftest.exe ::


.text

.globl ___write
___write:
.globl __write
__write:
push %ebp
mov %esp, %ebp
push %ebx
push %ecx
push %edx

# function code 4 = write
movl $4, %eax
# handle
movl 8(%ebp), %ebx
# data pointer
movl 12(%ebp), %ecx
# length
movl 16(%ebp), %edx
int $0x80
pop %edx
pop %ecx
pop %ebx
pop %ebp
ret


.globl ___exita
___exita:
.globl __exita
__exita:
# exit/terminate
push %ebp
mov %esp, %ebp
push %ebx
movl 8(%ebp), %ebx
movl $1, %eax
int $0x80
pop %ebx
pop %ebp
ret


.globl myentry
myentry:

push $9
push $message
push $1
call __write

push $0
call __exita


# PDOS/386 can now cope with CR missing when
# writing to stdout

.globl message
message:
.string "hi\nthere\n"
