##########################################################
# User configurable build options

# libc or fooboot
PLATFORM = libc

# WARNING: GPL license implications from using READLINE
USE_READLINE ?=

USE_SDL ?= 1

#CFLAGS ?= -O2 -Wall -Werror -Wextra -MMD -MP
CFLAGS ?= -O2 -Wall -Werror -MMD -MP

EXTRA_WAC_LIBS ?=
EXTRA_WACE_LIBS ?=


##########################################################

CC = gcc $(CFLAGS) -std=gnu99 -m32 -g
EMCC = emcc $(CFLAGS) -s WASM=1 -s SIDE_MODULE=1 -s LEGALIZE_JS_FFI=0

WA_DEPS = util.o thunk.o

ifeq (libc,$(PLATFORM))
  CFLAGS += -DPLATFORM=1
  ifeq (,$(strip $(USE_READLINE)))
    RL_LIBRARY ?= edit
  else
    RL_LIBRARY ?= readline
    CFLAGS += -DUSE_READLINE=1
  endif
  WAC_LIBS = m dl $(RL_LIBRARY)
  WAX_LIBS = m dl
  WACE_LIBS = m dl $(RL_LIBRARY)
  ifneq (,$(strip $(USE_SDL)))
    WACE_LIBS += SDL2 SDL2_image GL glut
  endif
else
ifeq (fooboot,$(PLATFORM))
  CFLAGS += -DPLATFORM=2
else
  $(error unknown PLATFORM: $(PLATFORM))
endif
endif

WAC_LIBS += $(EXTRA_WAC_LIBS)
WAX_LIBS += $(EXTRA_WAX_LIBS)
WACE_LIBS += $(EXTRA_WACE_LIBS)

# Basic build rules
.PHONY:
all: wac wax wace

%.a: %.o
	ar rcs $@ $^

%.o: %.c
	$(CC) -c $(filter %.c,$^) -o $@

# Additional dependencies
util.o: util.h
wa.o: wa.h util.h platform.h
thunk.o: wa.h thunk.h
wa.a: util.o thunk.o platform_$(PLATFORM).o
wac: wa.a wac.o
wax: wa.a wasi.o wax.o
wace: wa.a wace.o

#
# Platform
#
ifeq (libc,$(PLATFORM)) # libc Platform
wac:
	$(CC) -rdynamic -Wl,--no-as-needed -o $@ \
	    -Wl,--start-group $^ -Wl,--end-group $(foreach l,$(WAC_LIBS),-l$(l))
wax:
	$(CC) -rdynamic -Wl,--no-as-needed -o $@ \
	    -Wl,--start-group $^ -Wl,--end-group $(foreach l,$(WAX_LIBS),-l$(l))
wace: wace_emscripten.o
	$(CC) -rdynamic -Wl,--no-as-needed -o $@ \
	    -Wl,--start-group $^ -Wl,--end-group $(foreach l,$(WACE_LIBS),-l$(l))

else  # fooboot OS platform

  FOO_TARGETS = wac wax wace
  include fooboot/Makefile

wace: wace_fooboot.o
endif


.PHONY:
clean::
	rm -f *.o *.a *.d wac wax wace wace-sdl.c \
	    lib/*.o lib/*.d kernel/*.o kernel/*.d \
	    examples_c/*.js examples_c/*.html \
	    examples_c/*.wasm examples_c/*.wat \
	    examples_wat/*.wasm

##########################################################

# Wat example build rules
examples_wam/%.wasm: examples_wam/printnum.wam examples_wam/%.wam
	wamp $^ > $(subst .wasm,.wat,$@)
	wasm-as $(subst .wasm,.wat,$@) -o $@

examples_wam/hello.wasm: examples_wam/console_curses.wam
examples_wam/colors.wasm: examples_wam/console_curses.wam
examples_wam/snake.wasm: examples_wam/console_curses.wam

examples_wat/%.wasm: examples_wat/%.wat
	wasm-as $< -o $@

# General C example build rules
examples_c/%.wasm: examples_c/%.c
	$(EMCC) -I examples_c/include -s USE_SDL=2 $< -o $@

.SECONDARY:
examples_c/%.wat: examples_c/%.wasm
	wasm-dis $< -o $@

examples_c/%: examples_c/%.c
	$(CC) $< -o $@ -lSDL2 -lSDL2_image -lGL -lglut

