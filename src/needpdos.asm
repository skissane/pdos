; needpdos.asm - stub for Win32 executables
;
; This program written by Paul Edwards
; Released to the public domain

.model tiny

.stack 400h

; Note that the string as it currently stands is exactly the
; right size to fit into "ldwin" peigen.c. And note that
; "od -t x4" on needpdos.exe will generate hex values suitable
; for updating peigen.c. So you can change the string but not
; make it bigger. Unless you change other stuff as well.

.data
ifdef OS220
msg  db  "This program needs OS/2 2.0 or equivalent"
else
ifdef PDOS86
msg  db  "This program needs PDOS/86 or equivalent"
else
msg  db  "Install HX or upgrade to PDOS/386 or Wine etc"
endif
endif
msg2 db  0DH
msg3 db  0AH
msg4 db "$"

.code

; Note that although in the tiny memory model cs and ds will
; be pointing to the PSP and the IP will be 100H on entry,
; that is something for the linker to sort out, we don't do
; our own "org 100h" as we would if the intended destination
; was a .com file. I could be wrong though ...

public top
top:

push cs
pop ds

mov ah,09h
mov dx,offset msg
int 21h

mov al,1 ; set an error return code
mov ah,4ch
int 21h ; terminate

; No need for a return. We're screwed anyway if the above
; doesn't work. There's nothing on the stack to return to.
;ret

end top
