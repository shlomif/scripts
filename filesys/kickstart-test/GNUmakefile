CFLAGS=-O2 -std=c99 -Wall -fno-diagnostics-color -fstack-protector-all -fPIE -pie -pipe

fakeopen.so : fakeopen.c
	$(CC) $(CFLAGS) -shared -fPIC -ldl fakeopen.c -o fakeopen.so

ks.pre : extract-ks-scripts fakeopen.so
	@test $${ks:?"usage: make ks.pre ks=kickstart-file"}
	./extract-ks-scripts $(ks)
	./testks fakeopen.so < ks.pre

ks.post : extract-ks-scripts fakeopen.so
	@test $${ks:?"usage: make ks.post ks=kickstart-file"}
	./extract-ks-scripts $(ks)
	./testks fakeopen.so < ks.post

.PHONY: ks.pre ks.post
