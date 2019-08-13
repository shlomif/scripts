include(`../../../lib/m4/cf.m4')dnl
divert(-1)
OpenBSD, amd64, NASM

localarchdir - emit directory name for local platform binaries and
non-portable scripts (such as is-mute.{Darwin,OpenBSD} over in music)

asociar(`CUR_LAD', `printf "OpenBSD%s-%s" $(uname -r) $(uname -m)')
divert(0)dnl
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
    ladir db "CUR_LAD",10
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
