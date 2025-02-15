// See:
// https://github.com/syntacore/sc-bl/blob/master/src/startup.S
// https://codebrowser.dev/glibc/glibc/sysdeps/riscv/start.S.html

// https://github.com/riscv-non-isa/riscv-asm-manual/blob/master/riscv-asm.md

/**
 * RISC-V bootup test
 * Author: Daniele Lacamera <root@danielinux.net>
 *
 * MIT License
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

.macro trap_entry
	addi	sp,sp,-256

//	sd x0,0(sp)		/* zero register */
	sd x1,8(sp)		/* ABI link register ra */
//	sd x2,16(sp)	/* ABI stack pointer sp */
	sd x3,24(sp)
	sd x4,32(sp)
	sd x5,40(sp)
	sd x6,48(sp)
	sd x7,56(sp)
	sd x8,64(sp)
	sd x9,72(sp)
	sd x10,80(sp)
	sd x11,88(sp)
	sd x12,96(sp)
	sd x13,104(sp)
	sd x14,112(sp)
	sd x15,120(sp)
	sd x16,128(sp)
	sd x17,136(sp)
	sd x18,144(sp)
	sd x19,152(sp)
	sd x20,160(sp)
	sd x21,168(sp)
	sd x22,176(sp)
	sd x23,184(sp)
	sd x24,192(sp)
	sd x25,200(sp)
	sd x26,208(sp)
	sd x27,216(sp)
	sd x28,224(sp)
	sd x29,232(sp)
	sd x30,240(sp)
	sd x31,248(sp)
.endm


.macro trap_exit
//	ld x0,0(sp)		/* zero register */
	ld x1,8(sp)		/* ABI link register ra */
//	ld x2,16(sp)	/* ABI stack pointer sp */
	ld x3,24(sp)
	ld x4,32(sp)
	ld x5,40(sp)
	ld x6,48(sp)
	ld x7,56(sp)
	ld x8,64(sp)
	ld x9,72(sp)
	ld x10,80(sp)
	ld x11,88(sp)
	ld x12,96(sp)
	ld x13,104(sp)
	ld x14,112(sp)
	ld x15,120(sp)
	ld x16,128(sp)
	ld x17,136(sp)
	ld x18,144(sp)
	ld x19,152(sp)
	ld x20,160(sp)
	ld x21,168(sp)
	ld x22,176(sp)
	ld x23,184(sp)
	ld x24,192(sp)
	ld x25,200(sp)
	ld x26,208(sp)
	ld x27,216(sp)
	ld x28,224(sp)
	ld x29,232(sp)
	ld x30,240(sp)
	ld x31,248(sp)
	addi	sp,sp,256
.endm

/* set all integer registers to zero */
.macro fill0_i64
	mv	x3, zero	/* x0 */
	mv  x4, x3
	mv  x5, x3
	mv  x6, x3
	mv  x7, x3
	mv  x8, x3
	mv  x9, x3
	mv x10, x3
	mv x11, x3
	mv x12, x3
	mv x13, x3
	mv x14, x3
	mv x15, x3
	mv x16, x3
	mv x17, x3
	mv x18, x3
	mv x19, x3
	mv x20, x3
	mv x21, x3
	mv x22, x3
	mv x23, x3
	mv x24, x3
	mv x25, x3
	mv x26, x3
	mv x27, x3
	mv x28, x3
	mv x29, x3
	mv x30, x3
	mv x31, x3
.endm

/* Set all double precision floating registers to zero */
.macro fill0_fp64
	fcvt.d.w f0, x0
	fcvt.d.w f1, x0
	fcvt.d.w f2, x0
	fcvt.d.w f3, x0
	fcvt.d.w f4, x0
	fcvt.d.w f5, x0
	fcvt.d.w f6, x0
	fcvt.d.w f7, x0
	fcvt.d.w f8, x0
	fcvt.d.w f9, x0
	fcvt.d.w f10, x0
	fcvt.d.w f11, x0
	fcvt.d.w f12, x0
	fcvt.d.w f13, x0
	fcvt.d.w f14, x0
	fcvt.d.w f15, x0
	fcvt.d.w f16, x0
	fcvt.d.w f17, x0
	fcvt.d.w f18, x0
	fcvt.d.w f19, x0
	fcvt.d.w f20, x0
	fcvt.d.w f21, x0
	fcvt.d.w f22, x0
	fcvt.d.w f23, x0
	fcvt.d.w f24, x0
	fcvt.d.w f25, x0
	fcvt.d.w f26, x0
	fcvt.d.w f27, x0
	fcvt.d.w f28, x0
	fcvt.d.w f29, x0
	fcvt.d.w f30, x0
	fcvt.d.w f31, x0
.endm

trap_func:
	trap_entry
	//fill0_i64
	//fill0_fp64		/* set all floating registers to zero */
	call handle_trap //\func\tail
	trap_exit
	mret

.global Reset_Handler
.section .startup0
.align 8
Reset_Handler:
	fill0_i64		/* set all integer registers to zero */
	/* stack initilization */
	la	sp, __stack
	mv	ra, zero	// return address - link register
	mv	gp, zero
    mv  a0, zero
    mv  a1, zero
	la  t0, trap_func
   	csrw mtvec, t0

	//fill0_fp64		/* set all floating registers to zero */
	call board_start