# Darwin, x86_64, GNU-based assembly
#
# echo - write arguments to the standard output, plus some support for
# complications added over time

.equ SYS_EXIT,  0x2000001
.equ SYS_WRITE, 0x2000004

.section __DATA,__data
    newline: .ascii "\n"

.section __TEXT,__text
.globl start
start:
    movq %rsp,%rbp
    movq (%rbp),%r12            # argc (includes program name)
    andq $-16,%rsp              # manually align stack to 16
                                # bytes because Apple does not
                                # follow System V ABI for AMD64
    cmpq $1,%r12                # only the program name?
    je _emitnl

    addq $16,%rbp               # move past argc and name

    xorq %r14,%r14              # -n flag off by default
    movq (%rbp),%rbx
    movzbl (%rbx),%eax
    cmpb $0x2d,%al              # '-'
    jne _firstarg
    movzbl 1(%rbx),%eax
    cmpb $0x6e,%al              # 'n'
    jne _firstarg
    movzbl 2(%rbx),%eax
    testb %al,%al               # '\0'
    jnz _firstarg

    cmp $2,%r12                 # was -n the only argument?
    je _finish
    movq $1,%r14                # enable -n flag
    addq $8,%rbp                # and step past this argument
    decq %r12

_firstarg:
    cld                         # up-memory
    movq (%rbp),%rdi            # in RDI
    xorb %al,%al                # search for '\0'
# ARG_MAX from /usr/include/sys/syslimits.h on Mac OS X 10.11.6
# actual limit will be below this as exec includes environ(7)
    movl $0x40000,%ecx          # within this limit of memory

_nextarg:
    repne scasb
# however ignore ARG_MAX limit as the odds of not seeing the necessary
# number of NUL in memory is quite low
#   jnz _error                  # no '\0' found in limit ??
    movb $0x20,-1(%rdi)         # '\0' becomes ' '
    decq %r12
    cmp $1,%r12                 # only the program name left?
    ja _nextarg

    subq $1,%rdi                # fix length as scasb steps past

# "iBCS2 compatible systems" add a trailing "\c" complication
    movzbl -2(%rdi),%eax
    cmpb $0x5c,%al              # '\'
    jne _printargs
    movzbl -1(%rdi),%eax
    cmpb $0x63,%al              # 'c'
    jne _printargs
    movq $1,%r14                # enable -n flag
    subq $2,%rdi                # omit "\c" from buffer to print

_printargs:
    movl $SYS_WRITE,%eax
    movq %rdi,%rdx
    subq (%rbp),%rdx            # length
    movq $1,%rdi                # stdout
    movq (%rbp),%rsi            # what to print
    syscall
    cmp $0,%rdx
    jnz _error

    test %r14,%r14              # need trailing newline?
    jnz _finish
_emitnl:
    movl $SYS_WRITE,%eax
    movl $1,%edi                # stdout
    movq $1,%rdx                # length
    leaq newline(%rip),%rsi     # what
    syscall
    cmp $0,%rdx
    jnz _error

_finish:
    movl $SYS_EXIT,%eax
    xorw %di,%di                # exit code
    syscall

_error:
    movl $SYS_EXIT,%eax
    movw $1,%di                 # exit code
    syscall
