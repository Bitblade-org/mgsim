CC=slc
INCLUDE=-I/home/nijntje/mgsim_install/share/mgsim/
	#-I/opt/svp/sl-s3.4.90-5937-c3.8.47-e209-mta-p4876-ptl-preV5-hlsim-irfan-h4932-trunk/share/sl-core/mtalpha-sim/include/ \
	 
	
#-I/opt/svp/sl-s3.4.90-5937-c3.8.47-e209-mta-p4876-ptl-preV5-hlsim-irfan-h4932-trunk/share/sl-core/include/ -I/opt/svp/sl-s3.4.90-5937-c3.8.47-e209-mta-p4876-ptl-preV5-hlsim-irfan-h4932-trunk/share/sl-core/mtalpha-sim/include/
SLCFLAGS=-b mta
LDFLAGS=
HEADERS=defines.h main.h manager.h MgtMsg.h pagetable.h PTBuilder.h
SOURCES=main.c PTBuilder.c pagetable.c manager.c
#SOURCES=manager.c
#SOURCES=test.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=boot.bin
PREFIX=/home/nijntje/mgexec/slr/assets

all: boot.bin
    
boot.bin: $(OBJECTS) $(HEADERS)
	$(CC) $(SLCFLAGS) $(LDFLAGS) $(INCLUDE) $(OBJECTS) -o $@
	
%.o: %.c $(HEADERS)
	$(CC) -c $(SLCFLAGS) $(CFLAGS) $(INCLUDE) $< -o $@



$(PREFIX)/boot.bin: boot.bin
	cp ./boot.bin $(PREFIX)/boot.bin

clean:
	rm *.o
	rm boot.bin
	
install: $(PREFIX)/boot.bin

.phony: install	clean
