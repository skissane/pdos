# mfsupa - mainframe support code in assembler
# Written by Paul Edwards
# Released to the public domain

         .text
         .balign 4
         .using .,r15
;         MVC   92(8,r13),=D'1.0E0'
         L     r15,=F'3'
         BR    r14
         .balign 8
         .ltorg
