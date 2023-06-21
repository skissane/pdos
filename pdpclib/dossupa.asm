; dossupa.asm - assembler support functions for DOS
;
; This program written by Paul Edwards
; Released to the public domain

; Note that a "proc" with an argument (parameter to the
; proc) will do a "push bp" and "mov bp,sp" itself, and
; will pop the bp at function exit.

% .model memodel, c

extrn __divide:proc
extrn __modulo:proc
ifdef WATCOMC
extrn __addhpi:proc
extrn __subhpi:proc
extrn __subhphp:proc
extrn __cmphphp:proc
endif

public fidrqq
public fiwrqq
public fierqq

.data

fidrqq  dw  ?
fiwrqq  dw  ?
fierqq  dw  ?

ifdef MSC
public _AHINCR
public _AHSHIFT
_AHINCR dw 1000H
_AHSHIFT dw 12
endif

.code


; The PDOS86 OS, when linking with PDPCLIB, uses the application-level
; routines in pdossupc instead, which do not do interrupts but instead
; link directly to the OS

ifndef PDOS86

; extern int CTYP __open(const char *filename, int mode, int *errind);

public __open
__open proc uses bx dx ds, \
            filename:ptr, mode:word, errind:ptr

if @DataSize
lds dx, filename
else
mov dx, filename
endif

mov al,byte ptr mode

mov ah, 3dh
int 21h

jc ___open1
if @DataSize
lds bx, errind
else
mov bx, errind
endif

mov word ptr [bx], 0
jmp short ___open2

___open1:
if @DataSize
lds bx, errind
else
mov bx, errind
endif
mov word ptr [bx], 1

___open2:

ret
__open endp


; extern int CTYP __creat(const char *filename, int mode, int *errind);

public __creat
__creat proc uses bx dx ds, \
             filename:ptr, mode:word, errind:ptr

if @DataSize
lds dx, filename
else
mov dx, filename
endif

mov cx,mode

mov ah, 3ch
int 21h

jc ___creat1

if @DataSize
lds bx, errind
else
mov bx, errind
endif
mov word ptr [bx], 0
jmp short ___creat2

___creat1:
if @DataSize
lds bx, errind
else
mov bx, errind
endif
mov word ptr [bx], 1

___creat2:

ret
__creat endp


; extern int CTYP __read(int handle, void *buf, unsigned int len, int *errind);

public __read
__read proc uses bx cx dx ds, \
            handle:word, buf:ptr, len:word, errind:ptr

mov bx,handle
if @DataSize
lds dx, buf
else
mov dx, buf
endif
mov cx,len

mov ah, 3fh
int 21h

jc ___read1

if @DataSize
lds bx, errind
else
mov bx, errind
endif
mov word ptr [bx], 0
jmp short ___read2

___read1:
if @DataSize
lds bx, errind
else
mov bx, errind
endif
mov word ptr [bx], 1

___read2:

ret
__read endp


; extern int CTYP __write(int handle, const void *buf, unsigned int len,
;                         int *errind);

public __write
__write proc uses bx cx dx ds, \
             handle:word, buf:ptr, len:word, errind:ptr

mov bx,handle

if @DataSize
lds dx, buf
else
mov dx, buf
endif

mov cx, len

mov ah, 40h
int 21h

jc ___write1

if @DataSize
lds bx, errind
else
mov bx, errind
endif

mov word ptr [bx], 0
jmp short ___write2

___write1:
if @DataSize
lds bx, errind
else
mov bx, errind
endif
mov word ptr [bx], 1

___write2:

ret
__write endp


; extern int CTYP __seek(int handle, long offset, int whence);

public __seek
__seek proc uses bx dx cx, \
            handle:word, offs:dword, whence:word

mov bx, handle
mov dx, word ptr offs
mov cx, word ptr offs + 2
mov al, byte ptr whence

mov ah, 042h
int 021h

jc ___seek1
mov ax, 0
___seek1:

ret
__seek endp


; extern void CTYP __close(int handle);

public __close
__close proc uses bx, handle:word

mov bx,handle

mov ah, 03eh
int 021h

ret
__close endp


; extern void CTYP __devginfo(int handle, unsigned int *info);

public __devginfo
__devginfo proc uses ds bx dx, handle:word, info:ptr

mov ah, 044h
mov al, 0h
mov bx, handle

int 021h

if @DataSize
lds bx, info
else
mov bx, info
endif

mov word ptr [info], dx

ret
__devginfo endp


; extern void CTYP __devsinfo(int handle, unsigned int info);

public __devsinfo
__devsinfo proc uses bx dx, handle:word, info:word

mov ah, 044h
mov al, 01h
mov bx, handle
mov dx, info

int 021h

ret
__devsinfo endp


; extern void CTYP __remove(const char *filename);

public __remove
__remove proc uses ds dx, filename:ptr

if @DataSize
lds dx, filename
else
mov dx, filename
endif

mov ah, 041h
int 021h

ret
__remove endp


; extern void CTYP __rename(const char *old, const char *newnam);

public __rename
__rename proc uses ds dx es di, old:ptr, newnam:ptr

if @DataSize
lds dx, old
else
mov dx, old
endif

if @DataSize
les di, newnam
else
mov di, newnam
push ds
pop es
endif

mov ah, 056h
int 021h

ret
__rename endp


; void CTYP __allocmem(size_t size, void **ptr);

public __allocmem
ifdef __SZ4__
__allocmem proc uses bx dx ds cx, sz:dword, res:ptr
else
__allocmem proc uses bx dx ds, sz:word, res:ptr
endif

if @DataSize eq 0

; return NULL

mov bx, res
mov word ptr [bx], 0

ret

else

ifdef __SZ4__
mov ax, word ptr sz+2
mov cl, 12
shl ax, cl
mov bx, word ptr sz
else
mov bx,sz
mov ax,0
endif

shr bx,1
shr bx,1
shr bx,1
shr bx,1

add bx,1
add bx,ax

mov ah, 48h
int 21h

jnc allocok
mov ax, 0

allocok:

lds bx, res
mov word ptr [bx], 0
mov word ptr [bx+2], ax

ret

endif

__allocmem endp


public __freemem
__freemem proc uses es dx cx, buf:ptr

if @DataSize
les cx, buf
else
ret
endif

mov ah, 049h
int 21h

ret
__freemem endp


endif   ; PDOS86



public __setj
__setj proc jbuf:ptr

        push ds

if @DataSize
        mov ax, word ptr jbuf
        mov ds, word ptr jbuf + 2
else
        mov ax, jbuf
endif
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
if @CodeSize
        mov ax, [bp + 4]     ; return address
        mov [bx + 16], ax    ; return address
endif

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

        ret
__setj endp


public __longj
__longj proc jbuf:ptr

if @DataSize
        mov bx, word ptr jbuf
        mov ds, word ptr jbuf + 2
else
        mov bx, jbuf
endif
        mov bp, [bx + 10]
        mov sp, bp
        mov bp, [bx + 12]
        pop ax               ; position of old bx
        pop ax               ; position of old ds
        pop ax               ; position of old bp
        pop ax               ; position of old return address
        pop ax               ; position of old return address

if @CodeSize
        mov ax, [bx + 16]    ; return address
        push ax
endif
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

        ret
__longj endp


ifndef PDOS86

; int CTYP __exec(char *cmd, void *env);
; full path, parm block

public __exec
__exec proc uses ds dx es bx cx si di, path:ptr, env:ptr

if @DataSize
lds dx, path
les bx, env
else
mov dx, path
mov bx, env
mov es, dx
endif

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

ret
__exec endp


; get return code/errorlevel
public __getrc
__getrc proc

mov ah, 04dh
int 21h

ret
__getrc endp


; void CTYP __datetime(void *ptr);

public __datetime
__datetime proc uses ds dx cx bx, buf:ptr

if @DataSize
lds bx, buf
else
mov bx, buf
endif

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

ret
__datetime endp


endif   ; PDOS86


ifdef WATCOMC
; divide dx:ax by cx:bx, result in dx:ax
; remainder in cx:bx I think
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
call f_lumod@
mov cx, dx
mov bx, ax
call f_ludiv@

ret
_U4D endp
endif


ifdef WATCOMC
; increment address dx:ax by cx:bx, normalized result in dx:ax
public _PIA
_PIA proc

; need to preserve these, as Watcom C apparently doesn't
push bx
push cx

push bx
push cx
push ax
push dx

call __addhpi
add sp,8

pop cx
pop bx

ret
_PIA endp
endif


ifdef WATCOMC
; decrement address dx:ax by cx:bx, normalized result in dx:ax
public _PIS
_PIS proc

; need to preserve these, as Watcom C apparently doesn't
push bx
push cx

push bx
push cx
push ax
push dx

call __subhpi
add sp,8

pop cx
pop bx

ret
_PIS endp
endif


ifdef WATCOMC
; subtract address dx:ax with address cx:bx, number of bytes in dx:ax
public _PTS
_PTS proc

; need to preserve these, as Watcom C apparently doesn't
push bx
push cx

push bx
push cx
push ax
push dx

call __subhphp
add sp,8

pop cx
pop bx

ret
_PTS endp
endif


ifdef WATCOMC
; compare address dx:ax to cx:bx
public _PTC
_PTC proc

; need to preserve these, as Watcom C apparently doesn't
push bx
push cx

push bx
push cx
push ax
push dx

call __cmphphp
add sp,8

pop cx
pop bx

; ax will be set to 1, 2 or 3
; so there is no issues with respect to whether a
; signed or unsigned compare is being done
cmp ax, 2H

ret
_PTC endp
endif


; must release stack space in this procedure
; original value and divisor will be on stack
; return value in dx:ax
public f_ludiv@
f_ludiv@ proc
push bp
mov bp,sp

; Watcom seems to be allowed to trash bx and cx,
; and we call a C routine later
push bx
push cx

cmp word ptr [bp + 10 + @CodeSize * 2], 0
jne ludiv_full

mov ax, [bp + 6 + @CodeSize * 2]
mov dx, 0
div word ptr [bp + 8 + @CodeSize * 2]
mov bx, ax
mov ax, [bp + 4 + @CodeSize * 2]
div word ptr [bp + 8 + @CodeSize * 2]

mov dx, bx
jmp short ludiv_fin

ludiv_full:
push word ptr [bp + 10 + @CodeSize * 2]
push word ptr [bp + 8 + @CodeSize * 2]
push word ptr [bp + 6 + @CodeSize * 2]
push word ptr [bp + 4 + @CodeSize * 2]

call __divide

add sp, 8

ludiv_fin:

pop cx
pop bx
pop bp
ret 8
f_ludiv@ endp


; dx:ax divided by cx:bx, result in dx:ax, module on cx:bx

ifdef WATCOMC
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
call f_lmod@
mov cx, dx
mov bx, ax
call f_ldiv@
ret
_I4D endp
endif


; must release stack space in this procedure
public f_ldiv@
f_ldiv@ proc
push bp
mov bp,sp

; I don't know why we need to preserve dx. I expected
; the return value to be a dx:ax pair. Maybe it works
; in conjunction with the f_lmod call, and the return
; value is only ax
push dx

; Watcom seems to be allowed to trash bx and cx,
; and we call a C routine later
push bx
push cx

cmp word ptr [bp + 10 + @CodeSize * 2], 0
jne ldiv_full

mov ax,[bp+4+@CodeSize*2]
mov dx,[bp+6+@CodeSize*2]
idiv word ptr [bp+8+@CodeSize*2]
jmp short ldiv_fin

ldiv_full:
push word ptr [bp + 10 + @CodeSize * 2]
push word ptr [bp + 8 + @CodeSize * 2]
push word ptr [bp + 6 + @CodeSize * 2]
push word ptr [bp + 4 + @CodeSize * 2]

call __divide

add sp, 8

ldiv_fin:

pop cx
pop bx
pop dx
pop bp
ret 8
f_ldiv@ endp


public f_lmod@
f_lmod@ proc
push bp
mov bp,sp

; Watcom seems to be allowed to trash bx and cx,
; and we call a C routine later
push bx
push cx

cmp word ptr [bp + 10 + @CodeSize * 2], 0
jne lmod_full

mov ax,[bp+4+@CodeSize*2]
mov dx,[bp+6+@CodeSize*2]
idiv word ptr [bp+8+@CodeSize*2]
mov ax,dx
mov dx,0
jmp short lmod_fin

lmod_full:
push word ptr [bp + 10 + @CodeSize * 2]
push word ptr [bp + 8 + @CodeSize * 2]
push word ptr [bp + 6 + @CodeSize * 2]
push word ptr [bp + 4 + @CodeSize * 2]

call __modulo

add sp, 8

lmod_fin:

pop cx
pop bx
pop bp
ret 8
f_lmod@ endp


; procedure needs to fix up stack
; original value and divisor will be on stack
; return in dx:ax
public f_lumod@
f_lumod@ proc
push bp
mov bp,sp

; Watcom seems to be allowed to trash bx and cx,
; and we call a C routine later
push bx
push cx

cmp word ptr [bp + 10 + @CodeSize * 2], 0
jne lumod_full

mov ax, [bp + 6 + @CodeSize * 2]
mov dx, 0
div word ptr [bp + 8 + @CodeSize * 2]
mov ax, [bp + 4 + @CodeSize * 2]
div word ptr [bp + 8 + @CodeSize * 2]
mov ax,dx
mov dx, 0
jmp short lumod_fin

lumod_full:
push word ptr [bp + 10 + @CodeSize * 2]
push word ptr [bp + 8 + @CodeSize * 2]
push word ptr [bp + 6 + @CodeSize * 2]
push word ptr [bp + 4 + @CodeSize * 2]

call __modulo

add sp, 8

lumod_fin:

pop cx
pop bx
pop bp
ret 8
f_lumod@ endp


; multiply cx:bx by dx:ax, result in dx:ax

public _I4M
_I4M:
public _U4M
_U4M:
public f_lxmul@
f_lxmul@ proc uses cx bx si di

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

ret
f_lxmul@ endp


; shift dx:ax left by cl

ifdef MSC
public _aFlshl
_aFlshl:
endif

public f_lxlsh@
f_lxlsh@ proc uses bx

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

ret
f_lxlsh@ endp


; shift dx:ax right by cl

ifdef MSC
public _aFulshr
_aFulshr:
endif

public f_lxursh@
f_lxursh@ proc uses bx

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

ret
f_lxursh@ endp


; this procedure needs to fix up the stack
; can't use "uses" keyword until proc knows about parameters
public f_scopy@
f_scopy@ proc

push bp
mov bp, sp
push cx
push ds
push es
push si
push di
lds si, [bp + 4 + @CodeSize * 2]
les di, [bp + 8 + @CodeSize * 2]
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

ifdef WATCOMC
public _CHP
_CHP proc
ret
_CHP endp
endif

public _brkpoint_
_brkpoint_:
public __brkpoint
__brkpoint proc
int 3 ; this is an x'cc' which will need to be zapped with x'90'
      ; before you can progress
nop   ; people can zap this to x'cc' too if they want
ret
__brkpoint endp

; It is convenient to have a second breakpoint function for now
public __brkpoint2
__brkpoint2 proc
int 3 ; this is an x'cc' which will need to be zapped with x'90'
      ; before you can progress
nop
ret
__brkpoint2 endp


; Display an integer (PDOS/86 extension)

public __dint
__dint proc uses ax cx, myint:word
mov cx, myint
mov ah, 0f6h
mov al, 0
int 21h
ret
__dint endp


end
