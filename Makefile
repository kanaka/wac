CC ?= gcc
# WARNING: GPL license implications
USE_READLINE ?=

ifeq (,$(USE_READLINE))
    RL_LIBRARY ?= edit
else
    RL_LIBRARY ?= readline
    CFLAGS += -DUSE_READLINE=1
endif

# Basic build rules
%.a: %.o
	ar rcs $@ $^

%.o: %.c
	$(CC) -c $(filter %.c,$^) -o $@

test/%.js: test/%.c
	emcc -s WASM=1 $< -o $@

test/%.html: test/%.c
	emcc -s WASM=1 $< -o $@

# Additional dependencies
util.o: util.h
wa.o: wa.h util.h
em.o: util.h

wa.a: util.o


wac: wac.c wa.a
	$(CC) -rdynamic wac.c wa.a -o $@ -lm -ldl -l$(RL_LIBRARY)

wace: wace.c wa.a
	$(CC) -rdynamic wace.c wa.a -o $@ -lm -ldl -l$(RL_LIBRARY)


.PHONY:
clean:
	rm -f *.o *.a wac wace
