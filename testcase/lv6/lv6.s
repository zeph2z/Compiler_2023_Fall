	.text
	.globl main
main:
	addi sp, sp, -32
	li t0, 2
	sw t0, 0(sp)
	lw t0, 0(sp)
	sw t0, 4(sp)
	lw t0, 4(sp)
	bnez t0, then_0
	j else_0

then_0:
	lw t0, 0(sp)
	sw t0, 8(sp)
	lw t0, 8(sp)
	li t1, 1
	add t2, t0, t1
	sw t2, 12(sp)
	lw t0, 12(sp)
	sw t0, 0(sp)
	j end_0

else_0:
	li t0, 0
	sw t0, 0(sp)
	j end_0

end_0:
	lw t0, 0(sp)
	sw t0, 16(sp)
	lw a0, 16(sp)
	addi sp, sp, 32
	ret
