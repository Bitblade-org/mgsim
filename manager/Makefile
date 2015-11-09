CC=slc
INCLUDE=-I/home/nijntje/mgsim_install/share/mgsim/
	#-I/opt/svp/sl-s3.4.90-5937-c3.8.47-e209-mta-p4876-ptl-preV5-hlsim-irfan-h4932-trunk/share/sl-core/mtalpha-sim/include/ \
	 
	
#-I/opt/svp/sl-s3.4.90-5937-c3.8.47-e209-mta-p4876-ptl-preV5-hlsim-irfan-h4932-trunk/share/sl-core/include/ -I/opt/svp/sl-s3.4.90-5937-c3.8.47-e209-mta-p4876-ptl-preV5-hlsim-irfan-h4932-trunk/share/sl-core/mtalpha-sim/include/
SLCFLAGS=-b mta -O1 -g
LDFLAGS=
HEADERS=defines.h main.h manager.h memreader.h MgtMsg.h pagetable.h PTBuilder.h pt_index.h
SOURCES=main.c manager.c memreader.c pagetable.c PTBuilder.c pt_index.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=boot.bin
PREFIX=/home/nijntje/mgexec/slr/assets
CFLAGS=-O1 -g
#-Wa,-adhln

all: boot.bin
    
boot.bin: $(OBJECTS) $(HEADERS)
	$(CC) $(SLCFLAGS) $(LDFLAGS) $(INCLUDE) $(OBJECTS) -o $@
	
%.o: %.c $(HEADERS)
	$(CC) -c $(SLCFLAGS) $(CFLAGS) $(INCLUDE) $< -o $@



$(PREFIX)/boot.bin: boot.bin
	cp --verbose ./boot.bin $(PREFIX)/boot.bin

clean:
	rm *.o
	rm boot.bin
	
install: $(PREFIX)/boot.bin

.phony: install	clean
