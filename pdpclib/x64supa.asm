; Written by Paul Edwards
; Released to the public domain

.code

public __setj
__setj proc
mov eax, 0
ret
__setj endp

public __longj
__longj proc
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
