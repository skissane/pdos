; Written by Paul Edwards
; Released to the public domain

.code

; I think rcx will have env
; And it looks like we only need to save rbx, rcx, rdx, r8, r9, rsp
; plus return address
; And first location is reserved for return value
; (second is for return address)
public __setj
__setj proc
mov [rcx + 8*2], rbx
mov [rcx + 8*3], rcx
mov [rcx + 8*4], rdx
mov [rcx + 8*5], r8
mov [rcx + 8*6], r9
mov [rcx + 8*7], rsp
mov rax, [rsp]
mov [rcx + 8*1], rax
; we only return int (eax), but since we're not preserving
; rax anyway (maybe we should?), may as well clear it.
xor rax,rax
ret
__setj endp

; I think rcx will have env
public __longj
__longj proc
mov rax, [rcx + 8*7]
mov rsp, rax
mov rax, [rcx + 8*1]
mov [rsp], rax
mov rbx, [rcx + 8*2]
mov rdx, [rcx + 8*4]
mov r8, [rcx + 8*5]
mov r9, [rcx + 8*6]
mov rax, [rcx]
mov rcx, [rcx + 8*3]
ret
__longj endp

public __main
__main proc
	ret
__main endp

.data

public _fltused
_fltused dd ?

end
