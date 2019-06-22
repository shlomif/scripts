; Darwin, amd64, NASM
;
; localarchdir - emit directory name for local platform binaries and non-
; portable scripts (such as is-mute.{Darwin,OpenBSD} over in music)

BITS 64

%define sys_exit  0x2000001
%define sys_write 0x2000004

%define STDOUT_FILENO 1

[section .data align=16]
    ladir: db "Darwin15.6.0-x86_64",10
    .len:  equ $-ladir

[section .text align=16]
global start
start:
    mov rax,sys_write
    mov rdi,STDOUT_FILENO
    mov rsi,ladir
    mov rdx,ladir.len
    syscall
    cmp rdx,0
    jnz error

    mov rax,sys_exit
    mov rdi,0
    syscall

error:
    mov rax,sys_exit
    mov rdi,1
    syscall
