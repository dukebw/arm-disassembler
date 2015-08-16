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
    adc r0, r1, r2, LSL #16
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
	ldmfd r9, {r1, r3, r5, r7}^
	ldmdb r0!, {r2, r7, r15}^
	ldr r0, [r1]
	ldreq r2, [r3, r2, RRX]
	ldrhi r4, [r1, #0x100]
	ldrvc r5, [r6, r7, ROR #12]
	ldr r8, [r9, #-0xFC]
	ldrb r0, [r1, #0xFE]!
	ldrb r8, [r9, r10, LSR #31]
	strb r8, [r9, r10, LSL #31]
	streqbt r8, [r11]
	ldrmibt r0, [r1, r2, ROR #0]
	ldr pc, [r9, sp]!
	ldr pc, [r11, -sp]!
	ldr r0, [r1, -r2, RRX]
	ldr r3, [r4, -r5, LSL #15]!
	ldr r0, [r1], r2
	ldrbt r0, [r1], #0x100
	ldrne r2, [r3], -r4, RRX
	ldreq r4, [r5, r6, RRX]!
	ldrhib r7, [r8, r9, LSL #31]!
	ldrlob r10, [r11], r12, LSL #31
	ldreqd r0, [r2]
	strned r2, [r1, #0xFF]
	ldrvcsb r11, [r12, #-0xFC]!
	ldrplsh r13, [r14, -r12]
	strhih r0, [r1, r2]
	ldrmish r3, [r4], #-0xF0
	ldrd r4, [r6], #0xF
	streqh r7, [r8], -r9
	ldrvsd r0, [r3, -r4]!
	ldrex r0, [r1]
	ldrexeq r2, [r14]
	ldrt r0, [r1]
	ldrt r2, [r3], #0xF00
	mcr	p15, 0, r0, c1, c0, 0
	mcr p15, 0, r0, c7, c10
	mcr p15, 0, r0, c7, c5, 0
	mcr p7, 7, r0, c7, c5, 7
	mcrrne	p15, 4, r4, r4, c14
	mcrr2 p14, 7, r0, r14, c15
	mla r0, r1, r2, r3
	mlaeq r4, r5, r6, r7
	mlas r8, r9, r10, r11
	mlaeqs r12, r13, r14, r10
	moveq r0, r1, LSL #17
	movvcs r3, r4, LSL r6
	mov r7, #0x10400
	movvs r8, r10
	movhis r11, r13, RRX
    movs r2, r3, LSL r0
	mrc	p15, 0, r1, c0, c0, 1
	mrc	p14, 7, r15, c3, c7, 6
	mrrc p15, 7, r9, r13, c7
	mrrcne p14, 6, r14, r12, c6
	mrrc2 p13, 5, r11, r10, c5
	mrs	lr, spsr
	mrs lr, cpsr
	mrseq r9, cpsr
	msr	spsr, r13
	msrvc cpsr, r9
	msrne cpsr_cxsf, #0xFF00
	msr cpsr_f, r0
	msrmi spsr_cxsf, #0xFF
	msrpl spsr_xs, #0x100
	msrhi spsr_cf, r15
	mul r0, r1, r2
	muls r3, r4, r5
	mvneqs r0, r1, LSL r2
	mvn r3, r4, RRX
	mvns r3, #0x10000
	orrpls r7, r8, r9, ASR #31
	orrs r10, r11, #0x10800
	orr r15, r14, r13, RRX
	pkhbteq r0, r1, r2
	pkhbt r3, r4, r5, LSL #31
	pkhtbvc r6, r7, r8, ASR #1
	pkhtb r6, r7, r8, ASR #16
	pld	[r1, #0]
	pld	[r1, #92]
	pld [r2, r3, ROR #12]
	pld [r4, -r5, RRX]
	qaddmi r0, r1, r2
	qdadd r0, r1, r2
	qadd r12, r14, r13
	qadd16 r12, r14, r13
	qadd8 r12, r14, r13
	qaddsubx r12, r14, r13
	qdsub r0, r1, r2
	qsub r12, r14, r13
	qsub16 r12, r14, r13
	qsub8 r12, r14, r13
	qsubaddx r12, r14, r13
	reveq r0, r1
	rev r2, r3
	rev16 r4, r5
	revsh r6, r7
	rfeia r0!
	rfedb sp
	nop

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
