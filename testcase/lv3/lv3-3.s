	.text
	.globl main
main:
	addi sp, sp, -256
	li t0, 0
	sub t2, t0, t1
	snez t3, t2
	sw t3, 0(sp)
	lw a0, 0(sp)
	addi sp, sp, 256
	ret
