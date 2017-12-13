# Darwin, x86_64, GNU-based assembly
#
# hexip4todottedquad - converts raw hex IPv4 address to the more
# typical dotted quad form

.equ SYS_EXIT,  0x2000001
.equ SYS_WRITE, 0x2000004

.section __DATA,__data
    ipbuf: .ascii "xxxxxxxxxxxxxxxn"

.section __TEXT,__text
.globl start
start:
    movq %rsp,%rbp
    andq $-16,%rsp              # manually align stack to 16
                                # bytes because Apple does not
                                # follow System V ABI for AMD64
    cmpq $2,(%rbp)              # single arg? (plus program name)
    jne _failure

    xorl %eax,%eax              # evict any stray bits
    xorq %r12,%r12              # high or low byte toggle
    xorq %r14,%r14              # number of octets seen

    addq $16,%rbp               # move past argc and name
    leaq ipbuf(%rip),%rsi       # what to print
    movq %rsi,%rdi              # where in ipbuf output goes

    movq (%rbp),%r13            # load first argument
_nextchar:
    movzbl (%r13),%edx
    cmpb $0,%dl                 # is the input incomplete?
    je _failure

    cmpb $0x30,%dl              # '0'
    jb _failure
    cmpb $0x39,%dl              # '9'
    ja _maybe_upcase
    subb $0x30,%dl              # obtain 0..9
    jmp _convert

_maybe_upcase:
    cmpb $0x41,%dl              # 'A'
    jb _failure
    cmpb $0x46,%dl              # 'F'
    ja _maybe_lowcase
    subb $0x37,%dl              # obtain 10..15
    jmp _convert

_maybe_lowcase:
    cmpb $0x61,%dl              # 'a'
    jb _failure
    cmpb $0x66,%dl              # 'f'
    ja _failure
    subb $0x57,%dl              # obtain 10..15

_convert:
    test %r12,%r12
    jnz _low
    movb %dl,%al                # high: clobber existing value
    shlb $4,%al                 # and scoot those bits over
    xorq $1,%r12                # toggle next byte low

    incq %r13
    jmp _nextchar

_low:
    xorb %dl,%al                # low: bool the bits in
    xorq $1,%r12                # toggle next byte high

    call _hextonum

    movb $0x2e,(%rdi)           # '.' after all octets
    addq $1,%rdi

    incq %r13                   # next char in input
    incq %r14                   # octet count
    cmpq $4,%r14                # correct number of octets? 
    je _print

    jmp _nextchar

_print:
    movzbl (%r13),%edx
    cmpb $0,%dl                 # exactly done with the input?
    jne _failure

    movb $0x0a,-1(%rdi)         # '\n' (clobbers last '.')
    movq %rdi,%rdx
    subq %rsi,%rdx              # length
    movl $SYS_WRITE,%eax
    movq $1,%rdi                # stdout
    syscall

    movl $SYS_EXIT,%eax
    xorw %di,%di                # exit code
    syscall

_failure:
    movl $SYS_EXIT,%eax
    movw $1,%di                 # exit code
    syscall

# AX must contain the hex value; EDI is written to and advanced
_hextonum:
    push %rax
    push %rbx
    push %rdx
    xorl %edx,%edx              # DX is an input to DIVW
                                # also to clear high bits for lea
    movq $10,%rbx               # /10
    divw %bx
    testw %ax,%ax
    je _hextonum_print          # all done, print the digits
    call _hextonum              # recurse to next digit

_hextonum_print:
    lea 0x30(%edx),%eax         # remainder + '0' offset
    movb %al,(%edi)
    incl %edi
    pop %rdx                    # restores next remainder as unrecurse
    pop %rbx
    pop %rax
    ret
