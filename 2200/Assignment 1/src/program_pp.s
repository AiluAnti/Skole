# 1 "benchmark.S"
# 1 "/users/aei034/Desktop/2200-Assignment1/src//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "benchmark.S"
# 1 "asmdef.h" 1
# 2 "benchmark.S" 2

.globl asm_function; .type asm_function, @function

################################################################################
# name: benchmark
# action:
# in:

# out:
# modifies:
# notes:
################################################################################
asm_function:
    pushl %ebp # Preserve old basepointer
    movl %esp, %ebp # Create a new stack frame by setting
    pushl %esi

    movl 8(%ebp), %eax # Kopierer 1st argument(array *a) til eax
 movl 12(%ebp), %ecx # Kopierer 2nd argument(num iter) til ecx
 movl 16(%ebp), %ebx
 movl $0, %esi

forloop:
 movl (%eax), %edx
 movl (%eax), %edx
 movl (%ebx), %edx
 movl (%ebx), %edx
 decl %ecx
 cmpl %esi, %ecx
 jne forloop;

end:
 # Restore callee's stack frame						
    popl %esi
    popl %ebp # Restore caller's stack frame
    ret
