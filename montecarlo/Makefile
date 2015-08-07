CFLAGS += `pkg-config --libs --cflags goptfoo`
# to make warnings fatal and thus searchable with :make :cnext in vim; requires
#   set makeprg=make\ %:r
# set in vim.
#CFLAGS += -Werror

birds: birds.c

orb-of-fire: orb-of-fire.c

clean:
	@-rm -f birds orb-of-fire *.core 2>/dev/null
