# Darwin, x86_64, GNU-based assembly
#
# register pollution tests

.section __TEXT,__text
.globl start
start:
# do leftover values in RDI pollute the exit code if only the 16-bit DI
# is touched? if so, should see 131075 or more likely 3 instead of 1
#
#   $ perl -e 'printf "%017b\n", (2**17+2)|1'
#   100000000000000011
#
# if this fails then the larger register will need to be clobbered
    movl    $131074,%edi
    movw    $1,%di              # exit code
# similar concern for RAX versus EAX
    movq    $0xFFFFFFFFFFFFFFFF,%rax
    movl    $0x2000001,%eax     # sys_exit
    syscall
