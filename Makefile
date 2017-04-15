CC = gcc -std=gnu99 -m32

# WARNING: GPL license implications from using READLINE
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

# C example build rules
examples_c/%.js: examples_c/%.c
	emcc -s WASM=1 -s RELOCATABLE=1 -O2 -s USE_SDL=2 $< -o $@

examples_c/%.html: examples_c/%.c
	emcc -s WASM=1 -s RELOCATABLE=1 -O2 -s USE_SDL=2 $< -o $@

examples_c/%: examples_c/%.c
	$(CC) $< -o $@ -lSDL2

.SECONDARY:
examples_c/%.wasm: examples_c/%.js
	@true

.SECONDARY:
examples_c/%.wast: examples_c/%.wasm
	wasm2wast $< -o $@


# Wast example build rules
examples_wast/%.wasm: examples_wast/%.wast
	wast2wasm $< -o $@


# Additional dependencies
util.o: util.h
wa.o: wa.h util.h
em.o: util.h

wa.a: util.o


wac: wac.c wa.a
	$(CC) -rdynamic wac.c wa.a -o $@ -lm -ldl -l$(RL_LIBRARY)

wace: wace.c wa.a em.o
	$(CC) -rdynamic $^ -o $@ -lm -ldl -lSDL2 -lEGL -lGL


.PHONY:
clean:
	rm -f *.o *.a wac wace wace-sdl.c \
	    examples_c/*.js examples_c/*.html \
	    examples_c/*.wasm examples_c/*.wast \
	    examples_wast/*.wasm
