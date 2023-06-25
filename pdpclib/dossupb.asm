; dossupb.asm - assembler support functions for DOS
;
; This program written by Paul Edwards
; Released to the public domain

; For functions/variables that aren't C so shouldn't
; have C as the language

% .model memodel

public FIDRQQ
public FIWRQQ
public FIERQQ
public FJARQQ
public FIARQQ
public FJCRQQ
public FICRQQ
public FJSRQQ
public FISRQQ

.data

FIDRQQ  dw  ?
FIWRQQ  dw  ?
FIERQQ  dw  ?
FJARQQ  dw  ?
FIARQQ  dw  ?
FJCRQQ  dw  ?
FICRQQ  dw  ?
FJSRQQ  dw  ?
FISRQQ  dw  ?

end
