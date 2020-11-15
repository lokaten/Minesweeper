
ifeq ($(DEBUG), yes)
CFLAGS = -Og -ggdb -DDEBUG -v
CC += -v
endif

ifeq ($(SMALL), yes)
CFLAGS ?= -Os -DSMALL
endif

CFLAGS ?= -Ofast


ifeq ($(CLANG), yes)
CC = clang -std=gnu99
CXX = clang++ -std=gnu++11
CFLAGS += -Weverything -Wno-disabled-macro-expansion -Wno-error=switch-enum -Wno-error=padded -Wno-error=bad-function-cast
endif

ifeq ($(GCC), yes)
CC = gcc -std=gnu99
CXX = g++ -std=gnu++2a
CFLAGS += -Wlogical-op -faggressive-loop-optimizations
CFLAGS += -pedantic -Wold-style-definition -Wmissing-prototypes -Wstrict-prototypes -Wdeclaration-after-statement
CFLAGS += -Wstrict-aliasing -Wunreachable-code -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wswitch-default -Wundef -fdiagnostics-show-option
endif

ifeq ($(CLANGPP), yes)
CC = clang++ -std=gnu++11
CXX = clang++ -std=gnu++11
CFLAGS += -Wevrything -Wno-address-of-temporary -Wno-missing-field-initializers
CFLAGS += -Wno-old-style-cast -Wno-c++98-compat-pedantic -Wno-c++98-compat -Wno-error=deprecated -Wno-error=writable-strings -Wno-error=c++11-narrowing -Wno-error=address-of-temporary
endif

ifeq ($(GPP), yes)
CC = g++ -std=gnu++2a
CXX = g++ -std=gnu++2a
CFLAGS += -Wlogical-op -Wctor-dtor-privacy -Wnoexcept -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Wno-error=missing-field-initializers -Wno-missing-field-initializers -Wno-error=pedantic -Wno-error
CXXFLAGS += -Wnoexcept -Wstrict-null-sentinel
CFLAGS += -Wstrict-aliasing -Wunreachable-code -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wswitch-default -Wundef -fdiagnostics-show-option
endif

ifeq ($(NO_TERM), yes)
CFLAGS += -DNO_TERM
endif

ifeq ($(DEV), yes)
CFLAGS += -Werror -DDEBUG -pg -ftrapv -fsanitize=address -ftime-report
LIBS += -lasan
endif

ifeq ($(NATIVE), yes)
CC += -march=native
endif

CFLAGS += -fstrict-aliasing -Wall -Wextra -Wformat-security -Werror=format-security

CXXFLAGS = $(CFLAGS) -Wctor-dtor-privacy -Woverloaded-virtual -Wsign-promo

PFLAGS =
LTO_FLAGS =
LDFLAGS =

ifeq ($(LTO),yes)
LTO_FLAGS += -flto -flto=jobserv
endif

ifeq ($(PROFILE_GEN),yes)
PFLAGS += -fprofile-generate -Wno-error=coverage-mismatch
else
ifeq ($(PROFILE_USE),yes)
PFLAGS += -fprofile-use -Wno-error=coverage-mismatch
endif
endif

RM = rm -f
STRIP = strip --strip-unneeded

TARGET = Minesweeper

ifeq ($(EPOXY),yes)
LIBS += -lwayland-egl -lwayland-client -lepoxy
else
LIBS += -lSDL2 -lSDL2_image
endif

LIBS += -pthread

#LIBS += -lrt

.PHONY: all clean
.SUFFIXES: .c .o

all: $(TARGET)

analyze:
	clang-check -analyze *.c *.h

clean:
	$(RM) *.o *.su *.plist *.gcda *~ $(TARGET)

strip:
	$(STRIP) $(TARGET)

ifeq ($(EPOXY),yes)
$(TARGET): main.o FreeNode.o epoxy.o minefield.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS) $(PFLAGS) $(LTO_FLAGS)
else
$(TARGET): main.o FreeNode.o userinterface.o minefield.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS) $(PFLAGS) $(LTO_FLAGS)
endif

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
ifeq ($(STUB_FREENODE), yes)
Freenode.o:      FN_stub.c        MS_util.h
else
FreeNode.o:      FreeNode.c       MS_util.h  
endif
userinterface.o: userinterface.c  MS_util.h userinterface.h minefield.h ComandStream.h
epoxy.o:         epoxy.c          MS_util.h userinterface.h minefield.h 
minefield.o:     minefield.c      MS_util.h userinterface.h minefield.h ComandStream.h
