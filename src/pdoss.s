/ pdos support routines in assembler
/ written by Paul Edwards
/ released to the public domain

/ symbols defined here that are accessed from elsewhere
        .globl _call32
        .globl _callwithbypass
        .globl _loadPageDirectory
        .globl _saveCR3
        .globl _enablePaging
        .globl _disablePaging
        .globl _readCR2
        .globl _switchFromToThread
        .globl _getEFLAGSAndDisable
        .globl _setEFLAGS
        .globl _callDllEntry

        .text

/////////////////////////////////////////////////////////////
/ int _call32(int entry, int sp, TCB *curTCB);
_call32:
        push    %ebp
        mov     %esp, %ebp
        pushf
/ disable interrupts so that we can play with stack        
/ Note - I think this can actually be moved down to where the
/ actual switch takes place
        cli
        push    %ebx
        push    %ecx
        push    %edx
        push    %esi
        push    %edi
        push    _call32_esp
        push    _saveesp2
        mov     %esp, _call32_esp
/ _call32_esp has to be saved in TCB
        mov     16(%ebp), %edi
        mov     %esp, 4(%edi)
/ save stack of caller
        mov     _saveesp, %eax
        mov     %eax, _saveesp2
/ load return address into ecx
        lea    _call32_ret, %ecx
/ get subroutine's address into ebx
        mov    8(%ebp), %ebx
/ load address of single lret into edi
        lea    _call32_singlelret, %edi
/ load address of single ret into esi
        lea    _call32_singleret, %esi
/ switch stack etc to new one
        mov    12(%ebp), %eax
/ I think this is where interrupts should really be disabled
        mov    %eax, %esp
        mov    $0x30, %ax
        mov    %ax, %ss
        mov    %ax, %gs
        mov    %ax, %fs
        mov    %ax, %es
        mov    %ax, %ds
        
/ push return address
/ +++ should next 2 %eax be %ax?
        mov    $0x8, %eax
        push   %eax
        push   %ecx
/ push parameters for subroutine from right to left
        push   %edx
        mov    $0, %eax
        push   %eax
        push   %eax
        push   %eax        
/ push address of a "lret" statement so that they can come back
        push   %edi
/ push subroutine's address
        push    %ebx
/ push address of a "ret" statement so that we don't directly call
/ their code, but they can return from their executable with a
/ normal ret
/ +++ should next 2 references be %ax instead of %eax?
        mov     $0x28, %eax
        push    %eax
        push    %esi
/ reenable interrupts for called program        
        sti
/ call it via far return        
        lret
_call32_singleret:
        ret
_call32_singlelret:
/ and skip the parameters too
        add     $16, %esp
        lret
_call32_ret:
/ disable interrupts so that we can play with stack
        cli
/ restore our old stack etc
        mov    $0x10, %bx
        mov    %bx, %ds
        mov    _call32_esp, %esp
        mov    %bx, %ss
        mov    %bx, %gs
        mov    %bx, %fs
        mov    %bx, %es
/ reenable interrupts
        sti

_call32_pops:
        pop    _saveesp2
        pop    _call32_esp
        pop    %edi
        pop    %esi        
        pop    %edx
        pop    %ecx
        pop    %ebx
        popf
        pop    %ebp
        ret

                
/////////////////////////////////////////////////////////////
/ void _callwithbypass(int retcode);
/ for use by programs which terminate via int 21h.
/ This function works to get PDOS back to the state it was
/ when it called the user program.

_callwithbypass:
/ skip return address, not required
        pop     %eax
/ restore old esp
        mov     _saveesp2, %eax
        mov     %eax, _saveesp
/ get return code
        pop     %eax
        jmp     _call32_ret

/////////////////////////////////////////////////////////////
/ void loadPageDirectory(unsigned long page_directory_address);
/ Loads Page Directory address into CR3.
_loadPageDirectory:
        push    %ebp
        mov     %esp, %ebp
        mov     8(%ebp), %eax
        mov     %eax, %cr3
        pop     %ebp
        ret

/////////////////////////////////////////////////////////////
/ unsigned long saveCR3(void);
/ Returns the content of CR3.
_saveCR3:
        push    %ebp
        mov     %esp, %ebp
        mov     %cr3, %eax
        pop     %ebp
        ret

/////////////////////////////////////////////////////////////
/ void enablePaging(void);
/ Sets the Paging bit of CR0.
_enablePaging:
        push    %ebp
        mov     %esp, %ebp
        mov     %cr0, %eax
        or      $0x80000000, %eax
        mov     %eax, %cr0
        pop     %ebp
        ret

/////////////////////////////////////////////////////////////
/ void disablePaging(void);
/ Clears the Paging bit of CR0.
_disablePaging:
        push    %ebp
        mov     %esp, %ebp
        mov     %cr0, %eax
        and     $0x7fffffff, %eax
        mov     %eax, %cr0
        pop     %ebp
        ret

/////////////////////////////////////////////////////////////
/ unsigned long readCR2(void);
/ Returns the content of CR2.
_readCR2:
        push    %ebp
        mov     %esp, %ebp
        mov     %cr2, %eax
        pop     %ebp
        ret

/////////////////////////////////////////////////////////////
/ void switchFromToThread(TCB *oldTCB, TCB *newTCB);
/ Switches from oldTCB thread to newTCB thread.
/ Interrupts should be disabled before calling.
_switchFromToThread:
/ Saves registers.
    push %eax
    push %ebx
    push %ecx
    push %edx
    push %esi
    push %edi
    push %ebp
    pushf

/ Loads oldTCB into EDI and newTCB into ESI.
    push %ebp
    mov %esp, %ebp
    mov 40(%ebp), %edi
    mov 44(%ebp), %esi
    pop %ebp

/ Saves ESP of the current thread into oldTCB.
    mov %esp, 0(%edi)

/ Loads state from newTCB.
    mov 0(%esi), %esp
    mov 4(%esi), %eax
    mov %eax, _call32_esp

/ Code running after the switch.
/ Pops registers.
    popf
    pop %ebp
    pop %edi
    pop %esi
    pop %edx
    pop %ecx
    pop %ebx
    pop %eax
    ret

/////////////////////////////////////////////////////////////
/ unsigned int getEFLAGSAndDisable(void);
/ Returns current EFLAGS and disables interrupts.
_getEFLAGSAndDisable:
        push    %ebp
        mov     %esp, %ebp
        pushf
        pop     %eax
        cli
        pop     %ebp
        ret

/////////////////////////////////////////////////////////////
/ void setEFLAGS(unsigned int flags);
/ Sets EFLAGS.
_setEFLAGS:
        push    %ebp
        mov     %esp, %ebp
        mov     8(%ebp), %eax
        push    %eax
        popf
        pop     %ebp
        ret

/////////////////////////////////////////////////////////////
/ BOOL callDllEntry(void *entry_point, HINSTANCE hinstDll,
/                   DWORD fdwReason, LPVOID lpvReserved);
/ Calls the provided DLL entry point using __stdcall convention.
_callDllEntry:
        push    %ebp
        mov     %esp, %ebp
/ Copies arguments for the DLL entry function (right to left).
        mov     20(%ebp), %eax
        push    %eax
        mov     16(%ebp), %eax
        push    %eax
        mov     12(%ebp), %eax
        push    %eax
/ Calls the entry point.
        mov     8(%ebp), %eax
        call    *%eax
        pop     %ebp
        ret

.bss
        .p2align 2
saveess:
        .space 4
        .p2align 2
        .globl _saveesp
_saveesp:
        .space 4
        .p2align 2
_saveesp2:
        .space 4
        .p2align 2
        .globl _call32_esp
_call32_esp:
        .space 4
