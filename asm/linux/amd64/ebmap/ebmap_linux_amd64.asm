; Linux, amd64, NASM
;
; filter that translates 8-bit values according to a table. should be
; faster than tr(1) though less portable and more difficult to setup for
; a specific translation.

BITS 64

%define sys_read  0
%define sys_write 1
%define sys_exit  60

%define STDIN_FILENO  0
%define STDOUT_FILENO 1
%define STDERR_FILENO 2

section .data
; 8-bit table built with xlate-build-*
    CharMap:
    db 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    db 16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    db 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47
    db 48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    db 64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79
    db 80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95
    db 96,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79
    db 80,81,82,83,84,85,86,87,88,89,90,123,124,125,126,127
    db 128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143
    db 144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159
    db 160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175
    db 176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191
    db 192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207
    db 208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223
    db 224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239
    db 240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255

    ReadErrMsg: db "ebmap: read error",10
    ReadErrLen: equ $-ReadErrMsg

    WriteErrMsg: db "ebmap: write error",10
    WriteErrLen: equ $-WriteErrMsg

section .bss
    READLEN equ 8192
    ReadBuffer: resb READLEN

section .text
global _start
_start:

_readbuf:
    mov rax,sys_read
    mov rdi,STDIN_FILENO
    mov rsi,ReadBuffer
    mov rdx,READLEN
    syscall

    cmp rax,0                   ; EOF (or if negative, error)
    je _exitok
    jl _exitreaderr

    mov rdx,rax                 ; bytes read for sys_write - RDX BEGIN
    xor rbx,rbx                 ; zero as bl used to lookup the character

_translate:
    mov bl,byte [ReadBuffer-1+rax]
    mov bl,byte [CharMap+rbx]
    mov byte [ReadBuffer-1+rax],bl
    dec rax
    jnz _translate

_writebuf:
    mov rax,sys_write
    mov rdi,STDOUT_FILENO
    mov rsi,ReadBuffer
    ; rdx set above for number of bytes to write - RDX END
    syscall

    cmp rax,0
    jl _exitwriteerr

    jmp _readbuf

_emiterrmsg:
    push rdi                    ; preserve exit code
    mov rax,sys_write
    mov rdi,STDERR_FILENO
    syscall
    pop rdi
    ret

_exitreaderr:
    neg rax                     ; exit code is failure value
    mov rdi,rax

    mov rsi,ReadErrMsg
    mov rdx,ReadErrLen
    call _emiterrmsg
    jmp _finish

_exitwriteerr:
    neg rax                     ; exit code is failure value
    mov rdi,rax

    mov rsi,WriteErrMsg
    mov rdx,WriteErrLen
    call _emiterrmsg
    jmp _finish

_exitok:
    mov rdi,0                   ; exit code

_finish:
    mov rax,sys_exit
    syscall
