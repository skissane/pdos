; support.asm - assembler support functions for DOS
;
; This program written by Paul Edwards
; Released to the public domain

% .model memodel, c

ifdef SMALLERC
.386
endif

.code

public int86
ifdef SMALLERC
int86 proc uses eax ebx cx dx esi di ds es, \
           intnum:dword, regsin:ptr, regsout:ptr
else
int86 proc uses ax bx cx dx si di ds es, \
           intnum:word, regsin:ptr, regsout:ptr
endif

ifndef SMALLERC

if @DataSize
  lds si, regsin
else
  mov si, regsin
endif

else
  mov esi, regsin
  ror esi, 4
  mov ds, si
  shr esi, 28
endif

pushf
pop ax
and ax, 0f700h  ; clear relevant flag bits
push ax
popf

mov ax, word ptr [si + 0]
mov bx, word ptr [si + 2]
mov cx, word ptr [si + 4]
mov dx, word ptr [si + 6]
mov di, word ptr [si + 10]
mov si, word ptr [si + 8]

push bp

cmp intnum, 08h
jne not8
int 08h
jmp fintry
not8:

cmp intnum, 09h
jne not9
int 09h
jmp fintry
not9:

cmp intnum, 010h
jne not10
int 010h
jmp fintry
not10:

cmp intnum, 012h
jne not12
int 012h
jmp fintry
not12:

cmp intnum, 013h
jne not13
int 013h
jmp fintry
not13:

cmp intnum, 014h
jne not14
int 014h
jmp fintry
not14:

cmp intnum, 015h
jne not15
int 015h
jmp fintry
not15:

cmp intnum, 016h
jne not16
int 016h
jmp fintry
not16:

cmp intnum, 01Ah
jne not1A
int 01Ah
jmp fintry
not1A:

cmp intnum, 020h
jne not20
int 020h
jmp fintry
not20:

cmp intnum, 021h
jne not21
int 021h
jmp fintry
not21:

cmp intnum, 025h
jne not25
int 025h
jmp fintry
not25:

cmp intnum, 026h
jne not26
int 026h
jmp fintry
not26:

; Copied BIOS interrupts for PDOS-32.
cmp intnum, 0A0h
jne notA0
int 0A0h
jmp fintry
notA0:

cmp intnum, 0A3h
jne notA3
int 0A3h
jmp fintry
notA3:

cmp intnum, 0A4h
jne notA4
int 0A4h
jmp fintry
notA4:

cmp intnum, 0A5h
jne notA5
int 0A5h
jmp fintry
notA5:

cmp intnum, 0A6h
jne notA6
int 0A6h
jmp fintry
notA6:

cmp intnum, 0AAh
jne notAA
int 0AAh
jmp fintry
notAA:

; Copied BIOS IRQ handler interrupts for PDOS-32.
cmp intnum, 0B0h
jne notB0
int 0B0h
jmp fintry
notB0:

cmp intnum, 0B1h
jne notB1
int 0B1h
jmp fintry
notB1:

cmp intnum, 0BEh
jne notBE
int 0BEh
jmp fintry
notBE:

fintry:

pop bp
push si

push ax

mov ax, 0
jnc flagclear
mov ax, 1
flagclear:

push ax ; cflag
pushf

ifndef SMALLERC

if @DataSize
  lds si, regsout
else
  mov si, regsout
endif

else
  mov esi, regsout
  ror esi, 4
  mov ds, si
  shr esi, 28
endif

pop ax ; flags
mov word ptr [si + 14], ax

pop ax ; cflag
mov word ptr [si + 12], ax

pop ax

mov [si + 0], ax
mov [si + 2], bx
mov [si + 4], cx
mov [si + 6], dx
mov [si + 10], di
pop ax ; actually si
mov [si + 8], ax

ret

int86 endp


public int86x
ifdef SMALLERC
int86x proc uses eax ebx cx dx esi di ds es, \
           intnum:dword, regsin:ptr, regsout:ptr, sregs:ptr
else
int86x proc uses ax bx cx dx si di ds es, \
            intnum:word, regsin:ptr, regsout:ptr, sregs:ptr
endif

push ds; for restoration after interrupt

ifndef SMALLERC

if @DataSize
  lds si, sregs
else
  mov si, sregs
endif

else
  mov esi, sregs
  ror esi, 4
  mov ds, si
  shr esi, 28
endif


mov es, [si + 6]
push es ; new value for ds

mov es, [si + 0]

ifndef SMALLERC

if @DataSize
  lds si, regsin
else
  mov si, regsin
endif

else
  mov esi, regsin
  ror esi, 4
  mov ds, si
  shr esi, 28
endif

pushf
pop ax
and ax, 0f700h  ; clear relevant flag bits
push ax
popf

mov ax, word ptr [si + 0]
mov bx, word ptr [si + 2]
mov cx, word ptr [si + 4]
mov dx, word ptr [si + 6]
mov di, word ptr [si + 10]
mov si, word ptr [si + 8]

pop ds; load previously saved value for ds

cmp intnum, 08h
jne xnot8
int 08h
jmp xfintry
xnot8:

cmp intnum, 09h
jne xnot9
int 09h
jmp xfintry
xnot9:

cmp intnum, 010h
jne xnot10
int 010h
jmp xfintry
xnot10:

cmp intnum, 013h
jne xnot13
int 013h
jmp xfintry
xnot13:

cmp intnum, 014h
jne xnot14
int 014h
jmp xfintry
xnot14:

cmp intnum, 015h
jne xnot15
int 015h
jmp xfintry
xnot15:

cmp intnum, 016h
jne xnot16
int 016h
jmp xfintry
xnot16:

cmp intnum, 01Ah
jne xnot1A
int 01Ah
jmp xfintry
xnot1A:

cmp intnum, 021h
jne xnot21
int 021h
jmp xfintry
xnot21:

cmp intnum, 025h
jne xnot25
int 025h
jmp xfintry
xnot25:

cmp intnum, 026h
jne xnot26
int 026h
jmp xfintry
xnot26:

; Copied BIOS interrupts for PDOS-32.
cmp intnum, 0A0h
jne xnotA0
int 0A0h
jmp xfintry
xnotA0:

cmp intnum, 0A3h
jne xnotA3
int 0A3h
jmp xfintry
xnotA3:

cmp intnum, 0A4h
jne xnotA4
int 0A4h
jmp xfintry
xnotA4:

cmp intnum, 0A5h
jne xnotA5
int 0A5h
jmp xfintry
xnotA5:

cmp intnum, 0A6h
jne xnotA6
int 0A6h
jmp xfintry
xnotA6:

cmp intnum, 0AAh
jne xnotAA
int 0AAh
jmp xfintry
xnotAA:

; Copied BIOS IRQ handler interrupts for PDOS-32.
cmp intnum, 0B0h
jne xnotB0
int 0B0h
jmp xfintry
xnotB0:

cmp intnum, 0B1h
jne xnotB1
int 0B1h
jmp xfintry
xnotB1:

xfintry:

push es
push ds
push si
push ax
push bp

mov bp, sp
mov ax, [bp+10]; restore ds immediately, can't move without it
pop bp
mov ds, ax

mov ax, 0
jnc xflagclear
mov ax, 1
xflagclear:

push ax ; cflag
pushf

ifndef SMALLERC

if @DataSize
  lds si, regsout
else
  mov si, regsout
endif

else
  mov esi, regsout
  ror esi, 4
  mov ds, si
  shr esi, 28
endif

pop ax ; flags
mov word ptr [si + 14], ax

pop ax ; cflag
mov word ptr [si + 12], ax

pop ax
mov [si + 0], ax
mov [si + 2], bx
mov [si + 4], cx
mov [si + 6], dx
mov [si + 10], di
pop ax
mov [si + 8], ax ; si

ifndef SMALLERC

if @DataSize
  lds si, sregs
else
  mov si, sregs
endif

else
  mov esi, sregs
  ror esi, 4
  mov ds, si
  shr esi, 28
endif

pop ax
mov [si + 6], ax; restore ds
pop ax
mov [si + 0], ax ; restore es

pop ds  ; restore value saved over interrupt (but accessed directly already)

ret

int86x endp


end
