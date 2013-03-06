#!/usr/sbin/dtrace -q -s

syscall::open:entry {
  self->file = copyinstr(arg0);
  printf("%Y %s[%d] %s\n", walltimestamp, execname, pid, self->file);
}
