; dossupa.asm - assembler support functions for DOS
;
; This program written by Paul Edwards
; Released to the public domain

% .model memodel, c

extrn __divide:proc
extrn __modulo:proc

public fidrqq
public fiwrqq
public fierqq

.data

fidrqq  dw  ?
fiwrqq  dw  ?
fierqq  dw  ?

.code

public __open
__open proc
push bp
mov bp, sp

push bx
push dx
push ds

mov dx,[bp+8]
mov ds,dx
mov dx,[bp+6]
mov al,[bp+10]

mov ah, 3dh
int 21h

jc ___open1
mov dx,[bp+14]
mov ds,dx
mov bx,[bp+12]
mov word ptr [bx], 0
jmp short ___open2
___open1:
mov dx,[bp+14]
mov ds,dx
mov bx,[bp+12]
mov word ptr [bx], 1
___open2:
pop ds
pop dx
pop bx

pop bp
ret
__open endp


public __creat
__creat proc
push bp
mov bp, sp

push bx
push dx
push ds

mov dx,[bp+8]
mov ds,dx
mov dx,[bp+6]
mov cx,[bp+10]

mov ah, 3ch
int 21h

jc ___creat1
mov dx,[bp+14]
mov ds,dx
mov bx,[bp+12]
mov word ptr [bx], 0
jmp short ___creat2
___creat1:
mov dx,[bp+14]
mov ds,dx
mov bx,[bp+12]
mov word ptr [bx], 1
___creat2:
pop ds
pop dx
pop bx

pop bp
ret
__creat endp


public __read
__read proc
push bp
mov bp,sp

push bx
push cx
push dx
push ds

mov bx,[bp+6]
mov dx,[bp+10]
mov ds,dx
mov dx,[bp+8]
mov cx,[bp+12]

mov ah, 3fh
int 21h

jc ___read1
mov dx,[bp+16]
mov ds,dx
mov bx,[bp+14]
mov word ptr [bx], 0
jmp short ___read2
___read1:
mov dx,[bp+16]
mov ds,dx
mov bx,[bp+14]
mov word ptr [bx], 1
___read2:

pop ds
pop dx
pop cx
pop bx

pop bp
ret
__read endp


public __write
__write proc
push bp
mov bp,sp

push bx
push cx
push dx
push ds

mov bx,[bp+6]
mov dx,[bp+10]
mov ds,dx
mov dx,[bp+8]
mov cx,[bp+12]

mov ah, 40h
int 21h

jc ___write1
mov dx,[bp+16]
mov ds,dx
mov bx,[bp+14]
mov word ptr [bx], 0
jmp short ___write2
___write1:
mov dx,[bp+16]
mov ds,dx
mov bx,[bp+14]
mov word ptr [bx], 1
___write2:

pop ds
pop dx
pop cx
pop bx

pop bp
ret
__write endp


; seek - handle, offset, type

public __seek
__seek proc
push bp
mov bp, sp
push bx
push dx
push cx

mov bx, [bp + 6]
mov dx, [bp + 8]
mov cx, [bp + 10]
mov al, [bp + 12]

mov ah, 042h
int 021h

jc ___seek1
mov ax, 0
___seek1:

pop cx
pop dx
pop bx
pop bp
ret
__seek endp


public __close
__close proc
push bp
mov bp, sp
push bx

mov bx,[bp+6]

mov ah, 03eh
int 021h

pop bx
pop bp
ret
__close endp


public __remove
__remove proc
push bp
mov bp, sp
push ds
push dx

mov dx, [bp + 6]
mov ds, [bp + 8]

mov ah, 041h
int 021h

pop dx
pop ds
pop bp
ret
__remove endp


public __rename
__rename proc
push bp
mov bp, sp
push ds
push dx
push es
push di

mov dx, [bp + 6]
mov ds, [bp + 8]
mov di, [bp + 10]
mov es, [bp + 12]

mov ah, 056h
int 021h

pop di
pop es
pop dx
pop ds
pop bp
ret
__rename endp



public __allocmem
__allocmem proc
push bp
mov bp,sp

push bx
push dx
push ds

mov bx,[bp+6]

shr bx,1
shr bx,1
shr bx,1
shr bx,1

add bx,1
mov ah, 48h
int 21h

jnc allocok
mov ax, 0
allocok:
mov dx,[bp+10]
mov ds,dx
mov bx,[bp+8]

mov word ptr [bx], 0
mov word ptr [bx+2], ax

pop ds
pop dx
pop bx
pop bp
ret
__allocmem endp


public __freemem
__freemem proc
push bp
mov bp,sp
push es
push dx
push cx

mov dx,[bp+6]
mov cx,[bp+8]

shr dx, 1
shr dx, 1
shr dx, 1
shr dx, 1

add cx, dx
mov es, cx
mov ah, 049h
int 21h

pop cx
pop dx
pop es
pop bp
ret
__freemem endp


public __setj
__setj proc
        push bp
        mov bp,sp
        push ds

        mov ax, [bp + 6]
        mov ds, [bp + 8]
        push bx
        push bp
        mov bp, sp
        mov bx, bp
        pop bp
        push bx               ; sp

        mov bx,ax
        mov [bx + 2], cx
        mov [bx + 4], dx
        mov [bx + 6], di
        mov [bx + 8], si

        pop ax
        mov [bx + 10], bx    ; sp
        mov ax, [bp + 0]     ; bp
        mov [bx + 12], ax    ; bp

        mov ax, [bp + 2]     ; return address
        mov [bx + 14], ax    ; return address
        mov ax, [bp + 4]     ; return address
        mov [bx + 16], ax    ; return address

        pop ax               ; bx
        mov [bx + 0], ax
        pop ax               ; ds
        push ax
        mov [bx + 18], ax
        mov ax, es
        mov [bx + 20], ax    ; es
        mov ax, [bx + 0]     ; bx
        mov bx, ax

        mov ax, 0

        pop ds
        pop bp
        ret
__setj endp


public __longj
__longj proc
        push bp
        mov bp, sp

        mov bx, [bp + 6]
        mov ds, [bp + 8]
        mov bp, [bx + 10]
        mov sp, bp
        mov bp, [bx + 12]
        pop ax               ; position of old bx
        pop ax               ; position of old ds
        pop ax               ; position of old bp
        pop ax               ; position of old return address
        pop ax               ; position of old return address

        mov ax, [bx + 16]    ; return address
        push ax
        mov ax, [bx + 14]    ; return address
        push ax

        mov ax, [bx + 12]    ; bp saved as normal
        push ax

        mov cx, [bx + 2]
        mov dx, [bx + 4]
        mov di, [bx + 6]
        mov si, [bx + 8]

        mov ax, [bx + 0]
        push ax              ; bx
        mov ax, [bx + 18]    ; ds
        push ax
        mov ax, [bx + 20]    ; es
        mov es, ax
        mov ax, [bx + 22]    ; return value
        pop ds
        pop bx

        pop bp
        ret
__longj endp


; full path, parm block
public __exec
__exec proc
push bp
mov bp, sp
push ds
push dx
push es
push bx
push cx
push si
push di

mov dx, [bp + 6]
mov ds, [bp + 8]
mov bx, [bp + 10]
mov es, [bp + 12]

jmp short bypass
dummy1 dw ?
dummy2 dw ?
bypass:

mov dummy1, sp
mov dummy2, ss

mov al, 0
mov ah, 04bh
cld              ; dos 2 "bug"
int 21h

jc ___exec1
mov ax, 0
___exec1:

cli
mov ss, dummy2
mov sp, dummy1
sti

pop di
pop si
pop cx
pop bx
pop es
pop dx
pop ds
pop bp
ret
__exec endp


; get return code/errorlevel
public __getrc
__getrc proc
push bp
mov bp, sp

mov ah, 04dh
int 21h

pop bp
ret
__getrc endp


public __datetime
__datetime proc
push bp
mov bp, sp
push ds
push dx
push cx
push bx

mov bx, [bp + 6]
mov ds, [bp + 8]

mov ah, 02ah
int 021h

mov [bx + 0], cx
mov ch, 0
mov cl, dh
mov [bx + 2], cx
mov dh, 0
mov [bx + 4], dx

mov ah, 02ch
int 021h

mov ah, 0
mov al, ch
mov [bx + 6], ax
mov ch, 0
mov [bx + 8], cx
mov ah, 0
mov al, dh
mov [bx + 10], ax
mov dh, 0
mov [bx + 12], dx

pop bx
pop cx
pop dx
pop ds
pop bp
ret
__datetime endp


ifdef WATCOM
; divide dx:ax by cx:bx, result in dx:ax
public _U4D
_U4D proc
push cx
push bx
push dx
push ax
push cx
push bx
push dx
push ax
call far ptr f_lumod@
mov cx, dx
mov bx, ax
call far ptr f_ludiv@
ret
_U4D endp
endif


public f_ludiv@
f_ludiv@ proc far
push bp
mov bp,sp
push bx

cmp word ptr [bp + 12], 0
jne ludiv_full

mov ax, [bp + 8]
mov dx, 0
div word ptr [bp + 10]
mov bx, ax
mov ax, [bp + 6]
div word ptr [bp + 10]

mov dx, bx
jmp short ludiv_fin

ludiv_full:
push word ptr [bp + 12]
push word ptr [bp + 10]
push word ptr [bp + 8]
push word ptr [bp + 6]
call far ptr __divide
add sp, 8

ludiv_fin:

pop bx
pop bp
ret 8
f_ludiv@ endp


; dx:ax divided by cx:bx, result in dx:ax, module on cx:bx

ifdef WATCOM
public _I4D
_I4D proc
push cx
push bx
push dx
push ax
push cx
push bx
push dx
push ax
call far ptr f_lmod@
mov cx, dx
mov bx, ax
call far ptr f_ldiv@
ret
_I4D endp
endif


; must release stack space in this procedure
public f_ldiv@
f_ldiv@ proc
push bp
mov bp,sp
push dx

cmp word ptr [bp + 12], 0
jne ldiv_full

mov ax,[bp+6]
mov dx,[bp+8]
idiv word ptr [bp+10]
jmp short ldiv_fin

ldiv_full:
push word ptr [bp + 12]
push word ptr [bp + 10]
push word ptr [bp + 8]
push word ptr [bp + 6]
call far ptr __divide
add sp, 8

ldiv_fin:

pop dx
pop bp
ret 8
f_ldiv@ endp


public f_lmod@
f_lmod@ proc
push bp
mov bp,sp

cmp word ptr [bp + 12], 0
jne lmod_full

mov ax,[bp+6]
mov dx,[bp+8]
idiv word ptr [bp+10]
mov ax,dx
mov dx,0
jmp short lmod_fin

lmod_full:
push word ptr [bp + 12]
push word ptr [bp + 10]
push word ptr [bp + 8]
push word ptr [bp + 6]
call far ptr __modulo
add sp, 8

lmod_fin:

pop bp
ret 8
f_lmod@ endp


; procedure needs to fix up stack
public f_lumod@
f_lumod@ proc
push bp
mov bp,sp

cmp word ptr [bp + 12], 0
jne lumod_full

mov ax, [bp + 8]
mov dx, 0
div word ptr [bp + 10]
mov ax, [bp + 6]
div word ptr [bp + 10]
mov ax,dx
mov dx, 0
jmp short lumod_fin

lumod_full:
push word ptr [bp + 12]
push word ptr [bp + 10]
push word ptr [bp + 8]
push word ptr [bp + 6]
call far ptr __modulo
add sp, 8

lumod_fin:

pop bp
ret 8
f_lumod@ endp


; multiply cx:bx by dx:ax, result in dx:ax

public _I4M
_I4M:
public _U4M
_U4M:
public f_lxmul@
f_lxmul@ proc
push cx
push bx
push si
push di

; Code provided by Terje Mathisen
mov si,ax
mov di,dx
mul cx ;; hi * lo
xchg ax,di ;; First mul saved, grab org dx
mul bx ;; lo * hi
add di,ax ;; top word of result

mov ax,si ;; retrieve original AX
mul bx ;; lo * lo
add dx,di

pop di
pop si
pop bx
pop cx
ret
f_lxmul@ endp


; shift dx:ax left by cl

public f_lxlsh@
f_lxlsh@ proc
push bx

cmp cl, 24
jl lxlsh_16
mov dh, al
mov dl, 0
mov ax, 0
sub cl, 24
jmp short lxlsh_last

lxlsh_16:
cmp cl, 16
jl lxlsh_8
mov dx, ax
mov ax, 0
sub cl, 16
jmp short lxlsh_last

lxlsh_8:
cmp cl, 8
jl lxlsh_last
mov dh, dl
mov dl, ah
mov ah, al
mov al, 0
sub cl, 8
;jmp short lxlsh_last

lxlsh_last:

mov ch, 8
sub ch, cl
xchg ch, cl
mov bx, ax
shr bx, cl
xchg ch, cl
shl dx, cl
or dl, bh
shl ax, cl

pop bx
ret
f_lxlsh@ endp


; shift dx:ax right by cl

public f_lxursh@
f_lxursh@ proc
push bx

cmp cl, 24
jl lxursh_16
mov al, dh
mov ah, 0
mov dx, 0
sub cl, 24
jmp short lxursh_last

lxursh_16:
cmp cl, 16
jl lxursh_8
mov ax, dx
mov dx, 0
sub cl, 16
jmp short lxursh_last

lxursh_8:
cmp cl, 8
jl lxursh_last
mov al, ah
mov ah, dl
mov dl, dh
mov dh, 0
sub cl, 8
;jmp short lxursh_last

lxursh_last:

mov ch, 8
sub ch, cl
xchg ch, cl
mov bx, dx
shl bx, cl
xchg ch, cl
shr ax, cl
or ah, bl
shr dx, cl

pop bx
ret
f_lxursh@ endp


; this procedure needs to fix up the stack
public f_scopy@
f_scopy@ proc

push bp
mov bp, sp
push cx
push ds
push es
push si
push di
lds si, [bp + 6]
les di, [bp + 10]
cld
rep movsb

pop di
pop si
pop es
pop ds
pop cx
pop bp
ret 8
f_scopy@ endp


public f_ftol@
f_ftol@ proc
ret
f_ftol@ endp

public ___SIN__
___SIN__ proc
ret
___SIN__ endp

public ___COS__
___COS__ proc
ret
___COS__ endp

ifdef WATCOM
public _CHP
_CHP proc
ret
_CHP endp
endif

end
