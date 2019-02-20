
ifeq ($(CLANG), yes)
CC = clang -std=c99
CXX = clang++ -std=c++11
endif

ifeq ($(GCC), yes)
CC = gcc -std=c99
CXX = g++ -std=c++2a
endif

ifeq ($(CLANGPP), yes)
CC = clang++ -std=c++11
CXX = clang++ -std=c++11
endif

ifeq ($(GPP), yes)
CC = g++ -std=c++2a
CXX = g++ -std=c++2a
endif

ifeq ($(DEBUG), yes)
CFLAGS = -Og -ggdb -DDEBUG -v
endif

ifeq ($(SMALL), yes)
CFLAGS ?= -Os -DSMALL
endif

CFLAGS ?= -Ofast

ifeq ($(NO_TERM), yes)
CFLAGS += -DNO_TERM
endif

ifeq ($(GCC), yes)
CFLAGS += -Wlogical-op -faggressive-loop-optimizations
endif

ifeq ($(DEV), yes)
CFLAGS += -Werror -DDEBUG -pg
ifeq ($(CLANG), yes)
else
ifeq ($(CLANGPP), yes)
else
CFLAGS += -fstack-usage
endif
endif
endif

ifeq ($(NATIVE), yes)
CC += -march=native
endif

CFLAGS += -pedantic -Wall -Wextra -Wformat-security -Werror=format-security
CFLAGS += -Wstrict-aliasing -Wunreachable-code -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option 

CXXFLAGS = $(CFLAGS) -Wctor-dtor-privacy -Woverloaded-virtual -Wsign-promo

ifeq ($(GPP), yes)
CFLAGS += -Wlogical-op -Wctor-dtor-privacy -Wnoexcept -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Wno-error=missing-field-initializers -Wno-missing-field-initializers -Wno-error=pedantic -fpermissive -Wno-error # temporary mesure,  can't seem to turn of Werror exclusivly for -fpermissive
CXXFLAGS += -Wnoexcept -Wstrict-null-sentinel
else
CFLAGS += -Wold-style-definition -Wmissing-prototypes -Wstrict-prototypes -Wno-c99-extensions -Wdeclaration-after-statement
endif

ifeq ($(CLANGPP), yes)
CFLAGS += -Wctor-dtor-privacy -Woverloaded-virtual -Wsign-promo -Wno-error=deprecated -Wno-error=writable-strings -Wno-error=c++11-narrowing -Wno-error=address-of-temporary
endif

PFLAGS =
LTO_FLAGS =
LDFLAGS =

ifeq ($(LTO),yes)
LTO_FLAGS += -flto
endif

ifeq ($(PROFILE_GEN),yes)
PFLAGS += -fprofile-generate
ifeq ($(DEV), yes)
else
PFLAGS += -Wno-error=coverage-mismatch
endif
else
ifeq ($(PROFILE_USE),yes)
PFLAGS += -fprofile-use
ifeq ($(DEV), yes)
else
PFLAGS += -Wno-error=coverage-mismatch
endif
endif
endif

RM = rm -f
STRIP = strip --strip-unneeded

TARGET = Minesweeper
LIBS = -lrt -lSDL2 -lSDL2_image

.PHONY: all clean
.SUFFIXES: .c .o

all: $(TARGET)

analyze:
	clang-check -analyze *.c

clean:
	$(RM) *.o *.su *.gcda *~ $(TARGET)

strip:
	$(STRIP) $(TARGET)

$(TARGET): main.o GW.o minefield.o OPT.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS) $(PFLAGS) $(LTO_FLAGS)

ifeq ($(DEV),yes)
else
ifeq ($(DEBUG),yes)
else
	$(STRIP) $(TARGET)
endif
endif

%.o: %.c
	$(CC) $(CFLAGS) $(PFLAGS) $(LTO_FLAGS) -c -o $@ $<

# Dependencies
main.o:       Makefile main.c        MS_util.h GW.h             ComandStream.h OPT.h
GW.o:         Makefile GW.c          MS_util.h GW.h minefield.h
minefield.o:  Makefile minefield.c   MS_util.h                  ComandStream.h
OPT.o:        Makefile OPT.c         MS_util.h                                 OPT.h
