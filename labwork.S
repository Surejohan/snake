  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  
.global delay
.global time2string  
.global mytime
.global timstr
.global main
.global tick
.global tiend
.global enable_interrupt

  # Please add your own code below this line 
 hexasc:
	andi $t0, $a0, 0xf #takes the four least sig bits from argument
	
	addi $v0, $t0, 0x30  #adds bits to constant so that 0 equals ascii 0 to 9
	
	slti $t1, $v0, 0x3a
	beq $t1, $0, Else # if false goto else
	nop
	j Endif  # goto endif
	nop
 Else:
	addi $v0, $v0, 0x07  #adds bits so that 10 will output A
 Endif:
	jr $ra
	nop
	#v0 return value with 7 least sig bits all other = zero

time2string:
	#PUSH
	PUSH $v0 #nr0
	PUSH $ra #nr1
	PUSH $s0
	PUSH $s1
	move $s0, $a0 #$s0 will hold $a0
	move $s1, $a1 #$s1 will hold $a1
	
	#saves a0 and then moves a1 to a0
	move $a0, $s1
	#Stores ascii chars to rep sec
	jal	hexasc		# call hexasc
	nop			# delay slot filler (just in case)	
	sb $v0, 4($s0)          #Read kinda backwards so this is least sig sec
	srl $s1, $s1, 4
	
	#saves a0 and then moves a1 to a0
	move $a0, $s1
	jal	hexasc		# call hexasc
	nop			# delay slot filler (just in case)	
	sb $v0, 3($s0)          #not least sec
	srl $s1, $s1, 4
	
	#Stores the ascii char for :
	addi $t5, $zero, 0x3A
	sb $t5, 2($s0)
	
	#saves a0 and then moves a1 to a0
	move $a0, $s1
	#Stores ascii chars to rep min
	jal	hexasc		# call hexasc
	nop			# delay slot filler (just in case)	
	sb $v0, 1($s0)          #stores min
	srl $s1, $s1, 4
	
	#saves a0 and then moves a1 to a0
	move $a0, $s1
	jal	hexasc		# call hexasc
	nop			# delay slot filler (just in case)	
	sb $v0, 0($s0)          #stores big min
	srl $s1, $s1, 4
	
	#stores null byte
	move $t5, $zero
	sb $t5, 5($s0)
	
	
	#POP
	POP $s1
	POP $s0
	POP $ra #nr1
	POP $v0 #nr0
	
	jr $ra
	nop			
delay: #When arg0 is equal to 1000 the delay is 1000ms or 1sec

	#POP($s1)
	move $t2, $a0  #sets t2 to argument (or parameter)
	#addi $t2, $zero, 1 #sets it to 1000 so the delay should be about 1 sec

	While:
		slt $t1, $zero, $t2 #1 if argument is bigger than zero and it runs the code below 
		beq $t1, $zero, Done #doesn't run while loop if t1 is zero
		nop
		
		addi $t2, $t2, -1 #subtracts one from while statment t1

		#starts "for-loop"
		#$t3 = i
		move $t3, $zero #sets "i" to zero
		addi $t4, $zero, 11538 #uses t4 as 4711

		For:
			slt $t5, $t3, $t4 #if(i < 4711) 
			beq $t5, $zero, DoneFor  #ends/ skips for loop if i >= 4711
			nop
			
			addi $t3, $t3, 1 # i = i + 1 
			j For
			nop
		DoneFor:	
		j While
		
	Done: #end of while

	#PUSH($s1)
 	jr $ra
 	nop #end of dela y 
 	
 enable_interrupt:
 	ei 

  	jr $ra
  	nop
  
	
