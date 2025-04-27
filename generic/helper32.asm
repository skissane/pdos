; helper32.asm - helper code for loading 32-bit PEE executables
; on an x64 system
;
; This program written by Paul Edwards
; Released to the public domain

.386p
.model flat, c

;extrn hlp32st:proc


.data
banner  db  "HELPER32"

.code

top:

public __intstart
__intstart proc var1: dword
;
        mov eax, 13
        ret
__intstart endp


end top
