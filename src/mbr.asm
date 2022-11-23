; Public domain MBR by Mark Raymond
; Modified by Paul Edwards to be masm format
; and only use 8086 instructions

; This is a standard MBR, which loads the VBR of the
; active partition to 0x7c00 and jumps to it.

; ## Preconditions

; * MBR is loaded to the physical address 0x7c00
; * dl contains the disk ID the system was booted from

; ## Postconditions

; * If loading was successful
;   - VBR is loaded to the physical addres 0x7c00
;   - dl contains the disk ID the system was booted from
;   - ds:si and ds:bp point to the partition entry the VBR was loaded from

; * If loading was unsuccessful
;   - Error message is displayed on screen
;   - System hangs

; ## Errors

; This MBR will output an error message and hang if:

; * There are no active partitions
; * The active partition has a partition type of 0
; * The BIOS does not support LBA extensions
; * The disk cannot be read
; * The VBR does not end with the boot signature (0xaa55)

% .model memodel

.code

org 0600h

top:

; Clear interrupts during initialization
cli

; Initialize segment registers and stack
xor ax,ax
mov ds,ax
mov es,ax
mov ss,ax
mov sp,07c00h

; Allow interrupts
; See http://forum.osdev.org/viewtopic.php?p=236455#p236455 for more information
sti

; Relocate the MBR
mov si,07c00h       ; Set source and destination
mov di,0600h
mov cx,0100h       ; 0x100 words = 512 bytes
rep movsw           ; Copy mbr to 0x0600
;jmp 0:relocated     ; Far jump to copied MBR
mov ax, 0
push ax
mov ax, offset relocated
push ax
retf


relocated:

; Search partitions for one with active bit set
mov si,07beh   ; partition_table
mov cx,4
test_active:
test byte ptr [si],080h
jnz found_active
add si,16 ; entry_length
loop test_active
; If we get here, no active partition was found,
; so output and error message and hang
mov bp,offset no_active_partitions
jmp fatal_error

; Found a partition with active bit set
found_active:
cmp byte ptr [si+4],0; check partition type, should be non-zero
mov bp,offset active_partition_invalid
jz fatal_error

; Check BIOS LBA extensions exist
mov ah,041h
mov bx,055aah
int 013h
mov bp,offset no_lba_extensions
jc fatal_error
cmp bx,0aa55h
jnz fatal_error

; Load volume boot record
mov ax,[si+8]  ; put sector number into LBA packet
mov [lbalow],ax
mov ax,[si+10]
mov [lbahigh],ax
mov cx,10        ; ten tries
push si         ; save pointer to partition info
try_read:
mov ah,042h
mov si,offset lba_packet
int 013h        ; BIOS LBA read (dl already set to disk number)
jnc read_done
mov ah,0
int 013h        ; reset disk system
loop try_read
mov bp,offset read_failure
jmp fatal_error
read_done:
pop si          ; restore pointer to partition info

; Check the volume boot record is bootable
cmp word ptr es:[07dfeh],0aa55h
mov bp,offset invalid_vbr
jnz fatal_error

; Jump to the volume boot record
mov bp,si           ; ds:bp is sometimes used by Windows instead of ds:si
;jmp 0000h:07c00h   ; if boot signature passes, we can jump,
                    ; as ds:si and dl are already set
mov ax, 0
push ax
mov ax, 07c00h
push ax
retf

output_loop:
int 010h        ; output
inc bp
fatal_error:
mov ah,0eh     ; BIOS teletype
mov al,[bp]     ; get next char
cmp al,0        ; check for end of string
jnz output_loop
hang:
; Bochs magic breakpoint, for unit testing purposes.
; It can safely be left in release, as it is a no-op.
xchg bx,bx
sti
hlt
jmp hang

; Error messages
no_active_partitions:
xx1     db "No active partition found!",0
active_partition_invalid:
xx2 db "Active partition has invalid partition type!",0
no_lba_extensions:
xx3        db "BIOS does not support LBA extensions!",0
read_failure:
xx4             db "Failed to read volume boot record!",0
invalid_vbr:
xx5 db "Volume boot record is not bootable (missing 0xaa55 boot signature)!",0

; LBA packet for BIOS disk read
; It was previously 8-byte aligned (with "align 8"), but that
; gives a warning
; from wasm (which gets treated as an error), and it seems
; that alignment is not actually required
lba_packet:
sz         db 010h
reserved   db 0
sectors    dw 1
offst      dw 07c00h
segmnt     dw 0
lbalow     dw 0
lbahigh    dw 0
lbapadding dd 0

; force padding to 440 bytes of code
org 07b8h

;org 07beh
;partition_table:

;struc partition
;.start:
;.status:     resb 1
;.start_chs:  resb 3
;.type:       resb 1
;.end_chs:    resb 3
;.start_lba:  resd 1
;.length_lba: resd 1
;.end:
;endstruc

;org 07ffh
;filler db 0


end top

