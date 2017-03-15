; IA-32, Linux, NASM
;
; Emits the contents of various 32-bit registers hex encoded to standard
; output. Only tested on a little endian system, where the output agrees
; with what gdb(1) shows.
;
; A practical use might be to determine the contents of the registers
; following some call (e.g. CPUID) on some ancient host where gdb is not
; installed, but where this binary can be copied to and run (or written
; up by hand, the stripped binary being not too very long). In hindsight,
; there is a cpuid(1) tool available for Linux.

BITS 32

org 0x08048000                  ; default executable load address

ehdr:                           ; Elf32_Ehdr
    db          0x7f,"ELF",1,1,1,0
    times 8 db  0
    dw          2               ; e_type
    dw          3               ; e_machine
    dd          1               ; e_version
    dd          _start          ; e_entry
    dd          phdr - $$       ; e_phoff
    dd          0               ; e_shoff
    dd          0               ; e_flags
    dw          ehdrsize        ; e_ehsize
    dw          phdrsize        ; e_phentsize
    dw          1               ; e_phnum
    dw          0               ; e_shentsize
    dw          0               ; e_shstrndx

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
; alternative to figuring out enough of ELF to get a read/write .data section
    mov eax,192         ; sys_mmap2
    xor ebx,ebx         ; NULL addr
    mov ecx,16          ; length (a bit of overkill here)
    mov edx,3           ; read/write access
    mov esi,34          ; map private/anon
    mov edi,-1          ; fd
    xor ebp,ebp         ; offset
    int 80H

    test eax,eax
    jz _errexit

    mov esi,eax         ; where we can write to thanks to mmap2

    mov eax,1           ; processor type and etc
    cpuid

    call _emit_eax
    mov eax,ebx
    call _emit_eax
    mov eax,ecx
    call _emit_eax
    mov eax,edx
    call _emit_eax

    mov ebx,0           ; exit code
_finish:
    mov eax,1           ; sys_exit
    int 80h

_errexit:
    mov ebx,1           ; exit code
    jmp _finish

_emit_eax:
    pushad              ; preserve registers
    ;eax                ; register to print
    xor ebx,ebx         ; hex digit lookup
    ;cx                 ; looked up hex digits
    mov edx,3           ; byte positions to count through
    ;esi                ; writable memory from somewhere above
    mov word [esi+8],10 ; trailing newline
    .next_byte:
        mov bl,al       ; byte to lookup from eax
        mov cx,word [Digits+ebx*2] ; lookup hex digits via bl
        mov word [esi+edx*2],cx ; place result into string
        shr eax,8       ; shift over the next byte
        sub dl,1        ; decrement edx (sub sets flag for jae)
        jae .next_byte
    mov eax,4           ; sys_write
    mov ebx,1           ; stdout
    mov ecx,esi
    mov edx,9           ; eight hex chars plus the newline
    int 80h             ; print the string
    popad               ; restore registers
    ret

; perl -e 'for(0..255){printf "%02X",$_; print "\n" if $_%16==15}' | ...
Digits: db "000102030405060708090A0B0C0D0E0F"
        db "101112131415161718191A1B1C1D1E1F"
        db "202122232425262728292A2B2C2D2E2F"
        db "303132333435363738393A3B3C3D3E3F"
        db "404142434445464748494A4B4C4D4E4F"
        db "505152535455565758595A5B5C5D5E5F"
        db "606162636465666768696A6B6C6D6E6F"
        db "707172737475767778797A7B7C7D7E7F"
        db "808182838485868788898A8B8C8D8E8F"
        db "909192939495969798999A9B9C9D9E9F"
        db "A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
        db "B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
        db "C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
        db "D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
        db "E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
        db "F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF"

filesize equ $-$$
