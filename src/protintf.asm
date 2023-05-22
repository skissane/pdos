; protected mode interrupt handlers
; written by Paul Edwards
; released to the public domain

.386p
.model flat, c

; symbols defined outside of here that are accessed
        extrn gotint:proc
        extrn saveesp:dword
        extrn call32_esp:ptr

; symbols defined here that are accessed from elsewhere
        public inthdlr
        public inthdlr_0
        public inthdlr_1
        public inthdlr_3
        public inthdlr_8
        public inthdlr_9
        public inthdlr_E
        public inthdlr_10
        public inthdlr_13
        public inthdlr_14
        public inthdlr_15
        public inthdlr_16
        public inthdlr_1A
        public inthdlr_20
        public inthdlr_21
        public inthdlr_25
        public inthdlr_26
        public inthdlr_80
        public inthdlr_A0
        public inthdlr_A3
        public inthdlr_A4
        public inthdlr_A5
        public inthdlr_A6
        public inthdlr_AA
        public inthdlr_B0
        public inthdlr_B1
        public inthdlr_BE
        public int_enable

        .code

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; void inthdlr(void);
;
; handling interrupts is very complex.  here is an example:
;
; command.com, 0x30 does int 21 to exec pgm world
; interrupt saves esp into saveesp
; then a load is done, clobbering saveesp, but not before saving it,
; although since it was already 0x10 it doesn't need to be saved in this case
; then world is executed, which does an int 21 to do a print
; it saves the old saveesp onto the stack, puts the new esp into saveesp,
; then there is a bios call, but it doesn't do anything since ss is already
; 0x10.  then the interrupt ends, restoring saveesp

; inthdlr is the default interrupt handler designed to do nothing.
; It sets the interrupt number to 0xff for recognition by gotint.
inthdlr:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 0ffh
        jmp    inthdlr_q
inthdlr_0:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 0h
        jmp    inthdlr_q
inthdlr_1:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 01h
        jmp    inthdlr_q
inthdlr_3:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 03h
        jmp    inthdlr_q
inthdlr_8:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 08h
        jmp    inthdlr_q
inthdlr_9:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 09h
        jmp    inthdlr_q
inthdlr_E:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 0Eh
        jmp    inthdlr_r
inthdlr_10:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 010h
        jmp    inthdlr_p
inthdlr_13:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 013h
        jmp    inthdlr_p
inthdlr_14:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 014h
        jmp    inthdlr_p
inthdlr_15:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 015h
        jmp    inthdlr_p
inthdlr_16:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 016h
        jmp    inthdlr_p
inthdlr_1A:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 01Ah
        jmp    inthdlr_p
inthdlr_20:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 020h
        jmp    inthdlr_p
inthdlr_21:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 021h
        jmp    inthdlr_p
inthdlr_25:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 025h
        jmp    inthdlr_p
inthdlr_26:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 026h
        jmp    inthdlr_p
inthdlr_80:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 080h
        jmp    inthdlr_p
; Interrupt handlers used to access BIOS
inthdlr_A0:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 0A0h
        jmp    inthdlr_p
inthdlr_A3:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 0A3h
        jmp    inthdlr_p
inthdlr_A4:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 0A4h
        jmp    inthdlr_p
inthdlr_A5:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 0A5h
        jmp    inthdlr_p
inthdlr_A6:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 0A6h
        jmp    inthdlr_p
inthdlr_AA:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 0AAh
        jmp    inthdlr_p
; IRQ handlers.
inthdlr_B0:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 0B0h
        jmp    inthdlr_q
inthdlr_B1:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 0B1h
        jmp    inthdlr_q
inthdlr_BE:
        push   eax
        mov    ax, ds
        push   eax
        mov    eax, 010h
        mov    ds, ax
        push   intnum
        mov    intnum, 0BEh
        jmp    inthdlr_q

; by the time we get here, the following things are on the stack:
; original eax, original ds (stored as doubleword), original intnum

inthdlr_p:
        push   saveess
        push   saveesp
        push   ebx
        push   saveeax
        push   saveebx
        mov    eax, 0
        mov    ax, ss
        mov    saveess, eax
        mov    eax, esp
        mov    saveesp, eax
        push   ebp
        mov    ebp, esp
; Restore original eax (at time of interrupt) which is now located
; at offset 32 thanks to the above pushes
        mov    eax, 32[ebp]
        mov    saveeax, eax
        pop    ebp
        cmp    dword ptr saveess, 010h
        je     level10
        mov    eax, 010h
        mov    ss, ax
        mov    es, ax
        mov    fs, ax
        mov    gs, ax
        mov    eax, call32_esp
        mov    esp, eax
level10:
        mov    eax, saveeax
; saveess and saveesp must be saved on the stack
; because task switch can occur
; and the next interrupt might not restore them
; before switch back happens
        push   saveess
        push   saveesp
; some interrupts need bp, so we now make that accessible
        push   ebp
        push   edx
; above is actually room for flags
        push   edx
; above is actually room for cflag
        push   edi
        push   esi
        push   edx
        push   ecx
        push   ebx
        push   eax
        mov    esi, eax
        mov    edi, ebx
        mov    eax, esp
; above is pointer to saved registers
        push   eax
        mov    edx, intnum
        push   edx
; above interrupt number
        call   gotint
        pop    edx
        pop    eax
; pops saved registers
        pop    eax
        pop    ebx
        pop    ecx
        pop    edx
        pop    esi
        pop    edi
        mov    saveeax, eax
        pop    eax
; above is actually cflag
        mov    saveebx, ebx
        pop    ebx
; above is actually flags
; now we have ebp
        pop    ebp
        pop    saveesp
        pop    saveess
; above are saved saveesp and saveess to handle task switches
        cmp    dword ptr saveess, 010h
        je     level10b
        mov    eax, saveesp
        mov    esp, eax
level10b:
        mov    eax, saveess
        mov    ss, ax
        push   ebp
        mov    ebp, esp
; update the bottom 8 bits plus bit 11 (OF) of the flags
        and    dword ptr 44[ebp], 0fffff700h
        and    ebx, 08ffh
        or     44[ebp], ebx
        mov    ebx, saveebx
        mov    12[ebp], ebx
        push   eax
        mov    eax, saveeax
        mov    32[ebp], eax
        pop    eax
        pop    ebp
        mov    es, ax
        mov    fs, ax
        mov    gs, ax
        pop    saveebx
        pop    saveeax
        pop    ebx
        pop    saveesp
        pop    saveess
        pop    intnum
        pop    eax
        mov    ds, ax
        pop    eax
        push   ebp
        mov    ebp, esp
        pop    ebp
        iretd

; This is for interrupts that should not alter
; the flags, like the timer interrupt
; Also for things that specifically manipulate the flags,
; such as when tracing.

; by the time we get here, the following things are on the stack:
; original eax, original ds (stored as doubleword), original intnum

; And because this is an interrupt that does not push an error
; code, above those 3 dwords are the EIP, cs (stored as a
; dword), and the flags (also stored as a dword). All three of
; those things will be popped when we do an iretd.

; Above that is completely unpredictable, as it is just whatever
; the application had pushed onto the stack before the
; interrupt occurred.

; gotint() will receive the interrupt plus an array of registers.
; It will then pass it on to the specific interrupt handler, just
; passing the register array. This pointer is all that the
; interrupt will have to work with, so we need to calculate
; everything from that spot.

; In addition, the stack is actually switched (to the caller's
; stack, but with the OS (0x10) ss) prior to the register array
; being constructed. I think
; this switch is done to allow the interrupt to terminate the
; called program if it wishes to do so. The stack pointer before
; the switch is available on the new stack, after the registers
; (EAX, EBX, ECX, EDX, ESI, EDI) then cflag, then flags. Although
; at time of writing, the flags are not being correctly passed
; up to gotint().

; We don't switch stack if this is the highest level, ie ss of 0x10
; Note that when an application is running it will have an ss of
; 0x30 which is "spawn_data"

; old stack looks like this at the time of stack switch:
; unpredictable stack data pushed by application during execution,
; before interruption
; flags
; cs
; eip
; eax
; ds
; old intnum
; previous interrupt's ss (saveess)
; previous interrupt's esp (saveesp)
; ebx (not sure why it is needed, seems to do with flags)
; previous interrupt's eax (saveeax)
; previous interrupt's ebx (saveebx)
; The above is what saveesp will be pointing to
; ebp is temporarily stored here, and will remain if there is a
;     stack switch done

inthdlr_q:
        push   saveess
        push   saveesp
        push   ebx
        push   saveeax
        push   saveebx
        mov    eax, 0h
        mov    ax, ss
        mov    saveess, eax
        mov    eax, esp
        mov    saveesp, eax
        push   ebp
        mov    ebp, esp
; Restore original eax (at time of interrupt) which is now located
; at offset 32 thanks to the above pushes
        mov    eax, 32[ebp]
        mov    saveeax, eax
        pop    ebp
        cmp    dword ptr saveess, 010h
        je     level10c
        mov    eax, 010h
        mov    ss, ax
        mov    es, ax
        mov    fs, ax
        mov    gs, ax
        mov    eax, call32_esp
        mov    esp, eax
; Now we are on the new stack.
level10c:
        mov    eax, saveeax
; saveess and saveesp must be saved on stack
; because task switch can occur
; and the next interrupt might not restore them
; before switch back happens
        push   saveess
        push   saveesp
; some interrupts need bp, so we now make that accessible
        push   ebp
        push   edx
; above is actually room for flags
        push   edx
; above is actually room for cflag
        push   edi
        push   esi
        push   edx
        push   ecx
        push   ebx
        push   eax
        mov    esi, eax
        mov    edi, ebx
        mov    eax, esp
; above is pointer to saved registers
        push   eax
        mov    edx, intnum
        push   edx
; above interrupt number
        call   gotint
        pop    edx
        pop    eax
; pops saved registers
        pop    eax
        pop    ebx
        pop    ecx
        pop    edx
        pop    esi
        pop    edi
        mov    saveeax, eax
        pop    eax
; above is actually cflag
        mov    saveebx, ebx
        pop    ebx
; above is actually flags
; now we have ebp
        pop    ebp
        pop    saveesp
        pop    saveess
; above are saved saveesp and saveess to handle task switches
        cmp    dword ptr saveess, 010h
        je     level10d
        mov    eax, saveesp
        mov    esp, eax
level10d:
        mov    eax, saveess
        mov    ss, ax
        push   ebp
        mov    ebp, esp
; Don't set the flags
;        mov    %bl, 44[ebp]
        mov    ebx, saveebx
        mov    12[ebp], ebx
        push   eax
        mov    eax, saveeax
        mov    32[ebp], eax
        pop    eax
        pop    ebp
        mov    es, ax
        mov    fs, ax
        mov    gs, ax
        pop    saveebx
        pop    saveeax
        pop    ebx
        pop    saveesp
        pop    saveess
        pop    intnum
        pop    eax
        mov    ds, ax
        pop    eax
; These 3 instructions look pointless to me
        push   ebp
        mov    ebp, esp
        pop    ebp
        iretd

; This is for exceptions that have an error code pushed when they occur.
inthdlr_r:
        push   saveess
        push   saveesp
        push   ebx
        push   saveeax
        push   saveebx
        mov    eax, 0h
        mov    ax, ss
        mov    saveess, eax
        mov    eax, esp
        mov    saveesp, eax
        push   ebp
        mov    ebp, esp
; Restore original eax (at time of interrupt) which is now located
; at offset 32 thanks to the above pushes
        mov    eax, 32[ebp]
        mov    saveeax, eax
; Saves the error code pushed after the data for iretd.
        mov    eax, 36[ebp]
        mov    saveerrorcode, eax
        pop    ebp
        cmp    dword ptr saveess, 010h
        je     level10e
        mov    eax, 010h
        mov    ss, ax
        mov    es, ax
        mov    fs, ax
        mov    gs, ax
        mov    eax, call32_esp
        mov    esp, eax
level10e:
        mov    eax, saveeax
; saveess and saveesp must be saved on stack
; because task switch can occur
; and the next interrupt might not restore them
; before switch back happens
        push   saveess
        push   saveesp
        push   saveerrorcode
; above is duplicated error code
; some interrupts need bp, so we now make that accessible
        push   ebp
        push   edx
; above is actually room for flags
        push   edx
; above is actually room for cflag
        push   edi
        push   esi
        push   edx
        push   ecx
        push   ebx
        push   eax
        mov    esi, eax
        mov    edi, ebx
        mov    eax, esp
; above is pointer to saved registers
        push   eax
        mov    edx, intnum
        push   edx
; above interrupt number
        call   gotint
        pop    edx
        pop    eax
; pops saved registers
        pop    eax
        pop    ebx
        pop    ecx
        pop    edx
        pop    esi
        pop    edi
        mov    saveeax, eax
        pop    eax
; above is actually cflag
        mov    saveebx, ebx
        pop    ebx
; above is actually flags
; now we have ebp
        pop    ebp
        add    esp, 04h
; above is the duplicated error code
        pop    saveesp
        pop    saveess
; above are saved saveesp and saveess to handle task switches
        cmp    dword ptr saveess, 010h
        je     level10f
        mov    eax, saveesp
        mov    esp, eax
level10f:
        mov    eax, saveess
        mov    ss, ax
        push   ebp
        mov    ebp, esp
; Don't set the flags
;        mov    %bl, 44[ebp]
        mov    ebx, saveebx
        mov    12[ebp], ebx
        push   eax
        mov    eax, saveeax
        mov    32[ebp], eax
        pop    eax
        pop    ebp
        mov    es, ax
        mov    fs, ax
        mov    gs, ax
        pop    saveebx
        pop    saveeax
        pop    ebx
        pop    saveesp
        pop    saveess
        pop    intnum
        pop    eax
        mov    ds, ax
        pop    eax
        push   ebp
        mov    ebp, esp
        pop    ebp
; Removes the error code pushed when the exception occured.
        add    esp, 04h
        iretd

;////////////////////////////////////////////////////////////
; void int_enable(void);
;
int_enable:
        sti
        ret

.data
saveeax dd ?
saveebx dd ?
saveess dd ?
saveerrorcode dd ?
intnum dd ?

        end
