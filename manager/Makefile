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
	notPageAligned.c
HEADERS=defines.h main.h manager.h MgtMsg.h pagetable.h PTBuilder.h pt_index.h test/memTester.h test/tests.h
SOURCES=main.c manager.c pagetable.c PTBuilder.c pt_index.c test/memTester.c test/tests.c $(addprefix test/, $(TESTS)) 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=boot.bin
CFLAGS=-O1 -Werror
#-Wa,-adhln

all: boot.bin
    
boot.bin: require-MGSIM_INCLUDE $(OBJECTS) $(HEADERS)
	$(CC) $(SLCFLAGS) $(LDFLAGS) $(INCLUDE) -I$(MGSIM_INCLUDE) $(OBJECTS) -o $@
	
%.o: require-MGSSIM_INCLUDE %.c $(HEADERS)
	echo $(MGSIM_INCLUDE)
	$(CC) -c $(SLCFLAGS) $(CFLAGS) $(INCLUDE) -I$(MGSIM_INCLUDE) $< -o $@

$(PREFIX)/boot.bin: require-PREFIX boot.bin 
	cp --verbose ./boot.bin $(PREFIX)/boot.bin

clean: 
	rm *.o
	rm boot.bin
	
install: $(PREFIX)/boot.bin require-PREFIX

require-PREFIX:
  ifndef PREFIX
	$(error PREFIX is required)
  endif

require-MGSIM_INCLUDE:
  ifndef MGSIM_INCLUDE
	$(error MGSIM_INCLUDE is required)
  endif

#require-%:
#  ifdef $*
#	$(error $* is required)
#  endif
	
.phony: install	clean require-PREFIX require-MGSIM_INCLUDE
