#!/bin/sh

# Never, ever, ever use /tmp/sym or any sort of shared directory. Ever. Well,
# unless you like featuring your code in exploits or CVEs or the like. Instead,
# use /var/run or some application specific directory with minimum permissions,
# proper handling for crashes and orphaned pid files, etc etc etc. (Or, if you
# do need to use a temporary file, well, there's mktemp(1) and related.
LOCKFILE=sym

if [ -r $LOCKFILE ]; then
  exit 1
fi

# And the $100 question is, how much time exists between the above test and the
# below clobber, and how easily can bruteln make the naughty symlink? (Hint: it
# depends on how busy the system is. Assume well more than enough for any
# attack to succeed.)

echo notevil >> $LOCKFILE

# ... unimportant locked processing here ...

rm $LOCKFILE
exit 0
