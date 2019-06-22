# Darwin, amd64, GNU-based assembly
#
# false - returns a false value

.section __TEXT,__text
.globl start
start:
    movl    $0x2000001,%eax     # sys_exit
    movw    $1,%di              # exit code
    syscall
