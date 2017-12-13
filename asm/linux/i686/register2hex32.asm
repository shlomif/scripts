; Linux, i686, NASM
;
; emits the contents of various registers hex encoded to standard out
;
; a practical use might be to determine the contents of the registers
; following some call (e.g. CPUID) on some ancient host that lacks gdb
; but where this binary can be copied or written. in hindsight, there is
; a cpuid(1) tool for Linux

BITS 32

org 0x08048000                  ; default executable load address

ehdr:                           ; Elf32_Ehdr
    db         0x7f,"ELF",1,1,1,0
    times 8 db 0
    dw         2                ; e_type
    dw         3                ; e_machine
    dd         1                ; e_version
    dd         _start           ; e_entry
    dd         phdr - $$        ; e_phoff
    dd         0                ; e_shoff
    dd         0                ; e_flags
    dw         ehdrsize         ; e_ehsize
    dw         phdrsize         ; e_phentsize
    dw         1                ; e_phnum
    dw         0                ; e_shentsize
    dw         0                ; e_shstrndx
ehdrsize equ $-ehdr

phdr:                           ; Elf32_Phdr
    dd          1               ; p_type
    dd          0               ; p_offset
    dd          $$              ; p_vaddr
    dd          $$              ; p_paddr
    dd          filesize        ; p_filesz
    dd          filesize        ; p_memsz
    dd          5               ; p_flags
    dd          0x1000          ; p_align

phdrsize equ $-phdr

_start:
; alternative: learn more about ELF and create a R/W .data section
    mov eax,192                 ; sys_mmap2
    xor ebx,ebx                 ; NULL addr
    mov ecx,16                  ; length (a bit of overkill here)
    mov edx,3                   ; read/write access
    mov esi,34                  ; map private/anon
    mov edi,-1                  ; fd
    xor ebp,ebp                 ; offset
    int 80H

    test eax,eax
    jz _errexit

    mov edi,eax

; TWEAK or instead your code here
    mov eax,1                   ; processor type and etc
    cpuid

; show a bunch of the registers
    call _emit_eax
    mov eax,ebx
    call _emit_eax
    mov eax,ecx
    call _emit_eax
    mov eax,edx
    call _emit_eax

    xor ebx,ebx                 ; exit code
_finish:
    mov eax,1                   ; sys_exit
    int 80h

_errexit:
    mov ebx,1
    jmp _finish

_emit_eax:
    pushad
    xor ebx,ebx
    mov ecx,32
    .next_byte:
        mov esi,eax
        sub ecx,4
        sar esi,cl
        and esi,0xF
        mov dl,byte [HHH+esi]
        mov byte [edi+ebx],dl
        inc ebx
        test ecx,ecx
        jnz .next_byte
    mov byte [edi+8],10         ; trailing newline
    mov eax,4                   ; sys_write
    mov ebx,1                   ; stdout
    mov ecx,edi
    mov edx,9                   ; eight hex chars plus newline
    int 80h
    test eax,eax
    js _errexit
    popad
    ret

HHH: db "0123456789ABCDEF"

filesize equ $-$$
