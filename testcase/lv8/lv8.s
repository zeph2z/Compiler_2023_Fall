	.data
	.globl a
a:
	.word 10

	.text
	.globl inc
inc:
	addi sp, sp, -16
	lw t0, a
	sw t0, 0(sp)
	lw t0, 0(sp)
	li t1, 1
	add t2, t0, t1
	sw t2, 4(sp)
	lw t0, 4(sp)
	la t1, a
	sw t0, 0(t1)
	lw t0, a
	sw t0, 8(sp)
	lw a0, 8(sp)
	addi sp, sp, 16
	ret

	.text
	.globl print_a
print_a:
	addi sp, sp, -16
	sw ra, 12(sp)
	lw t0, a
	sw t0, 0(sp)
	lw a0, 0(sp)
	call putint
	li a0, 10
	call putch
	lw ra, 12(sp)
	addi sp, sp, 16
	ret

	.text
	.globl main
main:
	addi sp, sp, -48
	sw ra, 44(sp)
	li t0, 0
	sw t0, 0(sp)
	j while_entry_0

while_entry_0:
	lw t0, 0(sp)
	sw t0, 4(sp)
	lw t0, 4(sp)
	li t1, 10
	slt t2, t0, t1
	sw t2, 8(sp)
	lw t0, 8(sp)
	bnez t0, while_body_0
	j end_0

while_body_0:
	call inc
	sw a0, 12(sp)
	li t0, 1
	sw t0, 16(sp)
	lw t0, 16(sp)
	sw t0, 20(sp)
	lw t0, 20(sp)
	li t1, 2
	add t2, t0, t1
	sw t2, 24(sp)
	lw t0, 24(sp)
	sw t0, 16(sp)
	lw t0, 16(sp)
	sw t0, 28(sp)
	lw a0, 28(sp)
	call putint
	li a0, 10
	call putch
	call print_a
	lw t0, 0(sp)
	sw t0, 32(sp)
	lw t0, 32(sp)
	li t1, 1
	add t2, t0, t1
	sw t2, 36(sp)
	lw t0, 36(sp)
	sw t0, 0(sp)
	j while_entry_0

end_0:
	li a0, 0
	lw ra, 44(sp)
	addi sp, sp, 48
	ret

