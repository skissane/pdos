; ploadst.asm - startup code for pload
; 
; This program written by Paul Edwards
; Released to the public domain

% .model memodel, c

extrn dstart:proc

extrn _end:byte
extrn _edata:byte

ifndef MAKECOM
.stack 1000h
endif

.code

ifdef MAKECOM
org 0100h
endif

ifdef MAKEPL
org 0700h
endif

top:

public _startup
_startup proc

; signify MSDOS version 5 ("sys.com" looks for this byte at offset 3
; in the io.sys file).
nop
nop
mov ax, 0005h

; for standard startup, comment out this line
jmp bootstrap

standard:
; put this in in case we're running as an executable
push cs
push cs
pop ds
pop es
; free initially allocated memory
mov bx, 01111h
mov ah, 4ah
int 21h
jmp bypass

; for bootstrap
bootstrap:

ifdef NEWMODEL
mov ax, 0

else
push cs
pop ax
sub ax, 010h
endif

lea bx, newstart
push ax
push bx
retf ; should return to next instruction
newstart:

mov bx, dx ; preserve drive number


; relocate the first half (256 bytes) of the VBR - the bit that contains
; the BPB - to address 0x600 before it gets clobbered
; Do this before setting sp, to give flexibility of setting a value of
; sp that may clobber 7c00
; no need to preserve any of the below registers

mov dx, 0
mov ds, dx
mov es, dx
mov si,07c00h       ; Set source and destination
mov di,0600h
mov cx,080h         ; 0x80 words = 256 bytes
rep movsw           ; Copy bpb etc to 0x0600



ifdef NEWMODEL
mov dx, DGROUP
mov ds,dx
mov es,dx
mov ss, dx

; max out the stack, because it will potentially be shared
; by pdos.sys
mov sp, 0fffeh

else
push ax
push ax
pop es
pop ds
mov ss, ax
mov sp, 0fffeh
endif


bypass:



; see "nop" further down
;call displayc

; We need to call dstart instead of __start because we're
; not linking in the C library. And dstart needs to go into
; the main C code so that it gets the name mangling correct.
; It can be put into another source file if main is declared
; with __watcall

; This is where we need to load up to
ifdef NEWMODEL
push ds
mov di, offset DGROUP:_edata
push di
endif

push bx ; drive number

call dstart

add sp, 6


; Note that if we actually return here, we're screwed. So
; just loop

dmyloop:
hlt
jmp dmyloop



; Under more normal circumstances we could do something like below

mov ax, 0
push ax
call _exita
_startup endp


; This nop is required for the same reason as displayc needs it
nop ; for good measure

public clrbss
clrbss proc

; we are responsible for clearing our own BSS
; in Watcom at least, the BSS is at the end of the DGROUP
; which can be referenced as _end, and the end of the
; DATA section is referenced as _edata
; We can use rep stos to clear the memory, which initializes
; using the byte in al, starting at es:di, for a length of cx

push es
push cx
push di
push ax


; temporary eyecatcher
nop
nop
nop
nop
nop


mov cx, offset DGROUP:_end
mov di, offset DGROUP:_edata

sub cx, di

; putting these in instead of the sub doesn't solve the problem
;db 02bh
;db 0cfh

; move ds (data pointer, often DGROUP, into es)
push ds
pop es
mov al, 0
rep stosb


pop ax
pop di
pop cx
pop es

ret

clrbss endp



; There appears to be a bug in wasm in Open Watcom 1.6
; where references to displayc actually hit the byte
; before. At least with current alignment. But adding
; a "nop" safeguards against that.
nop

ifdef NEED_DISPLAYC
;display a 'C' just to let us know that it's working!
public displayc
displayc proc
push ax
push bx
mov ah, 0eh
mov al, 043h
mov bl, 0
mov bh, 0
int 10h
pop bx
pop ax
ret
displayc endp
endif

public _exita
_exita proc
;myloop:
;jmp myloop
pop ax
mov ah,4ch
int 21h ; terminate
_exita endp

ifdef WATCOM
ifndef NEWMODEL
public cstart_
cstart_:
ret
endif
endif

; Two routines needed for SubC
public xgetfar

xgetfar proc, xaddr: dword
        push ds
        push bx
        lds bx,xaddr
        mov ah,0
        mov al,[bx]
        pop bx
        pop ds
        ret
xgetfar endp

public xputfar

xputfar proc, address2: dword, character: word
        push ds
        push bx
        lds bx,address2           ;We receive a word yet
        mov al,byte ptr character ;we only put a char.
        mov [bx],al
        pop bx
        pop ds
        ret
xputfar endp


end top
