; startup code for 32 bit version of pdos
; written by Paul Edwards
; released to the public domain

.386p
.model flat, c

; symbols defined outside of here that are accessed
        extrn runaout_p:proc
        extrn pdosstrt:proc
        extrn __userparm:dword

; symbols defined here that are accessed from elsewhere
        public start

        .code

;
; void start(rawprot_parms *parmlist);

; This is the entry point for the whole 32 bit PDOS executable.
; It only takes one parameter, a pointer to some clumped parms.

start:
        push    ebp
        mov     ebp, esp

; Call runaout_p(parmlist) to set up anything it needs
        push    8[ebp]
        call    runaout_p
        add     esp, 4
        mov     __userparm, eax

; Call main C entry point for PDOS.
        call    pdosstrt

; Return to PDOS loader
        pop     ebp
        ret

        end
