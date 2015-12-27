CC=slc
INCLUDE=
SLCFLAGS=-b mta -O1
LDFLAGS=
TESTS= \
	lineBoundariesR.c \
	lineBoundariesRW.c \
	quickPageAlignedRW.c \
	notLineAlignedR.c \
	notLineAlignedRW.c \
	unalignedR.c \
	unalignedRW.c \
	smallPageBoundaries.c \
	mediumPageBoundaries.c \
	notPageAligned.c \
	syscall_test.c
HEADERS=defines.h main.h manager.h MgtMsg.h pagetable.h PTBuilder.h pt_index.h \
	IO.h syscall_gateway.h syscall.h SysCallMsg.h dispatcher.h mgt_io.h \
	test/memTester.h test/tests.h
SOURCES=main.c manager.c pagetable.c PTBuilder.c pt_index.c IO.c syscall.c \
	syscall_gateway.c dispatcher.c mgt_io.c \
	test/memTester.c test/tests.c $(addprefix test/, $(TESTS)) 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=boot.bin
CFLAGS=-O1 -Werror
#-Wa,-adhln

all: boot.bin

boot.bin: $(OBJECTS) $(HEADERS)
  ifndef MGSIM_INCLUDE
    $(error MGSIM_INCLUDE is required)
  endif
	$(CC) $(SLCFLAGS) $(LDFLAGS) $(INCLUDE) -I$(MGSIM_INCLUDE) $(OBJECTS) -o $@

%.o: %.c $(HEADERS)
  ifndef MGSIM_INCLUDE
    $(error MGSIM_INCLUDE is required)
  endif
	$(CC) -c $(SLCFLAGS) $(CFLAGS) $(INCLUDE) -I$(MGSIM_INCLUDE) $< -o $@

$(PREFIX)/boot.bin: boot.bin
  ifndef PREFIX
    $(error PREFIX is required)
  endif
	cp --verbose ./boot.bin $(PREFIX)/boot.bin

clean:
	rm *.o
	rm boot.bin

install: $(PREFIX)/boot.bin

.SUFFIXES:
.phony: install	clean
