
# for now specify gnu99, we shuld make sure that Minesweper complais whit iso c99
CC = gcc -std=gnu99

ifeq ($(DEBUG), yes)
CFLAGS = -Og -ggdb -DDEBUG
endif

ifeq ($(SMALL), yes)
CFLAGS ?= -Os -DNDEBUG
endif

CFLAGS ?= -O3 -DNDEBUG

CFLAGS += -pedantic -Wall -Wextra  -Wformat-security -Werror=format-security

PFLAGS =

ifeq ($(DEV), yes)
CFLAGS += -Werror -DDEBUG
endif

LTO_FLAGS =
LDFLAGS =
LIBS = -lrt -lSDL -lSDL_image

ifeq ($(LTO),yes)
LTO_FLAGS += -flto=1 -fuse-linker-plugin
endif

GCDA = 

ifeq ($(PROFILE_GEN),yes)
PFLAGS += -fprofile-generate
ifeq ($(DEV), yes)
else
PFLAGS += -Wno-error=coverage-mismatch
endif
else
ifeq ($(PROFILE_USE),yes)
PFLAGS += -fprofile-use
GCDA += *.gcda
ifeq ($(DEV), yes)
else
PFLAGS += -Wno-error=coverage-mismatch
endif
endif
endif

RM = rm -f
STRIP = strip --strip-unneeded

TARGET = Minesweeper

.PHONY: all clean
.SUFFIXES: .c .o

all: $(TARGET)

clean:
	$(RM) *.o *~ $(TARGET) $(GCDA)

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
main.o:       main.c        MS_util.h GW.h ComandStream.h OPT.h
GW.o:         GW.c          MS_util.h GW.h
minefield.o:  minefield.c   MS_util.h      ComandStream.h
OPT.o:        OPT.c         MS_util.h                     OPT.h
