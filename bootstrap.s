.code 32
.globl _start
_start:
	mov sp,#0x100
	ldr r0,thumbstart_add
	moveqs sp,#0x10000
	movnes sp,#0x10400
	movcs sp,#0x10800
	movcc sp,#0x10C00
	movmi sp,#0x11000
	movpl sp,#0x11400
	movvs sp,#0x11800
	movvc sp,#0x11C00
	movhi sp,#0x12000
	movls sp,#0x12400
	movge sp,#0x12800
	movlt sp,#0x12C00
	movgt sp,#0x13000
	movle sp,#0x13400
	movs sp,#0x13800
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
