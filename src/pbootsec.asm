; pbootsec.asm - pdos boot sector
;
; This program written by Paul Edwards
; Released to the public domain
; Revamped by William Bryan.
; All changes remain public domain

; This code will be loaded to location 07C00h by the BIOS
; We set the stack at 07C00h and make it grow down

; Registers at entry are:
; CS:IP = 07c00H, almost always 0000:7c00
; DL = drive number
; nothing else guaranteed

; We should store the drive number in BPB[25], since
; pload is expecting it there.  pload also expects
; that the CS:IP will have an IP of 0 when it is called.
; It also expects some sort of stack to be set up.

% .model memodel

_DATA   segment word public 'DATA'
_DATA   ends
_BSS segment word public 'BSS'
_BSS ends

_TEXT segment word public 'CODE'

org 0100h
top:

public start
start proc

;Jump over our BPB
 jmp bypass
 nop

; HD:
;
; 000000  EB479050 444F5320 782E78
;         00 02  bytes per sector
;           04   sectors per cluster
;          0100  # reserved
;             .G.PDOS x.x.....
; 000010  02           # fats
;     0002          # directory entries (512)
;      01 EC        total logical sectors
;     F8      media byte
;       3B00     sectors per FAT
;
; 3b=59*2 + 32 (directory sectors) + 1 (reserved) = 151
;
;         3F00      sectors per track (63)
;          1000     number of heads (16)
;         3F00   hidden (63)
; 151 + hidden=63 = 214
; 214-3*63 = 25 + 1 (1-based) = 26
;          0000  ......;.?...?...
; 000020  00000000 800029EC 1C591E4E 4F204E41  ......)..Y.NO NA
; 000030  4D452020 20204641 54313620 202031C0  ME FAT16   1.
; 000040  FA8ED0BC 007CFB0E 1FB80000 8ED0BC00  .....|..........
; 000050  7C0E1FE8 5D00B870 0050B800 0050CB06  |...]..p.P...P..
; 000060  53505152 1EB80000 8EC0BB78 00268B47  SPQR.......x.&.G
; 000070  028ED826 8B078BD0 B800008E C0BB3E7C  ...&..........>|
; 000080  B9000083 F90B7D0E 538BDA8A 075B2688  ......}.S....[&.
; 000090  07424341 EBEDB800 008EC0BB 780026C7  .BCA........x.&.
;
; floppy:
;
; 000000  EB3C904C 494E5558 342E31
;         00 02  bytes per sector
;           01   sectors per cluster
;          0100  # reserved
;             .<.LINUX4.1.....
; 000010  02           # fats
;     E000          # directory entries (224)
;      40 0B        total logical sectors
;     F0      media byte
;       0900     sectors per FAT
;
; 9*2 + 224*32/512 + 1 = 33
;         1200      sectors per track (18)
;          0200     number of heads (2)
;         0000   hidden
; 33 - 18 = 15 + 1 (1-based) = 16
;          0000  ...@............
; 000020  00000000 000029E3 7FFA4420 20202020  ......)...D
; 000030  20202020 20204641 54313220 2020FAFC  FAT12   ..
; 000040  31C08ED8 BD007CB8 E01F8EC0 89EE89EF  1.....|.........
; 000050  B90001F3 A5EA5E7C E01F0000 60008ED8  ......^|....`...
; 000060  8ED08D66 A0FB807E 24FF7503 885624C7  ...f...~$.u..V$.
; 000070  46C01000 C746C201 00E8E900 46726565  F....F......Free
; 000080  444F5300 8B761C8B 7E1E0376 0E83D700  DOS..v..~..v....
; 000090  8976D289 7ED48A46 1098F766 1601C611  .v..~..F...f....


;bpb db 51 dup(?)

;Lets make an actual BPB
; offset 3
OEMName           db 'PDOS x.x'
; offset b (should be considered offset 0 in other code)
BytesPerSector    dw 512   ;512 bytes is normal ;)
; offset d
SectorsPerClustor db 1     ;Sector == cluster
; offset e
ReservedSectors   dw 1     ;Reserve the boot sector
; offset 10
FatCount          db 2     ;2 copies of the fat is standard
; offset 11
RootEntries       dw 224   ;# of root entries
; offset 13
TotalSectors16    dw 0     ;# of sectors on the disk
; offset 15
MediaType         db 0f8h  ;Ignored for most things
; offset 16
FatSize16         dw 0     ;Size of a single fat in sectors
; offset 18
SectorsPerTrack   dw 0     ;Sectors Per Track
; offset 1a
Heads             dw 0     ;Head count
; offset 1c (DOS 3.31+)
HiddenSectors_Low  dw 0    ;Lower 16-bits of hidden sector count
HiddenSectors_High dw 0    ;Upper 16-bits of hidden sector count
; offset 20
TotalSectors32    dd 0     ;# of sectors, 32-bit!

;Start of our Fat12/16 info
; offset 24
DriveNum    db  0     ;Drive number (0x0x - floppy, 0x8x - hd
; offset 25
Reserved    db  0     ;Reserved, set to 0
; offset 26
BootSig     db  029h    ;Set to 0x29 if next 3 values are present
; offset 27
VolumeID    dd  0     ;Volume's ID, we don't care for it ;)
; offset 2b
VolumeLabel db  'PDOS Volume'  ;11 bytes
; offset 36
FileSystem  db  'FAT     '   ;File system (FAT12 or FAT16)

; offset 3e and onwards is our own code

BootDisk db 0
; new disk parameter table
;dpt db 11 dup(?)


bypass:
;Always clear direction bit first, just incase bios leaves it unset
 cld
 ;This should grab our current instruction pointer
 call GetIP
 GetIP:
 pop  ax
 and ax, 0ff00h
 cmp  ax, 0100h
 je  Skip
  ;Patch DS here if we aren't a com file
  mov  ax, 07b0h
  mov  ds, ax
  mov  ax, 0
  mov  es, ax
  mov es:[07C02h],dl  ; store boot disk in nop
 Skip:
 xor  ax, ax   ;Zero ax
;setting ss and sp must be paired
 mov  ss, ax
 mov  sp, 07c00h

 mov  [BootDisk], dl   ;Store our boot disk

; get disk geometry from BIOS instead of relying on values
; stored at format time, because the disk may have been moved
; to a different machine with a different faking mechanism
 mov ah, 08h
 mov di, 0
; es should already be 0
; dl is already set
 push es  ; preserve
 int 13h
 jc ignorec   ; if it fails, just use the values at format time

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

ignorec:
 pop es

 call CalculateLocation   ;Gets our data sector into dx:ax

 mov  cx, 55        ;Load 55 sectors (was 58, was 3)
; I dropped down to 55 in case we have a cdrom with 2048 byte
; sectors of which just the first 512 bytes are populated
 mov  bx, 0700h     ;Loaded to es:bx (0x00:0x0700)
 call ReadSectors   ;Read the actual sectors

 mov  ax, 0070h
 push ax
 mov  ax, 0
 push ax
 retf
start endp

;Calculates the location of our file to load (after Fat, after hidden 
;sectors, etc)
;Inputs:
; (None)
;Outputs:
; dx:ax - sector to load
CalculateLocation proc
;In order to calculate our data start:
;RootStart = ReservedSectors + FatCount * FatSectors
;DataStart = RootStart + RootEntryCount * 32 / BytesPerSector

;Store bx and cx so they aren't trashed
 push bx
 push cx

;Lets first start calculating the dynamic values
 mov ax, [FatSize16]
 xor dx, dx
 xor cx, cx
 mov cl, [FatCount]
 mul cx ;Multiply by a word to ensure dx:ax is the result
; DX:AX = FatSize16 * FatCount

;Store the results for later
 push ax
 push dx

;Zero dx before our multiply
 xor dx, dx
 mov ax, [RootEntries]
 mov cx, 32
 mul cx
; DX:AX = RootEntries * 32
 div word ptr [BytesPerSector]   ;Divide by # of bytes per sector
; AX = RootEntries * 32 / BytesPerSector
; DX = RootEntries * 32 % BytesPerSector

 pop dx ;Restore DX to original value
 pop bx ;Restore old value of AX into BX

 xor cx, cx ;Zero cx for later use (note that this clears the carry flag)

;So, we must add BX into AX to combine the numbers
 add ax, bx

;But, if it overflows (carry flag set), we must increment DX to compensate
 adc dx, cx ;ADC will add 0 (cx) and if carry flag set add 1

;Now we must add our reserved and hidden sectors
 add ax, [ReservedSectors]
 adc dx, cx ;ADC will add 0 (cx) and if carry flag set add 1

 add ax, [HiddenSectors_Low]
 adc dx, [HiddenSectors_High] ;ADC will add 0 (cx) and if carry flag set add 1

;Restore cx and bx
 pop  cx
 pop  bx
 ret
CalculateLocation endp

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
; dx:ax - sector to read
; es:bx - dest
;Outputs:
; (None)
ReadSingleSector proc
 push ax
 push bx
 push cx
 push dx
 push es
 call Lba2Chs     ;Grab our CHS
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

;Read multiple sectors
;Inputs:
; dx:ax - starting LBA sector
; es:bx - dest
; cx - # of sectors
;Outputs:
; (None)
ReadSectors proc
 push ax
 push bx
 ReadNextSector:
  call ReadSingleSector
  add  bx,  [BytesPerSector] ;Next sector
  add  ax,1                  ;Next LBA
  adc  dx,0                  ;If we had a carry, we must increment dx
  loop ReadNextSector        ;Until cx = 0
 pop  bx
 pop  ax
 ret
ReadSectors endp


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


org 02feh
lastword dw 0aa55h

_TEXT ends

end top

