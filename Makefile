obdurate: obdurate.c

segfault: segfault.c

segfaultmaybe: segfaultmaybe.c

clean:
	@-rm obdurate segfault segfaultmaybe *.core 2>/dev/null
