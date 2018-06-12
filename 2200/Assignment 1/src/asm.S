#include "asmdef.h"

DECLARE_GLOBAL(asm_function)


################################################################################
# name:          asm_function
# action:        
# in:            
#
# out:
# modifies:
# notes:
################################################################################
FUNC(asm_function):
    pushl   %ebp                        # Preserve old basepointer
    movl    %esp, %ebp                  # Create a new stack frame by setting
    pushl 	%ebx						# Push ebx to stack
    pushl 	%edi 						# Push edi to stack


    movl	8(%ebp), %eax				# Kopierer 1st argument(Valtype *a) til eax
	movl	12(%ebp), %ecx				# Kopierer 2nd argument(int start) til ecx
	movl	%ecx, %edx					# Kopierer end til edx

whileloop:
	movl	16(%ebp), %edi				# Kopierer 3rd argument(int end) til edi
	imull 	$2, %edx 					# Root * 2 // edx * 2
	incl 	%edx 						# Root++ // edx++
	cmpl 	%edi, %edx 					# Compare end og root * 2 + 1
	jge end;							# Jump til end hvis root * 2 + 1 er større eller lik root

firstif:
	incl 	%edx 						# Edx++ || Edx er nå brukt som child
	cmpl 	%edi, %edx 					# Compare end og child + 1
	jge decrement; 						# Hvis child + 1 er større eller lik end, jump til decrement

	decl 	%edx 						# Child-- for å få tak i child
	movl 	(%eax, %edx, 4), %ebx 		# Kopierer a[child] til ebx
	movl 	4(%eax, %edx, 4), %edi 		# Kopierer a[child+1] til edi
	cmpl 	%edi, %ebx 					# Compare a[child+1] og a[child]
	jge secondif; 						# Hvis a[child] er størst, hopp til secondif
	incl 	%edx						# Edx++
	movl	%edi, %ebx 					# Kopierer a[child+1] til ebx

secondif:
	movl 	(%eax, %ecx, 4), %edi  		# Kopierer a[root] til edi
	cmpl 	%ebx, %edi 					# Compare a[child] med a[root]
	jge end; 							# Jump til end hvis a[root] er større eller lik a[child] 
	movl 	%ebx, (%eax, %ecx, 4) 		# Kopierer a[child] til der a[root] ligger i array
	movl 	%edi, (%eax, %edx, 4) 		# Kopierer a[root] til der a[child] ligger i array
	movl 	%edx, %ecx 					# Kopierer child til root // root = child
	jmp whileloop;						# Loopen er ferdig, kjør gjennom på nytt fra toppen 

decrement:
	decl 	%edx 						# Edx--
	movl 	(%eax, %edx, 4), %ebx 		# Kopierer a[child] til ebx
	jmp 	secondif; 					# Jump til secondif

end:
	# Restore callee's stack frame						
	popl 	%edi
	popl 	%ebx
    popl    %ebp                        # Restore caller's stack frame
    ret                                 # Return to caller





