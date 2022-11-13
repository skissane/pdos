; dosstart.asm - startup code for C programs for DOS
;
; This program written by Paul Edwards
; Released to the public domain

.model large

extrn ___start:proc

extrn _end:byte
extrn _edata:byte

public ___psp
public ___envptr
public ___osver

.stack 1000h

.data

banner  db  "PDPCLIB"
___psp   dd  ?
___envptr dd ?
___osver dw ?

.code

top:

___intstart proc

; add some nops to create a cs-addressable save area, and also create a
; bit of an eyecatcher

nop
nop
nop
nop

; push the psp now, ready for calling start
push ds
mov ax, 0
push ax

; determine how much memory is needed. The stack pointer points
; to the top. Work out what segment that is, then subtract the
; starting segment (the PSP), and you have your answer.

mov ax, sp
mov cl, 4
shr ax, cl ; get sp into pages
mov bx, ss
add ax, bx
add ax, 2 ; safety margin because we've done some pushes etc
mov bx, es
sub ax, bx ; subtract the psp segment

; free initially allocated memory

mov bx, ax
mov ah, 4ah
int 21h

mov dx,DGROUP
mov ds,dx

; we are responsible for clearing our own BSS
; in Watcom at least, the BSS is at the end of the DGROUP
; which can be referenced as _end, and the end of the
; DATA section is referenced as _edata
; We can use rep stos to clear the memory, which initializes
; using the byte in al, starting at es:di, for a length of cx

push es
mov cx, offset DGROUP:_end
mov di, offset DGROUP:_edata
sub cx, di
mov es, dx ; we still have that set from above
mov al, 0
rep stosb
pop es

mov ah,30h
int 21h
xchg al,ah
mov [___osver],ax

mov word ptr ___psp, 0
mov word ptr [___psp + 2], es
mov word ptr ___envptr, 0
mov dx, es:[02ch]
mov word ptr [___envptr + 2], dx
mov dx, ds
mov es, dx

; we have already pushed the pointer to psp
call far ptr ___start
add sp, 4  ; delete psp from stack

push ax

; how do I get rid of the warning about "instruction can be compacted
; with override"?  The answer is certainly NOT to change the "far" to
; "near".
call far ptr ___exita
add sp, 2
ret
___intstart endp

public ___exita
___exita proc
push bp
mov bp, sp
mov ax, [bp + 6]
mov ah,4ch
int 21h ; terminate
pop bp
ret
___exita endp


public ___main
___main proc
ret
___main endp


end top
