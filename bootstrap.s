.code 32
.globl _start
_start:
    mov sp,#0x100
    ldr r0,thumbstart_add
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
