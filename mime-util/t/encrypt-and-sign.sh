#!/bin/sh -x
#
# To generate S/MIME bundles to test against. If using a self-signed
# certificate, use 'openssl smime -verify -noverify' to avoid 'self
# signed certificate in chain' errors.

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

# plus a decrypt+verify pass
openssl smime -decrypt -in message.sign-encrypt \
  -inkey test.key -recip test.cert \
| openssl smime \
  -verify -noverify -signer test.cert \
  -out message.decrypt-verify
