; Released to the public domain by Matthew Parker on 4 December 1995
; Mods by Paul Edwards, also public domain.
; For different models just change the .model directive

% .model memodel, c

public getfar, putfar, rportb, wportb, enable, disable
public callfar, callwithpsp, callwithbypass, a20e
public reboot, putabs, getabs, poweroff, boot

        .code
getfar proc, address: dword
        push ds
        push bx
        lds bx,address
        mov ah,0
        mov al,[bx]
        pop bx
        pop ds
        ret
getfar endp

putfar proc, address2: dword, character: word
        push ds
        push bx
        lds bx,address2           ;We recieve a word yet
        mov al,byte ptr character ;we only put a char.
        mov [bx],al
        pop bx
        pop ds
        ret
putfar endp

rportb proc, port: word
        push dx
        mov dx,port
        in al,dx
        pop dx
        ret
rportb endp

wportb proc, port2: word, outbyte: byte
        push dx
        mov dx,port2
        mov al,outbyte
        out dx,al
        pop dx
        ret
wportb endp

enable  proc 
        sti
        ret
enable  endp

disable proc 
        cli
        ret
disable endp

callfar proc, address: dword
        push bx
        push ds
        push ax
        
        lds bx, address
        lea ax, callfar_ret
        push cs
        push ax
        push ds
        push bx
        
        retf    ; call desired routine
callfar_ret:
        pop ax
        pop ds
        pop bx
        ret
callfar endp

callwithpsp proc, address: dword, psp: dword, ss_new: word, sp_new: word
        push bx
        push ds
        push cx
        push es
        push si
        
        jmp short bypass
ss_sav  dw ?
sp_sav  dw ?        
bypass:
        push ss_sav
        push sp_sav
                
        mov ss_sav, ss
        mov sp_sav, sp
        
        lea ax, callwithpsp_ret
        lds bx, address
        les cx, psp
        
        mov cx, ss_new
        mov si, sp_new
        cli
        mov ss, cx
        mov sp, si
        sti
        
        push cs
        push ax

; MSDOS pushes a 0 on the stack to do a near return, which points
; to the PSP, which has an int 20h in it, which is terminate
        
        mov ax, 0
        push ax
                
        push ds
        push bx

        push es
        pop ds

        mov ax, 0ffffh
        
        retf    ; call desired routine

callwithbypass label near
        pop ax ; skip return address,
        pop ax ;    won't be needed
        pop ax ; get the return code

callwithpsp_ret:

        cli
        mov ss, ss_sav
        mov sp, sp_sav
        sti
        
        pop sp_sav
        pop ss_sav

        pop si        
        pop es
        pop cx
        pop ds
        pop bx
        ret
callwithpsp endp
       
; enable a20 line
; original from Matthew Parker
; mods by Paul Edwards

a20e proc
        mov al, 0d1h
        out 64h, al
a20e_ii:
        in al, 064h
        test al, 2
        jnz a20e_ii
        mov al, 0dfh
        out 060h, al
;        mov ax, 02401h
;        int 015h
        ret
a20e endp

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

boot proc, drive: dword
        push ds
        push bx

; i would rather load dx:ax but i don't know how to do that
        lds bx,drive
        mov dl, bl

; we want to do a jmp 0000h:7c00h, but that seems not to assemble
        mov ax, 0h
        push ax
        mov ax, 7c00h
        push ax
        retf

        pop bx ; shouldn't get here
        pop ds
        ret
boot endp

getabs proc, address3: dword
        push ds
        push bx
        push cx
        lds bx,address3
        mov ax, ds
        mov cl, 12
        shl ax, cl
        mov ds, ax
        mov ah,0
        mov al,[bx]
        pop cx
        pop bx
        pop ds
        ret
getabs endp

putabs proc, address4: dword, character: word
        push ds
        push bx
        push cx
        lds bx,address4
        mov ax, ds
        mov cl, 12
        shl ax, cl
        mov ds, ax        
        mov ax, character
        mov [bx],al
        pop cx
        pop bx
        pop ds
        ret
putabs endp

poweroff proc
        mov ax, 5300h    ; APM Installation Check
        mov bx, 0000h    ; Power Device ID
        int 15h
        jc poweroff_err  ; CF=1: APM not installed
        cmp bx, 504dh    ; magic value not in BX: APM not installed
        jne poweroff_err ; useful in case something else is sitting
                         ; on int 15,AX=5300

        mov ax, 5301h    ; 15,AX=5301: Interface Connect
        mov bx, 0000h    ; BX=0000 (Power Device ID = System BIOS)
        int 15h
        jc poweroff_err

        mov ax, 5307h    ; 15,AX=5307: Set Power State
        mov bx, 0001h
        mov cx, 0003h    ; CX=0003 (Power State = Off)
        int 15h
poweroff_err:
        ret
poweroff endp

end
