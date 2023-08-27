; Public domain MBR by Mark Raymond
; Modified by Paul Edwards to be masm format
; and only use 8086 instructions
; Subsequently modified to use CHS instead of LBA for
; devices that don't support LBA

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

org 0100h

top:

; Clear interrupts during initialization
cli

; Initialize segment registers and stack
xor ax,ax
mov ds,ax
mov es,ax
mov ax, 050h
; we need ss and cs/ds to match later because of the use of bp
; we could use a different register, but a mismatch of ss and
; cs/ds may bite someone at a later date
mov ss,ax
; this is absolute 07c00h
mov sp,07700h

; Allow interrupts
; See http://forum.osdev.org/viewtopic.php?p=236455#p236455 for more information
sti

; Relocate the MBR
mov si,07c00h       ; Set source and destination
mov di,0600h
mov cx,0100h       ; 0x100 words = 512 bytes
rep movsw           ; Copy mbr to 0x0600
;jmp 0:relocated     ; Far jump to copied MBR
mov ax, 050h ; fake PSP to support COM file offsets
push ax
mov ax, offset relocated
push ax
retf


relocated:

push cs
pop ds

mov  [BootDisk], dl   ;Store our boot disk

; Search partitions for one with active bit set
mov si, 02beh   ; partition_table
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


; mark VBR invalid
mov word ptr es:[07dfeh],0h

mov ax, 0
mov es, ax

; get disk geometry from BIOS instead of relying on values
; stored at format time, because the disk may have been moved
; to a different machine with a different faking mechanism
 mov ah, 08h
 mov di, 0
; es should already be 0
; dl is already set
 int 13h
 jc ignorec   ; if it fails, just use the CHS values in the partition entry

 and cl,3fh
 mov ch, 0
 mov [SectorsPerTrack], cx

; dh currently has maximum head number, which are counted
; from 0, so the number of heads is 1 more than that, and
; may overflow a single byte
 mov dl,dh
 mov dh,0
 add dx,1
 mov [Heads], dx

mov ax,[si+8]  ; low 16 bits of sector number
mov dx,[si+10] ; high 16 bits of sector number

call Lba2Chs

jmp donemanual

ignorec:

; we use the CHS from the partition entry
mov dh, [si+1]
mov cl, [si+2]
mov ch, [si+3]


donemanual:


; es should still be 0
mov bx, 07c00h

; ch, cl, dh must be set already
; es:bx - dest
call ReadSingleSector






; Check the volume boot record is bootable
cmp word ptr es:[07dfeh],0aa55h
mov bp,offset invalid_vbr
jnz fatal_error

; Jump to the volume boot record
mov  dl, [BootDisk]  ;Restore boot disk
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
read_failure:
xx4             db "Failed to read VBR!",0
invalid_vbr:
xx5 db "VBR missing aa55 signature!",0





;Convert LBA -> CHS
;Inputs:
; dx:ax - sector (32-bit value)
;Outputs:
; Standard CHS format for use by int 13h
; INT 13h allows 256 heads, 1024 cylinders, and 63 sectors max
; Cylinder = LBA / (Heads_Per_Cylinder * Sectors_Per_Track)
; Temp = LBA % (Heads_Per_Cylinder * Sectors_Per_Track)
; Heads = Temp / Sectors_Per_Track
; Sector = Temp % Sectors_Per_Track + 1
; CH = Cylinder (Lowest 8-bits)
; CL = Lowest 6 bits contain Sector. Highest 2 bits are highest
;      2 bits of cylinder number
; DH = Head
; Div = Dx:AX / value
;  AX = Quotient (Result)
;  DX = Remainder (Leftover, Modulus)
Lba2Chs proc
 div  word ptr [SectorsPerTrack]
; AX = DX:AX / SectorsPerTrack (Temp)
; DX = DX:AX % SectorsPerTrack (Sector)
 mov  ch, 0
 mov  cl,  dl     ;Sector #
 inc  cl ;Add one since sector starts at 1, not zero
 xor  dx, dx       ;Zero out dx, so now we are just working on AX
 div  word ptr [Heads]
; AX = AX / Heads ( = Cylinder)
; DX = AX % Heads ( = Head)
 mov  dh,  dl     ;Mov dl into dh (dh=head)
 push dx
 mov  dh, al ; save lower 8 bits of cylinder number
 mov  al, 0  ; clear lower 8 bits of cylinder number
;Have to save cx because 8086 needs it to be able to shr!
 push cx
 mov  cl, 2
 shr  ax, cl
 pop  cx
 mov  ah, dh
 pop  dx
 or   cx, ax
 ret
Lba2Chs endp

;Used to reset our drive before we use it
;Inputs:
; (None)
;Outputs:
; (None)
ResetDrive proc
 push ax
 push dx
 RetryReset:
  mov  dl, [BootDisk]
  mov  ax,  0
  int  013h
  jc  RetryReset    ;Didn't reset, lets try again
 pop  dx
 pop  ax
 ret
ResetDrive endp

;Read a single sector from a floppy disk
;Inputs:
; ch, cl, dh must already be set
; es:bx - dest
;Outputs:
; (None)
ReadSingleSector proc
 push ax
 push bx
 push cx
 push dx
 push es
 RetryRead:
  mov  dl, [BootDisk]  ;Grab our boot disk
  mov  ax, 0201h   ;Read function, one sector
  int  13h
  jnc  fin
 call ResetDrive   ;Get drive ready..
 jmp   RetryRead
fin:
 pop es
 pop dx
 pop cx
 pop bx
 pop ax
 ret
ReadSingleSector endp



; routine copied from public domain mon86 and modified
dumpcx proc
;Print out 16-bit value in CX in hex

OUT16:
push ax
push bx
	MOV	AL,CH		;High-order byte first
	CALL	HEX
	MOV	AL,CL		;Then low-order byte
        CALL    HEX
	MOV	AL," "
	CALL	OUT2
pop bx
pop ax
        RET

;Output byte in AL as two hex digits

HEX:
	MOV	BL,AL		;Save for second digit
;Shift high digit into low 4 bits
	PUSH	CX
	MOV	CL,4
	SHR	AL,CL
	POP	CX

	CALL	DIGIT		;Output first digit
HIDIG:
	MOV	AL,BL		;Now do digit saved in BL
DIGIT:
	AND	AL,0FH		;Mask to 4 bits
;Trick 6-byte hex conversion works on 8086 too.
	ADD	AL,90H
	DAA
	ADC	AL,40H
	DAA

;Console output of character in AL

OUT2:
push bx
mov bx, 0

mov ah, 0eh
int 10h
pop bx
ret

dumpcx endp


SectorsPerTrack   dw 0     ;Sectors Per Track
Heads             dw 0     ;Head count
BootDisk db 0



; force padding to 440 bytes of code
org 02b8h

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

