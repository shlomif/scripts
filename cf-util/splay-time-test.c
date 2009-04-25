/*
Based on code at http://cfwiki.org/cfwiki/index.php/SplayTime_testing

Usage: splay-time-test host.example.org [host2 ..]

TODO what do the output times mean? miliseconds?
*/

#include <stdio.h>

#define CF_MACROALPHABET 61    /* a-z, A-Z plus a bit */
#define CF_HASHTABLESIZE 4969 /* prime number */

int main(int argc, char * argv[]) {
  int i, time, hash;
  char *temp, *prompt;

  time = 1; /* equivalent to SplayTime = ( N ) */

  for (i = 1; i < argc; i++) {
    hash = str2hash(argv[i]);
    printf("host=%s hash=%d time=%d\n", argv[i], hash, (int)(time*60*hash/CF_HASHTABLESIZE));
  }
}

int str2hash(char *name) {
  int i, slot = 0;

  for (i = 0; name[i] != '\0'; i++) {
    slot = (CF_MACROALPHABET * slot + name[i]) % CF_HASHTABLESIZE;
  }

  return slot;
}
