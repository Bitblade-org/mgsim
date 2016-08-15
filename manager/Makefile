CC=slc
AR=ar
ARFLAGS=-rvcs
INCLUDE=
SLCFLAGS=-b mta
#LDFLAGS=-L./lib/ -lpt -lsyscall
#LDFLAGS=-L/home/nijntje/mgsim/manager/lib/libpt.a -Llib/libsyscall.a
TESTS= \
	lineBoundariesR.c \
	lineBoundariesRW.c \
	smallPageAlignedRW.c \
	notLineAlignedR.c \
	notLineAlignedRW.c \
	unalignedR.c \
	unalignedRW.c \
	smallPageBoundaries.c \
	mediumPageBoundaries.c \
	notPageAligned.c \
	syscall_test.c \
	clock.c \
	clock.asm.c \
	b_4k_t_p.c
HEADERS=defines.h os/main.h manager/manager.h MgtMsg.h ptLib/pagetable.h ptLib/PTBuilder.h ptLib/pt_index.h \
	ioLib/IO.h os/syscall_gateway.h clientLib/syscall.h manager/dispatcher.h manager/mgt_io.h \
	tester/tester.h tester/test_init.h tester/test_lib.h tlbLib/tlbControl.h tlbLib/tlbRef.h
SOURCES_SYSCALL=clientLib/syscall.c
SOURCES_PT=ptLib/pt_index.c ptLib/pagetable.c ptLib/PTBuilder.c
SOURCES=os/main.c manager/manager.c ioLib/IO.c tlbLib/tlbControl.c \
	os/syscall_gateway.c manager/dispatcher.c manager/mgt_io.c \
	tester/tester.c tester/test_init.c tester/test_lib.c tester/tests/timedRead.asm.c $(addprefix tester/tests/, $(TESTS)) $(SOURCES_PT) $(SOURCES_SYSCALL)
OBJECTS_SYSCALL=$(SOURCES_SYSCALL:.c=.o)
OBJECTS_PT=$(SOURCES_PT:.c=.o)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=boot.bin
CFLAGS=-O1 -Werror
ASMC_CC=slc
ASMC_CFLAGS=-O0 -Werror -fno-asmopt $(SLCFLAGS)
#ASMC_CC=mtalpha-linux-gnu-gcc
#ASMC_CFLAGS=-nostdlib -mcpu=21264a -mno-explicit-relocs -O0 
#-Wa,-adhln

all: boot.bin

lib/libpt.a: $(OBJECTS_PT)
	mkdir -p lib
	$(AR) $(ARFLAGS) $@ $(OBJECTS_PT)
	
lib/libsyscall.a: $(OBJECTS_SYSCALL)
	mkdir -p lib
	$(AR) $(ARFLAGS) $@ $(OBJECTS_SYSCALL)

boot.bin: lib/libpt.a lib/libsyscall.a $(OBJECTS) $(HEADERS)
  ifndef MGSIM_INCLUDE
    $(error MGSIM_INCLUDE is required)
  endif
	$(CC) $(SLCFLAGS) $(LDFLAGS) $(INCLUDE) -I$(MGSIM_INCLUDE) $(OBJECTS) -o $@

#tester/tests/clock.o: tester/tests/clock.c $(HEADERS)
#  ifndef MGSIM_INCLUDE
#    $(error MGSIM_INCLUDE is required)
#  endif
#	$(CC) -c $(SLCFLAGS) $(VCFLAGS) -I$(MGSIM_INCLUDE) $< -o $@

%.asm.o: %.asm.c $(HEADERS)
  ifndef MGSIM_INCLUDE
    $(error MGSIM_INCLUDE is required)
  endif
	$(ASMC_CC) -c $(ASMC_CFLAGS) -I$(MGSIM_INCLUDE) $< -o $@

%.o: %.c $(HEADERS)
  ifndef MGSIM_INCLUDE
    $(error MGSIM_INCLUDE is required)
  endif
	$(CC) -c $(SLCFLAGS) $(CFLAGS) -I$(MGSIM_INCLUDE) $< -o $@

$(PREFIX)/boot.bin: boot.bin
  ifndef PREFIX
    $(error PREFIX is required)
  endif
	cp --verbose ./boot.bin $(PREFIX)/boot.bin

clean:
	find . -name "*.o" -type f -delete
	rm boot.bin

install: $(PREFIX)/boot.bin

.SUFFIXES:
.phony: install	clean
