; minimal (as far as I can make it, by executable size) "Hello World"
; for Linux amd64 with nasm

BITS 64

org 0x08048000                  ; default executable load address

ehdr:                           ; Elf64_Ehdr
    db          0x7f,"ELF"

; wrong wrong wrong wrong wrong but it saves us a few bytes (why does
; Linux even let this run?)
_exit:  mov rdi,0               ; exit code
        mov rax,60              ; sys_exit
        syscall

    dw          2               ; e_type
    dw          62              ; e_machine
    dd          1               ; e_version
    dq          _start          ; e_entry
    dq          phdr - $$       ; e_phoff
    dq          0               ; e_shoff
    dd          0               ; e_flags
    dw          ehdrsize        ; e_ehsize
    dw          phdrsize        ; e_phentsize
    dw          1               ; e_phnum
    dw          0               ; e_shentsize
    dw          0               ; e_shstrndx
ehdrsize equ $-ehdr

phdr:                           ; Elf64_Phdr
    dd          1               ; p_type
    dd          5               ; p_flags
    dq          0               ; p_offset
    dq          $$              ; p_vaddr
    dq          $$              ; p_paddr
    dq          filesize        ; p_filesz
    dq          filesize        ; p_memsz
    dq          0x1000          ; p_align
phdrsize equ $-phdr

_start: mov rax,1               ; sys_write
        mov rdi,1               ; stdout
        mov rsi,str
        mov rdx,str.len
        syscall
        jmp _exit

str:    db "Hello World",10
.len:   equ $-str

filesize equ $-$$
