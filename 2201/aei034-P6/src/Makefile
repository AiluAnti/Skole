# Makefile for the OS projects.

CC = gcc
LD = ld

KERNEL_LOCATION    = 0x8000 # physical & virtual address of kernel
PROCESS_LOCATION   = 0x1000000 # virtual address of processes

# Compiler flags
CCOPTS = -m32 -Wall -std=gnu99 -Wno-unused -g -c -O2 -fno-builtin -fno-stack-protector -fno-defer-pop -fno-unit-at-a-time -fno-toplevel-reorder \
         -mfpmath=387 -march=i386 -mno-mmx -mno-sse -mno-sse2 \
         -DPROCESS_START=$(PROCESS_LOCATION)
CC_SIMFLAGS = -m32 -Wall -std=gnu99 -g --no-builtin -DLINUX_SIM -DNDEBUG

# Linker flags
LDOPTS = -nostdlib -melf_i386

KERNEL = kernel.o

# Common objects used by both the kernel and user processes
COMMON = util.o print.o
# Processes to create
PROCESSES = shell.o process1.o process2.o process3.o \
				 process4.o

# USB subsystem
USB = usb/pci.o usb/uhci_pci.o usb/uhci.o usb/ehci_pci.o usb/usb_hub.o \
			usb/usb.o usb/usb_msd.o usb/scsi.o usb/usb_hid.o usb/usb_keyboard.o \
			usb/allocator.o

# Objects needed by the kernel
KERNELOBJ = $(COMMON) th1.o th2.o thread.o scheduler.o \
	interrupt.o mbox.o keyboard.o memory.o \
	sleep.o time.o dispatch.o $(USB) block.o fs.o

# Object files needed to build a process
PROCOBJ = $(COMMON) syslib.o

# Object files for the fake shell 
SIMOBJ = block_sim.o util_sim.o shell_sim.o thread_sim.o sim_fs.o print.o fs_error.o

ETAGS = etags
CTAGS = ctags

# Targets that aren't files (phony targets)
.PHONY: all demo progdisk depend clean distclean

### Makefile targets

all: image

# This target creates a shell that can be run on the development host.
# The shell can be used to simulate use of the filesystem during development
p6sh: $(SIMOBJ)
	$(CC) $(CC_SIMFLAGS) -o $@ $^
	dd if=/dev/zero of=./image_sim count=514

block_sim.o: block_sim.c
	$(CC) $(CC_SIMFLAGS) -c $<
util_sim.o: util_sim.c
	$(CC) $(CC_SIMFLAGS) -c $<
shell_sim.o: shell_sim.c
	$(CC) $(CC_SIMFLAGS) -c $<
thread_sim.o: thread_sim.c
	$(CC) $(CC_SIMFLAGS) -c $<
sim_fs.o: fs.c
	$(CC) $(CC_SIMFLAGS) -c -o $@ $<

# Targes for the kernel

kernel: entry.o $(KERNEL) $(KERNELOBJ)
	$(LD) $(LDOPTS) -Ttext $(KERNEL_LOCATION) -o $@ $^
	objcopy $@ $@ -G kernel_start

entry.o: entry.S
	$(CC) $(CCOPTS) -x assembler-with-cpp -c $< -o $@


# The processes
process1: proc_start.o process1.o $(PROCOBJ)
	$(LD) $(LDOPTS) -Ttext $(PROCESS_LOCATION) -o $@ $^

process2: proc_start.o process2.o $(PROCOBJ)
	$(LD) $(LDOPTS) -Ttext $(PROCESS_LOCATION) -o $@ $^

process3: proc_start.o process3.o $(PROCOBJ)
	$(LD) $(LDOPTS) -Ttext $(PROCESS_LOCATION) -o $@ $^

process4: proc_start.o process4.o $(PROCOBJ)
	$(LD) $(LDOPTS) -Ttext $(PROCESS_LOCATION) -o $@ $^

shell: proc_start.o shell.o $(PROCOBJ)
	$(LD) $(LDOPTS) -Ttext $(PROCESS_LOCATION) -o $@ $^


# other stuff

createimage: createimage.c
	$(CC) -o $@ $<  

asmsyms.h: asmdefs
	./$< > $@

asmdefs: asmdefs.c kernel.h
	$(CC) -o $@ $<

bootblock.o: bootblock.S kernel
	$(CC) $(CCOPTS) -DKERNEL_ADDR=0x$(shell nm kernel | grep "T kernel_start" | cut -c-8) -x assembler-with-cpp -o $@ $<

bootblock: bootblock.o kernel
	$(LD) $(LDOPTS) -Ttext 0x0 -o $@ $<

# Create an image to put on the USB stick
image: createimage bootblock kernel $(PROCESSES:.o=)
	./createimage --extended --vm --fs --kernel ./bootblock ./kernel \
	$(PROCESSES:.o=)

# Figure out dependencies, and store them in the hidden file .depend
depend: .depend
.depend: *.[cSh]
	$(CC) $(CCOPTS) -MM -MG *.[cS] > $@


# Clean up!
clean:
	-$(RM) *.o
	-$(RM) usb/*.o
	-$(RM) asmsyms.h
	-$(RM) $(PROCESSES:.o=) kernel image createimage bootblock asmdefs p6sh image_sim
	-$(RM) .depend

# No, really, clean up!
distclean: clean
	-$(RM) *~
	-$(RM) \#*
	-$(RM) *.bak
	-$(RM) TAGS tags
	-$(RM) bochsout.txt
	-$(RM) serial.out

# Make tags for emacs
TAGS:   *.[chsS]
	$(ETAGS) $^

# Make tags for vi
tags:   *.[chsS]
	$(CTAGS) $^

# How to compile a C file
%.o:%.c
	$(CC) $(CCOPTS) -o $@ $<

# How to assemble
%.o:%.s
	$(CC) $(CCOPTS) $<

%.o:%.S
	$(CC) $(CCOPTS) -x assembler-with-cpp $< 

# How to produce assembler input from a C file
%.s:%.c
	$(CC) $(CCOPTS) -S $<

# Include dependencies
# (the leading dash prevents warnings if the file doesn't exist)
-include .depend
