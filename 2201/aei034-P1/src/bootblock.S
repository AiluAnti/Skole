# bootblock.s

# .equ symbol, expression
# These directive set the value of the symbol to the expression
  .equ    BOOT_SEGMENT,      0x07c0
  .equ    DISPLAY_SEGMENT,   0xb800
  .equ    KERNEL_SEGMENT,    0x0000
  .equ    KERNEL_OFFSET,     0x1000

# You need to decide where to put the stack
  .equ    STACK_SEGMENT,     0x07e0
  .equ    STACK_POINTER,     0x9fc0

.text                   # Code segment
.globl    _start        # The entry point must be global
.code16                 # Real mode
.org 0x0

#
# The first instruction to execute in a program is called the entry
# point. The linker expects to find the entry point in the "symbol" _start
# (with underscore).
#

_start:
  jmp beyondReservedSpace

kernelSize:
  .word 0               # bootimage will write size of kernel, in sectors


beyondReservedSpace:
  movw $DISPLAY_SEGMENT, %bx
  movw %bx, %es
  
  # Clear screen
  movw $0x0a00, %ax     # Fill with black background / green foreground
  movw $2000, %cx       # Number of characters (80x25 screen in text mode = 2000)
  xorw %di, %di         # DI = 0
  rep stosw
  
  movb $0x4b, %es:(0x0) # Write 'K' in the upper left corner of the screen

  movw $BOOT_SEGMENT, %ax # copy BOOT_SEGMENT base to ax
  movw %ax, %ds # Set bootblock data segment to 0x7c0

  movw $STACK_SEGMENT, %ax # Copy STACK_SEGMENT base to ax
  movw %ax, %ss   #  Copy STACK_SEGMENT base to ss (stack segment)
  movw $STACK_POINTER, %ax # Copy STACK_POINTER to ax
  movw %ax, %sp  #Copy STACK_POINTER to sp (stack pointer)

  movw $KERNEL_SEGMENT, %ax # Move kernel segment to ax
  movw %ax, %es # Move kernel segment to es

  movw $KERNEL_OFFSET, %ax # Move kernel offset to ax
  movw %ax, %bx # Move kernel offset to bx


  movb $0x2, %ah # Set AH value to 2
  movb (kernelSize), %al # Sectors to read gotten from kernelSize  
  movb $0x0, %ch # Cylinder value 0
  movb $0x2, %cl # Sector value 2
  movb $0x0, %dh # Head value 0

  int $0x13 #interrupt call

  movw $KERNEL_SEGMENT, %ax # Copy kernel segment to ax
  movw %ax, %ds # Copy kernel segment to data segment

  ljmp $KERNEL_SEGMENT, $KERNEL_OFFSET

forever:
  jmp forever # Loop forever

