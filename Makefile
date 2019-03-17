
ifeq ($(CLANG), yes)
CC = clang -std=c99
CXX = clang++ -std=c++11
CFLAGS += -Weverything -Wno-error=disabled-macro-expansion -Wno-error=switch-enum -Wno-error=padded
endif

ifeq ($(GCC), yes)
CC = gcc -std=c99
CXX = g++ -std=c++2a
CFLAGS += -Wlogical-op -faggressive-loop-optimizations
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
CC += -v
endif

ifeq ($(SMALL), yes)
CFLAGS ?= -Os -DSMALL
endif

CFLAGS ?= -Ofast

ifeq ($(NO_TERM), yes)
CFLAGS += -DNO_TERM
endif

ifeq ($(DEV), yes)
CFLAGS += -Werror -DDEBUG -pg -ftrapv -fsanitize=address
LIBS += -lasan
endif

ifeq ($(NATIVE), yes)
CC += -march=native
endif

CFLAGS += -Wall -Wextra -Wformat-security -Werror=format-security -Wno-c99-extensions
CFLAGS += -Wstrict-aliasing -Wunreachable-code -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wswitch-default -Wundef -fdiagnostics-show-option

CXXFLAGS = $(CFLAGS) -Wctor-dtor-privacy -Woverloaded-virtual -Wsign-promo

ifeq ($(GPP), yes)
CFLAGS += -Wlogical-op -Wctor-dtor-privacy -Wnoexcept -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Wno-error=missing-field-initializers -Wno-missing-field-initializers -Wno-error=pedantic -Wno-error
CXXFLAGS += -Wnoexcept -Wstrict-null-sentinel
else
CFLAGS += -pedantic -Wold-style-definition -Wmissing-prototypes -Wstrict-prototypes -Wdeclaration-after-statement
endif

ifeq ($(CLANGPP), yes)
CFLAGS += -Wno-old-style-cast -Wno-c++98-compat-pedantic -Wno-c++98-compat -Wno-error=deprecated -Wno-error=writable-strings -Wno-error=c++11-narrowing -Wno-error=address-of-temporary -Wno-error=missing-field-initializers
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
LIBS += -lrt -lSDL2 -lSDL2_image

.PHONY: all clean
.SUFFIXES: .c .o

all: $(TARGET)

analyze:
	clang-check -analyze *.c

clean:
	$(RM) *.o *.su *.plist *.gcda *~ $(TARGET)

strip:
	$(STRIP) $(TARGET)

#$(TARGET): main.o epoxy.o minefield.o OPT.o
$(TARGET): main.o userinterface.o minefield.o OPT.o
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
main.o:          main.c           MS_util.h userinterface.h minefield.h                OPT.h
userinterface.o: userinterface.c  MS_util.h userinterface.h minefield.h
epoxy.o:         epoxy.c          MS_util.h userinterface.h minefield.h
minefield.o:     minefield.c      MS_util.h                 minefield.h ComandStream.h
OPT.o:           OPT.c            MS_util.h                                            OPT.h
