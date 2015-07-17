.code 32
.globl _start
_start:
	mov sp, #0x100
	ldr r0, thumbstart_add
	moveqs sp, #0x10000
	movnes sp, #0x10400
	movcs sp, #0x10800
	movcc sp, #0x10C00
	movmi sp, #0x11000
	movpl sp, #0x11400
	movvs sp, #0x11800
	movvc sp, #0x11C00
	movhi sp, #0x12000
	movls sp, #0x12400
	movge sp, #0x12800
	movlt sp, #0x12C00
	movgt sp, #0x13000
	movle sp, #0x13400
	movs sp, #0x13800
    adc sp, sp, sp
    adc r0, r1, r2, LSL #0x10
    adcs r0, r1, r2, LSL r3
    add r9, r8, r7, LSR #31
    add r9, r8, r7, LSR r6
    adc r0, r9, #0x10000
    adcs r0, r1, #0x10400
    add r0, r7, lr
    add r0, r1, #0x140
    adds r0, sp, #0x100
    mrs r0, CPSR
    mrs r1, SPSR
    b thumbstart
    b _start
    bl hang
    bl thumbstart_add
	bx r0

thumbstart_add:
	.word thumbstart

;@ ----- arm above, thumb below
.thumb

.thumb_func
thumbstart:
	bl notmain
hang:
	b hang
