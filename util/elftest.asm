# Simple ELF program for Linux x86

# Written by Paul Edwards
# Released the public domain

#ase86 -o elftest.o elftest.asm
# You should be able to use "-s" to strip symbols,
# and Linux is fine with that, but the PDOS/386
# loader currently requires them (should be changed)
# You shouldn't need the -Ttext, but for some reason
# you currently do
#lde86 -Ttext 400000 --emit-relocs -e myentry -o elftest.exe elftest.o
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

push $11
push $message
push $1
call __write

push $0
call __exita


# Currently you need \r if you want to run nicely
# under PDOS/386. And they seem to be harmless
# under Linux except for the fact that if you
# redirect the output, the output file contains
# the \r. Probably best to change PDOS/386

.globl message
message:
.string "hi\r\nthere\r\n"
