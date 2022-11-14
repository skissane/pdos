; int13.asm - do an interrupt 13 for disk services
; 
; This program written by Paul Edwards
; Released to the public domain

% .model memodel, c

.code

public int13x
int13x proc uses ax bx cx dx si di ds es, \
            regsin:ptr, regsout:ptr, sregs:ptr

push ds; for restoration after interrupt

if @DataSize
  lds si, sregs
else
  mov si, sregs
endif

mov es, [si + 6]
push es ; new value for ds

mov es, [si + 0]

if @DataSize
  lds si, regsin
else
  mov si, regsin
endif

mov ax, word ptr [si + 0]
mov bx, word ptr [si + 2]
mov cx, word ptr [si + 4]
mov dx, word ptr [si + 6]
mov di, word ptr [si + 10]
mov si, word ptr [si + 8]

pop ds; load previously saved value for ds

int 013h

push es
push ds
push si
push ax
push bp

mov bp, sp
mov ax, [bp+10]; restore ds immediately, can't move without it
pop bp
mov ds, ax

if @DataSize
  lds si, regsout
else
  mov si, regsout
endif

pop ax
mov [si + 0], ax
mov [si + 2], bx
mov [si + 4], cx
mov [si + 6], dx
mov [si + 10], di
pop ax
mov [si + 8], ax ; si
mov word ptr [si + 12], 0
jnc xflagclear
mov word ptr [si + 12], 1
xflagclear:

if @DataSize
  lds si, sregs
else
  mov si, sregs
endif

pop ax
mov [si + 6], ax; restore ds
pop ax
mov [si + 0], ax ; restore es

pop ds  ; restore value saved over interrupt (but accessed directly already)
ret
int13x endp

end
