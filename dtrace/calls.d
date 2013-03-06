#!/usr/sbin/dtrace -q -s

/*
Trace calls for a given PID. As a handy wrapper to get the PID:

  #!/bin/sh
  echo $$
  sleep 3
  exec ...

But there's probably a better way. (Following forks would be more work.)
*/

:::entry /pid == $1/ {
  printf("-> %s(%d, 0x%x, %4d)\n", probefunc, arg0, arg1, arg2);
}

:::return /pid == $1/ {
  printf("\t\t = %d - %d\n", arg1, errno);
}
