; pdos support routines in assembler
; written by Paul Edwards
; released to the public domain

.386p
.model flat, c

; symbols defined here that are accessed from elsewhere
        public call32
        public callwithbypass
        public loadPageDirectory
        public saveCR3
        public enablePaging
        public disablePaging
        public readCR2
        public switchFromToThread
        public getEFLAGSAndDisable
        public setEFLAGS
        public callDllEntry
        public stdcallWithArgCopy

        .code

;////////////////////////////////////////////////////////////
; int call32(int entry, int sp, TCB *curTCB);
call32:
        push    ebp
        mov     ebp, esp
        pushfd
; disable interrupts so that we can play with stack        
; Note - I think this can actually be moved down to where the
; actual switch takes place
        cli
        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi
        push    call32_esp
        push    saveesp2
        mov     call32_esp, esp
; call32_esp has to be saved in TCB
        mov     edi, 16[ebp]
        mov     4[edi], esp
; save stack of caller
        mov     eax, saveesp
        mov     saveesp2, eax
; load return address into ecx
        lea    ecx, call32_ret
; get subroutine's address into ebx
        mov    ebx, 8[ebp]
; load address of single retf into edi
        lea    edi, call32_singleretf
; load address of single ret into esi
        lea    esi, call32_singleret
; switch stack etc to new one
        mov    eax, 12[ebp]
; I think this is where interrupts should really be disabled
        mov    esp, eax
        mov    ax, 030h
        mov    ss, ax
        mov    gs, ax
        mov    fs, ax
        mov    es, ax
        mov    ds, ax
        
; push return address
; +++ should next 2 %eax be %ax?
        mov    eax, 08h
        push   eax
        push   ecx
; push parameters for subroutine from right to left
        push   edx
        mov    eax, 0h
        push   eax
        push   eax
        push   eax
; push address of a "retf" statement so that they can come back
        push   edi
; push subroutine's address
        push    ebx
; push address of a "ret" statement so that we don't directly call
; their code, but they can return from their executable with a
; normal ret
; +++ should next 2 references be %ax instead of %eax?
        mov     eax, 028h
        push    eax
        push    esi
; reenable interrupts for called program        
        sti
; call it via far return        
        retf
call32_singleret:
        ret
call32_singleretf:
        pop     ebx
; and skip the parameters too
        add     esp, 16
        retf
call32_ret:
; disable interrupts so that we can play with stack
        cli
; restore our old stack etc
        mov    bx, 010h
        mov    ds, bx
        mov    esp, call32_esp
        mov    ss, bx
        mov    gs, bx
        mov    fs, bx
        mov    es, bx
; reenable interrupts
        sti

call32_pops:
        pop    saveesp2
        pop    call32_esp
        pop    edi
        pop    esi
        pop    edx
        pop    ecx
        pop    ebx
        popfd
        pop    ebp
        ret

                
;////////////////////////////////////////////////////////////
; void _callwithbypass(int retcode);
; for use by programs which terminate via int 21h.
; This function works to get PDOS back to the state it was
; when it called the user program.

callwithbypass:
; skip return address, not required
        pop     eax
; restore old esp
        mov     eax, saveesp2
        mov     saveesp, eax
; get return code
        pop     eax
        jmp     call32_ret

;////////////////////////////////////////////////////////////
; void loadPageDirectory(unsigned long page_directory_address);
; Loads Page Directory address into CR3.
loadPageDirectory:
        push    ebp
        mov     ebp, esp
        mov     eax, 8[ebp]
        mov     cr3, eax
        pop     ebp
        ret

;////////////////////////////////////////////////////////////
; unsigned long saveCR3(void);
; Returns the content of CR3.
saveCR3:
        push    ebp
        mov     ebp, esp
        mov     eax, cr3
        pop     ebp
        ret

;////////////////////////////////////////////////////////////
; void enablePaging(void);
; Sets the Paging bit of CR0.
enablePaging:
        push    ebp
        mov     ebp, esp
        mov     eax, cr0
        or      eax, 080000000h
        mov     cr0, eax
        pop     ebp
        ret

;////////////////////////////////////////////////////////////
; void disablePaging(void);
; Clears the Paging bit of CR0.
disablePaging:
        push    ebp
        mov     ebp, esp
        mov     eax, cr0
        and     eax, 07fffffffh
        mov     cr0, eax
        pop     ebp
        ret

;////////////////////////////////////////////////////////////
; unsigned long readCR2(void);
; Returns the content of CR2.
readCR2:
        push    ebp
        mov     ebp, esp
        mov     eax, cr2
        pop     ebp
        ret

;////////////////////////////////////////////////////////////
; void switchFromToThread(TCB *oldTCB, TCB *newTCB);
; Switches from oldTCB thread to newTCB thread.
; Interrupts should be disabled before calling.
switchFromToThread:
; Saves registers.
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    pushfd

; Loads oldTCB into EDI and newTCB into ESI.
    push ebp
    mov ebp, esp
    mov edi, 40[ebp]
    mov esi, 44[ebp]
    pop ebp

; Saves ESP of the current thread into oldTCB.
    mov 0[edi], esp

; Loads state from newTCB.
    mov esp, 0[esi]
    mov eax, 4[esi]
    mov call32_esp, eax

; Code running after the switch.
; Pops registers.
    popfd
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

;////////////////////////////////////////////////////////////
; unsigned int getEFLAGSAndDisable(void);
; Returns current EFLAGS and disables interrupts.
getEFLAGSAndDisable:
        push    ebp
        mov     ebp, esp
        pushfd
        pop     eax
        cli
        pop     ebp
        ret

;////////////////////////////////////////////////////////////
; void setEFLAGS(unsigned int flags);
; Sets EFLAGS.
setEFLAGS:
        push    ebp
        mov     ebp, esp
        mov     eax, 8[ebp]
        push    eax
        popfd
        pop     ebp
        ret

;////////////////////////////////////////////////////////////
; BOOL callDllEntry(void *entry_point, HINSTANCE hinstDll,
;                   DWORD fdwReason, LPVOID lpvReserved);
; Calls the provided DLL entry point using __stdcall convention.
callDllEntry:
        push    ebp
        mov     ebp, esp
; Copies arguments for the DLL entry function (right to left).
        mov     eax, 20[ebp]
        push    eax
        mov     eax, 16[ebp]
        push    eax
        mov     eax, 12[ebp]
        push    eax
; Calls the entry point.
        mov     eax, 8[ebp]
        call    eax
        pop     ebp
        ret

;////////////////////////////////////////////////////////////
; unsigned int stdcallWithArgCopy(void *function, void *arguments,
;                                 unsigned int num_args);
; Calls the provided function using __stdcall convention
; with arguments copied from memory.
stdcallWithArgCopy:
        push    ebp
        mov     ebp, esp   
; Copies arguments from memory.
        mov     ecx, 16[ebp]
        test    ecx, ecx
        jz      end_loop
        mov     eax, 12[ebp]
        sub     eax, 4
copy_loop:
        push    [eax + ecx*4]
        sub     ecx, 1
        jnz     copy_loop
end_loop:
; Calls the function.
        mov     eax, 8[ebp]
        call    eax
        pop     ebp
        ret

.data?
saveess dd ?
        public saveesp
saveesp dd ?
saveesp2 dd ?
        public call32_esp
call32_esp dd ?

        end
