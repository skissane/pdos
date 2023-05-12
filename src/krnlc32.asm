; support routines
; written by Paul Edwards
; released to the public domain

.386p
.model flat

.code


extrn __imp__CreateDirectoryA@8:ptr
public _CreateDirectoryA
_CreateDirectoryA:
push 8[esp]
push 8[esp]
call __imp__CreateDirectoryA@8
ret
ret


end
