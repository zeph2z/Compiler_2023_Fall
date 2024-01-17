	.text
	.globl main
main:
	addi sp, sp, -64
	li t1, 0
	li t2, 8
	mul t1, t1, t2
	li t2, 0
	add t1, t1, t2
	add t1, t1, sp
	li t4, 16
	add t4, t4, sp
	sw t1, 0(t4)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	li t2, 16
	add t1, t1, t2
	add t1, t1, sp
	li t4, 20
	add t4, t4, sp
	sw t1, 0(t4)
	li t0, 1
	li t4, 20
	add t4, t4, sp
	lw t4, 0(t4)
	lw t4, 0(t4)
	sw t0, 0(t4)
	li t1, 1
	li t2, 4
	mul t1, t1, t2
	li t2, 16
	add t1, t1, t2
	add t1, t1, sp
	li t4, 24
	add t4, t4, sp
	sw t1, 0(t4)
	li t0, 2
	li t4, 24
	add t4, t4, sp
	lw t4, 0(t4)
	lw t4, 0(t4)
	sw t0, 0(t4)
	li t1, 1
	li t2, 8
	mul t1, t1, t2
	li t2, 0
	add t1, t1, t2
	add t1, t1, sp
	li t4, 28
	add t4, t4, sp
	sw t1, 0(t4)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	li t2, 28
	add t1, t1, t2
	add t1, t1, sp
	li t4, 32
	add t4, t4, sp
	sw t1, 0(t4)
	li t0, 3
	li t4, 32
	add t4, t4, sp
	lw t4, 0(t4)
	lw t4, 0(t4)
	sw t0, 0(t4)
	li t1, 1
	li t2, 4
	mul t1, t1, t2
	li t2, 28
	add t1, t1, t2
	add t1, t1, sp
	li t4, 36
	add t4, t4, sp
	sw t1, 0(t4)
	li t0, 4
	li t4, 36
	add t4, t4, sp
	lw t4, 0(t4)
	lw t4, 0(t4)
	sw t0, 0(t4)
	li t1, 0
	li t2, 8
	mul t1, t1, t2
	li t2, 0
	add t1, t1, t2
	add t1, t1, sp
	li t4, 40
	add t4, t4, sp
	sw t1, 0(t4)
	li t1, 1
	li t2, 4
	mul t1, t1, t2
	li t2, 40
	add t1, t1, t2
	add t1, t1, sp
	li t4, 44
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 44
	add t4, t4, sp
	lw t4, 0(t4)
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 48
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 48
	add t4, t4, sp
	lw a0, 0(t4)
	addi sp, sp, 64
	ret

