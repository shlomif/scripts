# Darwin, x86_64, GNU-based assembly
#
# true - returns a true value

.section __TEXT,__text
.globl start
start:
    movl    $0x2000001,%eax     # sys_exit
    xorw    %di,%di             # exit code
    syscall
