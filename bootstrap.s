.code 32
.globl _start
_start:
	mov sp, #0x100
	ldr r0, thumbstart_add
	ldrhi r2, thumbstart_add
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
    addeq r0, r1, r7, ASR #31
    adds r0, r1, r2, ASR r3
    adds r8, r9, r10, ROR #7
    add r4, r5, r6, ROR r7
    add r0, r1, r2, RRX
    adds r0, sp, #0x100
    and r0, r1, r2, LSR #0
    and r0, r1, r2, LSL #31
    ands r8, r7, r6, ROR #15
    ands r8, r8, r8, ROR r8
    and r1, r0, r15, ASR #7
    and r2, r3, r5, ASR r7
    and r1, r1, r1, RRX
    mrs r0, CPSR
    mrs r1, SPSR
    b thumbstart
    b _start
    bic r0, r1, r1
    biceqs r1, r2, r3, ROR r4
    bics r5, r6, r7, LSL #17
    bic r8, r9, r10, LSR #1
    bic r11, r12, r13, RRX
    bicnes r14, r14, r14, ASR r14
    bkpt #0x1000
    bkpt #0xFFFF
    bkpt #0x1
    bkpt #0x0
    bkpt #0x8
    bkpt #0x135C
    bl hang
    blne thumbstart_add
    blo _start
    blx hang
    blx _start
    blx thumbstart
    blx thumbstart_add
    blx address_one
    blx address_fun
    ldr r1, thumbstart_add
    bxjcs r1
    ldr r0, thumbstart
    blxeq r0
	bx r0
    cdpeq p15, 0, c0, c1, c2, 0
    cdp2 p14, 0xF, c7, c8, c13, 0x7
    clz r0, r1
    clzmi r10, r11
    cmn r2, #96
    cmn r0, r1, RRX
    cmneq r2, r3, LSL r4
    cmnmi r5, r6, LSR #31
    cmnvc r7, r8
    cmnvs r9, r10, ASR r11
    cmn r12, r13, ROR r14
    cmn r15, r0, ROR #15
	cmp r2, #0xFF0
	cmp r3, r4
	cmp r5, r6, LSL #31
	cmpeq r7, r8, ROR #15
	cmpmi r9, r10, RRX
	cpsid i
	cpsid f
	cpsie a, #31
	cpsid if
	cps #16
	cpyeq r0, r1
	cpypl r15, r14
	cpyvc r2, r3
	eoreqs r0, r1, r2, LSL r3
	eor r4, r5, r6
	eorvc r7, r8, r9, RRX
	eor r10, r12, r11, ASR #31
	ldc p15, c10, [pc, #0x100]
	ldc p14, c7, [r0, #-0x100]
	ldcne p13, c8, [r1, #0xF0]!
	ldcvc p0, c0, [r14], #0x100
	ldc p15, c15, [r15], {#0xFF}
	ldceql p14, c14, [r14], #0x0
	ldc2l p14, c14, [r14]
	ldmfd sp!, {r1 - r12}
	ldmia r0!, {r3 - r10}
	ldmia r0!, {r1 - r12}
	ldmia lr!, {r1 - r12}
	ldmvcib r7!, {r1 - r3}
    movs r2, r3, LSL r0

thumbstart_add:
	.word thumbstart
address_one:
    .word 0x0
address_fun:

;@ ----- arm above, thumb below
.thumb

.thumb_func
thumbstart:
	bl notmain
hang:
	b hang
