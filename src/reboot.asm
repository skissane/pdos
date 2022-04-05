; Released to the public domain Matthew Parker and Paul Edwards
; For different models just change the .model directive

; reboot - reboot computer
; works on MSDOS and clones

% .model memodel, c

public reboot

        .code

top:

reboot proc
        push es
        mov ax, 040h
        mov es, ax
        mov word ptr es:[072h], 01234h
        
; we want to do a jmp 0ffffh:0000h, but that seems not to assemble
        mov ax, 0ffffh
        push ax
        mov ax, 0
        push ax
        retf
        pop es ; shouldn't get here
        ret
reboot endp

end top
