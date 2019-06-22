; OpenBSD, amd64, NASM
;
; localarchdir - emit directory name for local platform binaries and
; non-portable scripts (such as is-mute.{Darwin,OpenBSD} over in music)

BITS 64

%define sys_exit  1
%define sys_write 4

%define STDOUT_FILENO 1

section .note.openbsd.ident
    align 2
    dd 8, 4, 1
    db 'OpenBSD',0
    dd 0
    align 2

section .data
    ladir db "OpenBSD6.5-amd64",10
    .len equ $-ladir

section .text
global _start
_start:
    mov rax,sys_write
    mov rdi,STDOUT_FILENO
    mov rsi,ladir
    mov rdx,ladir.len
    syscall
    cmp rax,ladir.len
    jne error

    mov rax,sys_exit
    xor rdi,rdi
    syscall

error:
    mov rax,sys_exit
    mov rdi,1
    syscall
