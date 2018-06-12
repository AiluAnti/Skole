#	entry.S
#	This file contains the code for kernel_entry, as well as
#	scheduler_entry, which saves the context of the current process
#	before calling the scheduler. The file is pre-processed before it
#	is assembled, allowing (among other things) the use of macros
#	and C-style comments.

.data
	.align 4
.text
.code32
	.align 4

# Make symbols visible for ld
.globl	kernel_entry
.globl	scheduler_entry


# void kernel_entry(int fn)
#
# SYSCALL(i) in syslib.c calls this function
# fn = system call number (defined in kernel.h)
#
ke_scratch:
	.long	0
ke_arg:
	.long	0
kernel_entry:
	# Save eax
	movl	%eax, (ke_scratch)
	
	# Save argument
	movl	4(%esp), %eax
	movl	%eax, (ke_arg)

	pushal 							# Push all registers
	pushfl 							# Push all flags
	
	movl	(current_running), %eax # Save entry point of the struct
	movl	%esp, (%eax) 			# Move stack pointer to 2nd entry in pcb (kernel stack pointer)

	pushl 	(ke_arg)				# Push kernel_entry_helper argument 
	call 	kernel_entry_helper		# Call kernel_entry_helper
	popl 	(ke_arg)				# Pop argument

	movl 	(current_running), %eax # Move 1st element in struct to EAX again
	movl 	(%eax), %esp 			# Return where the stack pointer left off before calling kernel_entry_helper
	
	popfl 							# Pop all flags
	popal							# Pop all registers
	ret 							# Return

loop:
	jmp		loop 					# Simple loop

# This function gets called to enter the scheduler, saving registers before doing so.
scheduler_entry:
	pushal 							# Push all registers
	pushfl 							# Push flags
	
	subl	$108, %esp 				# 
	fnsave	(%esp)					#
	fwait							#

	movl	(current_running), %eax #Save entry point of the struct
	movl	%esp, 4(%eax) 			#Move stack pointer to 2nd entry in pcb (kernel stack pointer)

	call scheduler

	movl 	(current_running), %eax #Get entry point of struct
	movl 	4(%eax), %esp 			#Return where the stack pointer left off
	
	frstor	(%esp)					#
	fwait							#
	addl	$108, %esp				#

	popfl 							# Pop all flags
	popal 							# Pop all registers
	ret 							# Return
