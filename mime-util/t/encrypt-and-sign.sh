#!/bin/sh -x
#
# $Id$
#
# To generate S/MIME bundles to test against. If using a self-signed
# certificate, use 'openssl -verify -noverify' to avoid 'self signed
# certificate in chain' error.

# sign
openssl smime \
  -text -in message.txt -out message.sign \
  -sign -signer test.cert -inkey test.key

# encrypt
openssl smime \
  -text -in message.txt -out message.encrypt \
  -encrypt -des3 test.cert

# all of above
openssl smime \
  -in message.txt -text \
  -sign -signer test.cert -inkey test.key \
| openssl smime \
  -out message.sign-encrypt \
  -encrypt -des3 test.cert

# and decrypt for raw output from OpenSSL
openssl smime -decrypt -in message.encrypt \
  -out message.decrypt \
  -inkey test.key -recip test.cert
