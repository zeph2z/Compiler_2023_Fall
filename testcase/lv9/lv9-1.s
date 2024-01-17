	.text
	.globl init
init:
	addi sp, sp, -128
	li t4, 0
	add t4, t4, sp
	sw a0, 0(t4)
	li t4, 0
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 4
	add t4, t4, sp
	sw t0, 0(t4)
	li t0, 0
	li t4, 8
	add t4, t4, sp
	sw t0, 0(t4)
	j while_entry_0

while_entry_0:
	li t4, 8
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 12
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 12
	add t4, t4, sp
	lw t0, 0(t4)
	li t1, 10
	slt t2, t0, t1
	li t4, 16
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 16
	add t4, t4, sp
	lw t0, 0(t4)
	bnez t0, while_body_0
	j end_0

while_body_0:
	li t0, 0
	li t4, 20
	add t4, t4, sp
	sw t0, 0(t4)
	j while_entry_1

end_0:
	addi sp, sp, 128
	ret

while_entry_1:
	li t4, 20
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 24
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 24
	add t4, t4, sp
	lw t0, 0(t4)
	li t1, 10
	slt t2, t0, t1
	li t4, 28
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 28
	add t4, t4, sp
	lw t0, 0(t4)
	bnez t0, while_body_1
	j end_1

while_body_1:
	li t0, 0
	li t4, 32
	add t4, t4, sp
	sw t0, 0(t4)
	j while_entry_2

end_1:
	li t4, 8
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 36
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 36
	add t4, t4, sp
	lw t0, 0(t4)
	li t1, 1
	add t2, t0, t1
	li t4, 40
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 40
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 8
	add t4, t4, sp
	sw t0, 0(t4)
	j while_entry_0

while_entry_2:
	li t4, 32
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 44
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 44
	add t4, t4, sp
	lw t0, 0(t4)
	li t1, 10
	slt t2, t0, t1
	li t4, 48
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 48
	add t4, t4, sp
	lw t0, 0(t4)
	bnez t0, while_body_2
	j end_2

while_body_2:
	li t4, 8
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 52
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 52
	add t4, t4, sp
	lw t0, 0(t4)
	li t1, 100
	mul t2, t0, t1
	li t4, 56
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 20
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 60
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 60
	add t4, t4, sp
	lw t0, 0(t4)
	li t1, 10
	mul t2, t0, t1
	li t4, 64
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 56
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 64
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 68
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 32
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 72
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 68
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 72
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 76
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 8
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 80
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 80
	add t4, t4, sp
	lw t1, 0(t4)
	li t2, 400
	mul t1, t1, t2
	li t4, 4
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 84
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 20
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 88
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 88
	add t4, t4, sp
	lw t1, 0(t4)
	li t2, 40
	mul t1, t1, t2
	li t4, 84
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 92
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 32
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 96
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 96
	add t4, t4, sp
	lw t1, 0(t4)
	li t2, 4
	mul t1, t1, t2
	li t4, 92
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 100
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 76
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 100
	add t4, t4, sp
	lw t4, 0(t4)
	sw t0, 0(t4)
	li t4, 32
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 104
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 104
	add t4, t4, sp
	lw t0, 0(t4)
	li t1, 1
	add t2, t0, t1
	li t4, 108
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 108
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 32
	add t4, t4, sp
	sw t0, 0(t4)
	j while_entry_2

end_2:
	li t4, 20
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 112
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 112
	add t4, t4, sp
	lw t0, 0(t4)
	li t1, 1
	add t2, t0, t1
	li t4, 116
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 116
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 20
	add t4, t4, sp
	sw t0, 0(t4)
	j while_entry_1

	.text
	.globl f1
f1:
	addi sp, sp, -240
	li t4, 244
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 0
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 0
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 4
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 8
	add t4, t4, sp
	sw a5, 0(t4)
	li t4, 8
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 12
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 16
	add t4, t4, sp
	sw a4, 0(t4)
	li t4, 16
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 20
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 24
	add t4, t4, sp
	sw a3, 0(t4)
	li t4, 24
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 28
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 32
	add t4, t4, sp
	sw a6, 0(t4)
	li t4, 32
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 36
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 40
	add t4, t4, sp
	sw a2, 0(t4)
	li t4, 40
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 44
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 48
	add t4, t4, sp
	sw a1, 0(t4)
	li t4, 48
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 52
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 240
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 56
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 56
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 60
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 64
	add t4, t4, sp
	sw a7, 0(t4)
	li t4, 64
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 68
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 72
	add t4, t4, sp
	sw a0, 0(t4)
	li t4, 72
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 76
	add t4, t4, sp
	sw t0, 0(t4)
	li t1, 0
	li t2, 4
	mul t1, t1, t2
	li t4, 76
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 80
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 80
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 84
	add t4, t4, sp
	sw t0, 0(t4)
	li t1, 1
	li t2, 4
	mul t1, t1, t2
	li t4, 52
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 88
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 88
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 92
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 84
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 92
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 96
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 2
	li t2, 4
	mul t1, t1, t2
	li t4, 44
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 100
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 100
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 104
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 96
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 104
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 108
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 3
	li t2, 4
	mul t1, t1, t2
	li t4, 28
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 112
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 112
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 116
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 108
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 116
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 120
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 4
	li t2, 4
	mul t1, t1, t2
	li t4, 20
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 124
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 124
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 128
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 120
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 128
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 132
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 5
	li t2, 4
	mul t1, t1, t2
	li t4, 12
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 136
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 136
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 140
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 132
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 140
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 144
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 6
	li t2, 4
	mul t1, t1, t2
	li t4, 36
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 148
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 148
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 152
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 144
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 152
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 156
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 7
	li t2, 4
	mul t1, t1, t2
	li t4, 68
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 160
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 160
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 164
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 156
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 164
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 168
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 8
	li t2, 4
	mul t1, t1, t2
	li t4, 60
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 172
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 172
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 176
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 168
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 176
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 180
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 9
	li t2, 4
	mul t1, t1, t2
	li t4, 4
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 184
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 184
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 188
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 180
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 188
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 192
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 192
	add t4, t4, sp
	lw a0, 0(t4)
	addi sp, sp, 240
	ret

	.text
	.globl f2
f2:
	addi sp, sp, -240
	li t4, 244
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 0
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 0
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 4
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 8
	add t4, t4, sp
	sw a5, 0(t4)
	li t4, 8
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 12
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 16
	add t4, t4, sp
	sw a4, 0(t4)
	li t4, 16
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 20
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 24
	add t4, t4, sp
	sw a3, 0(t4)
	li t4, 24
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 28
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 32
	add t4, t4, sp
	sw a6, 0(t4)
	li t4, 32
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 36
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 40
	add t4, t4, sp
	sw a2, 0(t4)
	li t4, 44
	add t4, t4, sp
	sw a1, 0(t4)
	li t4, 44
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 48
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 240
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 52
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 56
	add t4, t4, sp
	sw a7, 0(t4)
	li t4, 56
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 60
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 64
	add t4, t4, sp
	sw a0, 0(t4)
	li t4, 64
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 68
	add t4, t4, sp
	sw t0, 0(t4)
	li t1, 0
	li t2, 40
	mul t1, t1, t2
	li t4, 68
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 72
	add t4, t4, sp
	sw t1, 0(t4)
	li t1, 9
	li t2, 4
	mul t1, t1, t2
	li t4, 72
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 76
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 76
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 80
	add t4, t4, sp
	sw t0, 0(t4)
	li t1, 1
	li t2, 4
	mul t1, t1, t2
	li t4, 48
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 84
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 84
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 88
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 80
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 88
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 92
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 40
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 96
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 92
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 96
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 100
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 3
	li t2, 4
	mul t1, t1, t2
	li t4, 28
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 104
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 104
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 108
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 100
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 108
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 112
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 4
	li t2, 4
	mul t1, t1, t2
	li t4, 20
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 116
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 116
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 120
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 112
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 120
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 124
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 5
	li t2, 400
	mul t1, t1, t2
	li t4, 12
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 128
	add t4, t4, sp
	sw t1, 0(t4)
	li t1, 5
	li t2, 40
	mul t1, t1, t2
	li t4, 128
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 132
	add t4, t4, sp
	sw t1, 0(t4)
	li t1, 5
	li t2, 4
	mul t1, t1, t2
	li t4, 132
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 136
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 136
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 140
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 124
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 140
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 144
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 6
	li t2, 4
	mul t1, t1, t2
	li t4, 36
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 148
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 148
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 152
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 144
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 152
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 156
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 7
	li t2, 4
	mul t1, t1, t2
	li t4, 60
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 160
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 160
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 164
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 156
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 164
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 168
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 52
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 172
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 168
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 172
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 176
	add t4, t4, sp
	sw t2, 0(t4)
	li t1, 9
	li t2, 40
	mul t1, t1, t2
	li t4, 4
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 180
	add t4, t4, sp
	sw t1, 0(t4)
	li t1, 8
	li t2, 4
	mul t1, t1, t2
	li t4, 180
	add t4, t4, sp
	lw t4, 0(t4)
	add t1, t1, t4
	li t4, 184
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 184
	add t4, t4, sp
	lw t4, 0(t4)
	lw t0, 0(t4)
	li t4, 188
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 176
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 188
	add t4, t4, sp
	lw t1, 0(t4)
	add t2, t0, t1
	li t4, 192
	add t4, t4, sp
	sw t2, 0(t4)
	li t4, 192
	add t4, t4, sp
	lw a0, 0(t4)
	addi sp, sp, 240
	ret

	.text
	.globl main
main:
	addi sp, sp, -2047
	addi sp, sp, -1969
	li t4, 4012
	add t4, t4, sp
	sw ra, 0(t4)
	li t0, 0
	li t4, 0
	add t4, t4, sp
	sw t0, 0(t4)
	li t1, 0
	li t2, 400
	mul t1, t1, t2
	li t2, 4
	add t1, t1, t2
	add t1, t1, sp
	li t4, 4004
	add t4, t4, sp
	sw t1, 0(t4)
	li t4, 4004
	add t4, t4, sp
	lw t4, 0(t4)
	lw a0, 0(t4)
	call init
	li t4, 0
	add t4, t4, sp
	lw t0, 0(t4)
	li t4, 4008
	add t4, t4, sp
	sw t0, 0(t4)
	li t4, 4008
	add t4, t4, sp
	lw a0, 0(t4)
	call putint
	li a0, 10
	call putch
	li a0, 0
	li t4, 4012
	add t4, t4, sp
	lw ra, 0(t4)
	addi sp, sp, 2047
	addi sp, sp, 1969
	ret

