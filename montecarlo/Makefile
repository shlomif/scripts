CFLAGS += `pkg-config --libs --cflags goptfoo`
# to make warnings fatal and thus searchable with :make :cnext in vim; requires
#   set makeprg=make\ %:r
# set in vim.
#CFLAGS += -Werror

birds: birds.c

clean:
	@-rm -f birds *.core 2>/dev/null
