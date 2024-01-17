	.text
	.globl main
main:
	addi sp, sp, -80
	li t0, 0
	li t4, 0
	add t4, t4, sp
	sw t0, 0(t4)
	li t0, 21
	li t4, 4
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 4
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 8
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 8
	add t4, t4, sp
	lw t0, 0(t4)
	li t1, 1
	add t2, t0, t1
	li t4, 12
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 12
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 0
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 0
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 16
	add t4, t4, sp
	sw t0, 0(t4)
	li t0, 147
	li t4, 16
	add t4, t4, sp
	lw t1, 0(t4)
	sub t2, t0, t1
	li t4, 20
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 20
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 0
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 4
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 28
	add t4, t4, sp
	sw t0, 0(t4)
	li t0, 147
	li t4, 28
	add t4, t4, sp
	lw t1, 0(t4)
	sub t2, t0, t1
	li t4, 32
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 32
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 0
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 36
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 40
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 40
	add t4, t4, sp
	lw t0, 0(t4)
	li t1, 1
	mul t2, t0, t1
	li t4, 44
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 44
	add t4, t4, sp
	lw t0, 0(t4)
	li t1, 1
	mul t2, t0, t1
	li t4, 48
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 48
	add t4, t4, sp
	lw t0, 0(t4)
	li t1, 1
	mul t2, t0, t1
	li t4, 52
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 52
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 36
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 24
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 56
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 36
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 60
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 56
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 60
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 64
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 64
	add t4, t4, sp
	lw a0, 0(t4)
	addi sp, sp, 80
	ret

