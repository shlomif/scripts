#!/usr/sbin/dtrace -q -s

/* stat could also be stat64 on Mac OS X, luckily globs work \o/ */
syscall::stat*:entry {
  self->file = copyinstr(arg0);
  printf("%Y %s[%d] %s\n", walltimestamp, execname, pid, self->file);
}
