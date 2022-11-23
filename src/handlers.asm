; Released to the public domain by Matthew Parker on 4 December 1995
; Mods by Paul Edwards, also public domain.
; For different models just change the .model directive

% .model memodel, c

public instint

extrn int0:proc
extrn int1:proc
extrn int3:proc
extrn int20:proc
extrn int21:proc
extrn int25:proc
extrn int26:proc

.data
addr0 dd handler0
addr1 dd handler1
addr3 dd handler3
addr20 dd handler20
addr21 dd handler21
addr25 dd handler25
addr26 dd handler26

        .code

instint proc uses bx es ds
        mov bx, 0
        push bx
        pop es
        cli

        push ds
        lds bx, addr0
        mov es:[0h], bx
        mov es:[02h], ds
        pop ds

        push ds
        lds bx, addr1
        mov es:[04h], bx
        mov es:[06h], ds
        pop ds

        push ds
        lds bx, addr3
        mov es:[0ch], bx
        mov es:[0eh], ds
        pop ds

        push ds
        lds bx, addr20
        mov es:[80h], bx
        mov es:[82h], ds
        pop ds

        push ds
        lds bx, addr21
        mov es:[84h], bx
        mov es:[86h], ds
        pop ds

        push ds
        lds bx, addr25
        mov es:[94h], bx
        mov es:[96h], ds
        pop ds

        push ds
        lds bx, addr26
        mov es:[98h], bx
        mov es:[9Ah], ds
        pop ds

        sti
        ret
instint endp

; the stack will already have
; flags
; cs
; ip

handler0 proc
        push bp
        push ax
        push ax   ; dummy, actually cflag storage
        push bx
        push cx
        push dx
        push si
        push di
        push ds
        push es

        mov dx, DGROUP
        mov ds, dx
        mov ax, sp
        push ss
        push ax
        call int0
        add sp, 4

        pop es
        pop ds
        pop di
        pop si
        pop dx
        pop cx
        pop bx
        pop ax   ; actually cflag

        cmp ax, 0
        je clear0
        jmp notclear0
clear0:
        pop ax
        push bp
        mov bp, sp
        and word ptr [bp+6],0fffeh
        pop bp
        pop bp
        iret
notclear0:
        pop ax
        push bp
        mov bp, sp
        or word ptr [bp+6],0001h
        pop bp
        pop bp
        iret
handler0 endp

handler1 proc
        push bp
        push ax
        push ax   ; dummy, actually cflag storage
        push bx
        push cx
        push dx
        push si
        push di
        push ds
        push es

        mov dx, DGROUP
        mov ds, dx
        mov ax, sp
        push ss
        push ax
        call int1
        add sp, 4

        pop es
        pop ds
        pop di
        pop si
        pop dx
        pop cx
        pop bx
        pop ax   ; actually cflag

; Don't alter carry flag for this interrupt
; There is some redundant code here that should be cleaned up
        pop ax
        push bp
        mov bp, sp
        pop bp
        pop bp
        iret
handler1 endp

handler3 proc
        push bp
        push ax
        push ax   ; dummy, actually cflag storage
        push bx
        push cx
        push dx
        push si
        push di
        push ds
        push es

        mov dx, DGROUP
        mov ds, dx
        mov ax, sp
        push ss
        push ax
        call int3
        add sp, 4

        pop es
        pop ds
        pop di
        pop si
        pop dx
        pop cx
        pop bx
        pop ax   ; actually cflag

; Don't alter carry flag for this interrupt
; There is some redundant code here that should be cleaned up
        pop ax
        push bp
        mov bp, sp
        pop bp
        pop bp
        iret
handler3 endp

handler20 proc
        push ax
        push ax   ; dummy, actually cflag storage
        push bx
        push cx
        push dx
        push si
        push di
        push ds
        push es

        mov dx, DGROUP
        mov ds, dx
        mov ax, sp
        push ss
        push ax
        call int20
        add sp, 4

        pop es
        pop ds
        pop di
        pop si
        pop dx
        pop cx
        pop bx
        pop ax   ; actually cflag

        cmp ax, 0
        je clear20
        jmp notclear20
clear20:
        pop ax
        push bp
        mov bp, sp
        and word ptr [bp+6],0fffeh
        pop bp
        iret
notclear20:
        pop ax
        push bp
        mov bp, sp
        or word ptr [bp+6],0001h
        pop bp
        iret
handler20 endp

handler21 proc
        push ax
        push ax   ; dummy, actually cflag storage
        push bx
        push cx
        push dx
        push si
        push di
        push ds
        push es

        mov dx, DGROUP
        mov ds, dx
        mov ax, sp
        push ss
        push ax
        call int21
        add sp, 4

        pop es
        pop ds
        pop di
        pop si
        pop dx
        pop cx
        pop bx
        pop ax   ; actually cflag

        cmp ax, 0
        je clear21
        jmp notclear21
clear21:
        pop ax
        push bp
        mov bp, sp
        and word ptr [bp+6],0fffeh
        pop bp
        iret
notclear21:
        pop ax
        push bp
        mov bp, sp
        or word ptr [bp+6],0001h
        pop bp
        iret
handler21 endp

handler25 proc
        push ax
        push ax   ; dummy, actually cflag storage
        push bx
        push cx
        push dx
        push si
        push di
        push ds
        push es

        mov dx, DGROUP
        mov ds, dx
        mov ax, sp
        push ss
        push ax
        call int25
        add sp, 4

        pop es
        pop ds
        pop di
        pop si
        pop dx
        pop cx
        pop bx
        pop ax   ; actually cflag

        cmp ax, 0
        je clear25
        jmp notclear25
clear25:
        pop ax
        push bp
        mov bp, sp
        and word ptr [bp+6],0fffeh
        pop bp
        iret
notclear25:
        pop ax
        push bp
        mov bp, sp
        or word ptr [bp+6],0001h
        pop bp
        iret
handler25 endp

handler26 proc
        push ax
        push ax   ; dummy, actually cflag storage
        push bx
        push cx
        push dx
        push si
        push di
        push ds
        push es

        mov dx, DGROUP
        mov ds, dx
        mov ax, sp
        push ss
        push ax
        call int26
        add sp, 4

        pop es
        pop ds
        pop di
        pop si
        pop dx
        pop cx
        pop bx
        pop ax   ; actually cflag

        cmp ax, 0
        je clear26
        jmp notclear26
clear26:
        pop ax
        push bp
        mov bp, sp
        and word ptr [bp+6],0fffeh
        pop bp
        iret
notclear26:
        pop ax
        push bp
        mov bp, sp
        or word ptr [bp+6],0001h
        pop bp
        iret
handler26 endp

end
