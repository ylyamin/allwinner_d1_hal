
build/app.elf:     file format elf64-littleriscv


Disassembly of section .text:

0000000040000000 <main>:
        *dst++ = 0;
    }
} */

void main(void)
{
    40000000:	1101                	addi	sp,sp,-32
    40000002:	ec22                	sd	s0,24(sp)
    40000004:	1000                	addi	s0,sp,32
    int a = 1;
    40000006:	4785                	li	a5,1
    40000008:	fef42623          	sw	a5,-20(s0)
    int b = 2;
    4000000c:	4789                	li	a5,2
    4000000e:	fef42423          	sw	a5,-24(s0)
    int c = 0;
    40000012:	fe042223          	sw	zero,-28(s0)
    c = a + b;
    40000016:	fec42703          	lw	a4,-20(s0)
    4000001a:	fe842783          	lw	a5,-24(s0)
    4000001e:	9fb9                	addw	a5,a5,a4
    40000020:	fef42223          	sw	a5,-28(s0)
    40000024:	0001                	nop
    40000026:	6462                	ld	s0,24(sp)
    40000028:	6105                	addi	sp,sp,32
    4000002a:	8082                	ret

000000004000002c <_start>:
.global _start

_start:
	/*disable interrupt*/
	csrw mie, zero
    4000002c:	30401073          	csrw	mie,zero
	/*enable theadisaee*/
	li t1, 0x1 << 22
    40000030:	00400337          	lui	t1,0x400
	csrs mxstatus, t1
    40000034:	7c032073          	csrs	mxstatus,t1
	/*invaild ICACHE/DCACHE/BTB/BHT*/
	li t1, 0x30013
    40000038:	00030337          	lui	t1,0x30
    4000003c:	0133031b          	addiw	t1,t1,19
	csrs mcor, t1
    40000040:	7c232073          	csrs	mcor,t1
	j reset
    40000044:	a009                	j	40000046 <reset>

0000000040000046 <reset>:

reset:
	addi sp, sp, -32
    40000046:	1101                	addi	sp,sp,-32
	sd s0, 8(sp)
    40000048:	e422                	sd	s0,8(sp)
	sd s1, 16(sp)
    4000004a:	e826                	sd	s1,16(sp)
	sd ra, 24(sp)
    4000004c:	ec06                	sd	ra,24(sp)
	mv s0, a0
    4000004e:	842a                	mv	s0,a0

	li t0, 0x07090108
    40000050:	070902b7          	lui	t0,0x7090
    40000054:	1082829b          	addiw	t0,t0,264
	sw zero, (t0)
    40000058:	0002a023          	sw	zero,0(t0) # 7090000 <STACK_SIZE+0x708fc00>
	jal main
    4000005c:	fa5ff0ef          	jal	ra,40000000 <main>

	ld ra, 24(sp)
    40000060:	60e2                	ld	ra,24(sp)
	ld s0, 8(sp)
    40000062:	6422                	ld	s0,8(sp)
	ld s1, 16(sp)
    40000064:	64c2                	ld	s1,16(sp)
	addi sp, sp, 32
    40000066:	6105                	addi	sp,sp,32
	ret
    40000068:	8082                	ret

Disassembly of section .stack:

0000000040000070 <.stack>:
	...

Disassembly of section .debug_info:

0000000000000000 <.debug_info>:
   0:	007d                	c.nop	31
   2:	0000                	unimp
   4:	0004                	0x4
   6:	0000                	unimp
   8:	0000                	unimp
   a:	0108                	addi	a0,sp,128
   c:	000d                	c.nop	3
   e:	0000                	unimp
  10:	cb0c                	sw	a1,16(a4)
  12:	0000                	unimp
  14:	db00                	sw	s0,48(a4)
	...
  26:	0000                	unimp
  28:	0200                	addi	s0,sp,256
  2a:	0704                	addi	s1,sp,896
  2c:	0000                	unimp
  2e:	0000                	unimp
  30:	0000d603          	lhu	a2,0(ra)
  34:	0100                	addi	s0,sp,128
  36:	060e                	slli	a2,a2,0x3
  38:	0000                	unimp
  3a:	4000                	lw	s0,0(s0)
  3c:	0000                	unimp
  3e:	0000                	unimp
  40:	002c                	addi	a1,sp,8
  42:	0000                	unimp
  44:	0000                	unimp
  46:	0000                	unimp
  48:	9c01                	subw	s0,s0,s0
  4a:	0079                	c.nop	30
  4c:	0000                	unimp
  4e:	6104                	ld	s1,0(a0)
  50:	0100                	addi	s0,sp,128
  52:	0910                	addi	a2,sp,144
  54:	0079                	c.nop	30
  56:	0000                	unimp
  58:	0000                	unimp
  5a:	0000                	unimp
  5c:	6204                	ld	s1,0(a2)
  5e:	0100                	addi	s0,sp,128
  60:	0911                	addi	s2,s2,4
  62:	0079                	c.nop	30
  64:	0000                	unimp
  66:	004a                	c.slli	zero,0x12
  68:	0000                	unimp
  6a:	6304                	ld	s1,0(a4)
  6c:	0100                	addi	s0,sp,128
  6e:	0912                	slli	s2,s2,0x4
  70:	0079                	c.nop	30
  72:	0000                	unimp
  74:	0095                	addi	ra,ra,5
  76:	0000                	unimp
  78:	0500                	addi	s0,sp,640
  7a:	0504                	addi	s1,sp,640
  7c:	6e69                	lui	t3,0x1a
  7e:	0074                	addi	a3,sp,12
  80:	2a00                	fld	fs0,16(a2)
  82:	0000                	unimp
  84:	0200                	addi	s0,sp,256
  86:	5700                	lw	s0,40(a4)
  88:	0000                	unimp
  8a:	0800                	addi	s0,sp,16
  8c:	6501                	0x6501
  8e:	0000                	unimp
  90:	2c00                	fld	fs0,24(s0)
  92:	0000                	unimp
  94:	0040                	addi	s0,sp,4
  96:	0000                	unimp
  98:	6a00                	ld	s0,16(a2)
  9a:	0000                	unimp
  9c:	0040                	addi	s0,sp,4
  9e:	0000                	unimp
  a0:	0000                	unimp
  a2:	0001                	nop
  a4:	db00                	sw	s0,48(a4)
  a6:	0000                	unimp
  a8:	0c00                	addi	s0,sp,528
  aa:	0001                	nop
  ac:	0100                	addi	s0,sp,128
  ae:	80              	Address 0x00000000000000ae is out of bounds.


Disassembly of section .debug_abbrev:

0000000000000000 <.debug_abbrev>:
   0:	1101                	addi	sp,sp,-32
   2:	2501                	sext.w	a0,a0
   4:	130e                	slli	t1,t1,0x23
   6:	1b0e030b          	0x1b0e030b
   a:	550e                	lw	a0,224(sp)
   c:	10011117          	auipc	sp,0x10011
  10:	02000017          	auipc	zero,0x2000
  14:	0024                	addi	s1,sp,8
  16:	0b3e0b0b          	0xb3e0b0b
  1a:	00000e03          	lb	t3,0(zero) # 0 <STACK_SIZE-0x400>
  1e:	3f012e03          	lw	t3,1008(sp) # 100113fc <STACK_SIZE+0x10010ffc>
  22:	0319                	addi	t1,t1,6
  24:	3a0e                	fld	fs4,224(sp)
  26:	390b3b0b          	extu	s6,s6,14,16
  2a:	1119270b          	ext	a4,s2,4,17
  2e:	1201                	addi	tp,tp,-32
  30:	97184007          	flq	ft0,-1679(a6)
  34:	1942                	slli	s2,s2,0x30
  36:	1301                	addi	t1,t1,-32
  38:	0000                	unimp
  3a:	3404                	fld	fs1,40(s0)
  3c:	0300                	addi	s0,sp,384
  3e:	3a08                	fld	fa0,48(a2)
  40:	390b3b0b          	extu	s6,s6,14,16
  44:	0213490b          	lrb	s2,t1,ra,1
  48:	05000017          	auipc	zero,0x5000
  4c:	0024                	addi	s1,sp,8
  4e:	0b3e0b0b          	0xb3e0b0b
  52:	00000803          	lb	a6,0(zero) # 0 <STACK_SIZE-0x400>
  56:	0100                	addi	s0,sp,128
  58:	0011                	c.nop	4
  5a:	0610                	addi	a2,sp,768
  5c:	0111                	addi	sp,sp,4
  5e:	0112                	slli	sp,sp,0x4
  60:	0e1b0e03          	lb	t3,225(s6)
  64:	0e25                	addi	t3,t3,9
  66:	00000513          	li	a0,0
	...

Disassembly of section .debug_loc:

0000000000000000 <.debug_loc>:
   0:	000c                	0xc
   2:	4000                	lw	s0,0(s0)
   4:	0000                	unimp
   6:	0000                	unimp
   8:	000e                	c.slli	zero,0x3
   a:	4000                	lw	s0,0(s0)
   c:	0000                	unimp
   e:	0000                	unimp
  10:	0001                	nop
  12:	0e5f 0000 0040      	0x4000000e5f
  18:	0000                	unimp
  1a:	2800                	fld	fs0,16(s0)
  1c:	0000                	unimp
  1e:	0040                	addi	s0,sp,4
  20:	0000                	unimp
  22:	0200                	addi	s0,sp,256
  24:	7800                	ld	s0,48(s0)
  26:	286c                	fld	fa1,208(s0)
  28:	0000                	unimp
  2a:	0040                	addi	s0,sp,4
  2c:	0000                	unimp
  2e:	2c00                	fld	fs0,24(s0)
  30:	0000                	unimp
  32:	0040                	addi	s0,sp,4
  34:	0000                	unimp
  36:	0100                	addi	s0,sp,128
  38:	5e00                	lw	s0,56(a2)
	...
  4a:	0012                	c.slli	zero,0x4
  4c:	4000                	lw	s0,0(s0)
  4e:	0000                	unimp
  50:	0000                	unimp
  52:	0020                	addi	s0,sp,8
  54:	4000                	lw	s0,0(s0)
  56:	0000                	unimp
  58:	0000                	unimp
  5a:	0001                	nop
  5c:	205f 0000 0040      	0x400000205f
  62:	0000                	unimp
  64:	2800                	fld	fs0,16(s0)
  66:	0000                	unimp
  68:	0040                	addi	s0,sp,4
  6a:	0000                	unimp
  6c:	0200                	addi	s0,sp,256
  6e:	7800                	ld	s0,48(s0)
  70:	2868                	fld	fa0,208(s0)
  72:	0000                	unimp
  74:	0040                	addi	s0,sp,4
  76:	0000                	unimp
  78:	2c00                	fld	fs0,24(s0)
  7a:	0000                	unimp
  7c:	0040                	addi	s0,sp,4
  7e:	0000                	unimp
  80:	0200                	addi	s0,sp,256
  82:	9100                	0x9100
  84:	0068                	addi	a0,sp,12
	...
  92:	0000                	unimp
  94:	1600                	addi	s0,sp,800
  96:	0000                	unimp
  98:	0040                	addi	s0,sp,4
  9a:	0000                	unimp
  9c:	2400                	fld	fs0,8(s0)
  9e:	0000                	unimp
  a0:	0040                	addi	s0,sp,4
  a2:	0000                	unimp
  a4:	0200                	addi	s0,sp,256
  a6:	7800                	ld	s0,48(s0)
  a8:	2464                	fld	fs1,200(s0)
  aa:	0000                	unimp
  ac:	0040                	addi	s0,sp,4
  ae:	0000                	unimp
  b0:	2c00                	fld	fs0,24(s0)
  b2:	0000                	unimp
  b4:	0040                	addi	s0,sp,4
  b6:	0000                	unimp
  b8:	0100                	addi	s0,sp,128
  ba:	5f00                	lw	s0,56(a4)
	...

Disassembly of section .debug_aranges:

0000000000000000 <.debug_aranges>:
   0:	002c                	addi	a1,sp,8
   2:	0000                	unimp
   4:	0002                	c.slli64	zero
   6:	0000                	unimp
   8:	0000                	unimp
   a:	0008                	0x8
   c:	0000                	unimp
   e:	0000                	unimp
  10:	0000                	unimp
  12:	4000                	lw	s0,0(s0)
  14:	0000                	unimp
  16:	0000                	unimp
  18:	002c                	addi	a1,sp,8
	...
  2e:	0000                	unimp
  30:	002c                	addi	a1,sp,8
  32:	0000                	unimp
  34:	0002                	c.slli64	zero
  36:	0081                	addi	ra,ra,0
  38:	0000                	unimp
  3a:	0008                	0x8
  3c:	0000                	unimp
  3e:	0000                	unimp
  40:	002c                	addi	a1,sp,8
  42:	4000                	lw	s0,0(s0)
  44:	0000                	unimp
  46:	0000                	unimp
  48:	003e                	c.slli	zero,0xf
	...

Disassembly of section .debug_ranges:

0000000000000000 <.debug_ranges>:
   0:	0000                	unimp
   2:	4000                	lw	s0,0(s0)
   4:	0000                	unimp
   6:	0000                	unimp
   8:	002c                	addi	a1,sp,8
   a:	4000                	lw	s0,0(s0)
	...

Disassembly of section .debug_line:

0000000000000000 <.debug_line>:
   0:	0061                	c.nop	24
   2:	0000                	unimp
   4:	00210003          	lb	zero,2(sp)
   8:	0000                	unimp
   a:	0101                	addi	sp,sp,0
   c:	000d0efb          	0xd0efb
  10:	0101                	addi	sp,sp,0
  12:	0101                	addi	sp,sp,0
  14:	0000                	unimp
  16:	0100                	addi	s0,sp,128
  18:	0000                	unimp
  1a:	7301                	lui	t1,0xfffe0
  1c:	6372                	ld	t1,280(sp)
  1e:	0000                	unimp
  20:	616d                	addi	sp,sp,240
  22:	6e69                	lui	t3,0x1a
  24:	632e                	ld	t1,200(sp)
  26:	0100                	addi	s0,sp,128
  28:	0000                	unimp
  2a:	0500                	addi	s0,sp,640
  2c:	0001                	nop
  2e:	0209                	addi	tp,tp,2
  30:	0000                	unimp
  32:	4000                	lw	s0,0(s0)
  34:	0000                	unimp
  36:	0000                	unimp
  38:	05010e03          	lb	t3,80(sp)
  3c:	0309                	addi	t1,t1,2
  3e:	0901                	addi	s2,s2,0
  40:	0006                	c.slli	zero,0x1
  42:	0301                	addi	t1,t1,0
  44:	0901                	addi	s2,s2,0
  46:	0006                	c.slli	zero,0x1
  48:	0301                	addi	t1,t1,0
  4a:	0901                	addi	s2,s2,0
  4c:	0006                	c.slli	zero,0x1
  4e:	0501                	addi	a0,a0,0
  50:	09010307          	vlsbu.v	v6,(sp),a6,v0.t
  54:	0004                	0x4
  56:	0501                	addi	a0,a0,0
  58:	0301                	addi	t1,t1,0
  5a:	0901                	addi	s2,s2,0
  5c:	000e                	c.slli	zero,0x3
  5e:	0901                	addi	s2,s2,0
  60:	0008                	0x8
  62:	0100                	addi	s0,sp,128
  64:	a601                	j	364 <STACK_SIZE-0x9c>
  66:	0000                	unimp
  68:	0300                	addi	s0,sp,384
  6a:	2200                	fld	fs0,0(a2)
  6c:	0000                	unimp
  6e:	0100                	addi	s0,sp,128
  70:	fb01                	bnez	a4,ffffffffffffff80 <reset+0xffffffffbfffff3a>
  72:	0d0e                	slli	s10,s10,0x3
  74:	0100                	addi	s0,sp,128
  76:	0101                	addi	sp,sp,0
  78:	0001                	nop
  7a:	0000                	unimp
  7c:	0001                	nop
  7e:	0100                	addi	s0,sp,128
  80:	00637273          	csrrci	tp,0x6,6
  84:	7300                	ld	s0,32(a4)
  86:	6174                	ld	a3,192(a0)
  88:	7472                	ld	s0,312(sp)
  8a:	732e                	ld	t1,232(sp)
  8c:	0100                	addi	s0,sp,128
  8e:	0000                	unimp
  90:	0000                	unimp
  92:	0209                	addi	tp,tp,2
  94:	002c                	addi	a1,sp,8
  96:	4000                	lw	s0,0(s0)
  98:	0000                	unimp
  9a:	0000                	unimp
  9c:	0316                	slli	t1,t1,0x5
  9e:	0902                	c.slli64	s2
  a0:	0004                	0x4
  a2:	0301                	addi	t1,t1,0
  a4:	0901                	addi	s2,s2,0
  a6:	0004                	0x4
  a8:	0301                	addi	t1,t1,0
  aa:	0902                	c.slli64	s2
  ac:	0004                	0x4
  ae:	0301                	addi	t1,t1,0
  b0:	0901                	addi	s2,s2,0
  b2:	0008                	0x8
  b4:	0301                	addi	t1,t1,0
  b6:	0901                	addi	s2,s2,0
  b8:	0004                	0x4
  ba:	0301                	addi	t1,t1,0
  bc:	00020903          	lb	s2,0(tp) # 0 <STACK_SIZE-0x400>
  c0:	0301                	addi	t1,t1,0
  c2:	0901                	addi	s2,s2,0
  c4:	0002                	c.slli64	zero
  c6:	0301                	addi	t1,t1,0
  c8:	0901                	addi	s2,s2,0
  ca:	0002                	c.slli64	zero
  cc:	0301                	addi	t1,t1,0
  ce:	0901                	addi	s2,s2,0
  d0:	0002                	c.slli64	zero
  d2:	0301                	addi	t1,t1,0
  d4:	0901                	addi	s2,s2,0
  d6:	0002                	c.slli64	zero
  d8:	0301                	addi	t1,t1,0
  da:	0902                	c.slli64	s2
  dc:	0002                	c.slli64	zero
  de:	0301                	addi	t1,t1,0
  e0:	0901                	addi	s2,s2,0
  e2:	0008                	0x8
  e4:	0301                	addi	t1,t1,0
  e6:	0901                	addi	s2,s2,0
  e8:	0004                	0x4
  ea:	0301                	addi	t1,t1,0
  ec:	0902                	c.slli64	s2
  ee:	0004                	0x4
  f0:	0301                	addi	t1,t1,0
  f2:	0901                	addi	s2,s2,0
  f4:	0002                	c.slli64	zero
  f6:	0301                	addi	t1,t1,0
  f8:	0901                	addi	s2,s2,0
  fa:	0002                	c.slli64	zero
  fc:	0301                	addi	t1,t1,0
  fe:	0901                	addi	s2,s2,0
 100:	0002                	c.slli64	zero
 102:	0301                	addi	t1,t1,0
 104:	0901                	addi	s2,s2,0
 106:	0002                	c.slli64	zero
 108:	0901                	addi	s2,s2,0
 10a:	0002                	c.slli64	zero
 10c:	0100                	addi	s0,sp,128
 10e:	01              	Address 0x000000000000010e is out of bounds.


Disassembly of section .debug_str:

0000000000000000 <.debug_str>:
   0:	6e75                	lui	t3,0x1d
   2:	6e676973          	csrrsi	s2,0x6e6,14
   6:	6465                	lui	s0,0x19
   8:	6920                	ld	s0,80(a0)
   a:	746e                	ld	s0,248(sp)
   c:	4700                	lw	s0,8(a4)
   e:	554e                	lw	a0,240(sp)
  10:	4320                	lw	s0,64(a4)
  12:	3731                	addiw	a4,a4,-20
  14:	3120                	fld	fs0,96(a0)
  16:	2e30                	fld	fa2,88(a2)
  18:	2e31                	addiw	t3,t3,12
  1a:	2030                	fld	fa2,64(s0)
  1c:	6d2d                	lui	s10,0xb
  1e:	646f6d63          	bltu	t5,t1,678 <STACK_SIZE+0x278>
  22:	6c65                	lui	s8,0x19
  24:	6d3d                	lui	s10,0xf
  26:	6465                	lui	s0,0x19
  28:	6e61                	lui	t3,0x18
  2a:	2079                	0x2079
  2c:	6d2d                	lui	s10,0xb
  2e:	7261                	lui	tp,0xffff8
  30:	723d6863          	bltu	s10,gp,760 <STACK_SIZE+0x360>
  34:	3676                	fld	fa2,376(sp)
  36:	6934                	ld	a3,80(a0)
  38:	616d                	addi	sp,sp,240
  3a:	6466                	ld	s0,88(sp)
  3c:	6d2d2063          	0x6d2d2063
  40:	6261                	lui	tp,0x18
  42:	3d69                	addiw	s10,s10,-6
  44:	706c                	ld	a1,224(s0)
  46:	3436                	fld	fs0,360(sp)
  48:	2d20                	fld	fs0,88(a0)
  4a:	746d                	lui	s0,0xffffb
  4c:	6e75                	lui	t3,0x1d
  4e:	3d65                	addiw	s10,s10,-7
  50:	6f72                	ld	t5,280(sp)
  52:	74656b63          	bltu	a0,t1,7a8 <STACK_SIZE+0x3a8>
  56:	2d20                	fld	fs0,88(a0)
  58:	62646767          	0x62646767
  5c:	2d20                	fld	fs0,88(a0)
  5e:	2d20304f          	fnmadd.h	ft0,ft0,fs2,ft5,rup
  62:	7666                	ld	a2,120(sp)
  64:	7261                	lui	tp,0xffff8
  66:	742d                	lui	s0,0xfffeb
  68:	6172                	ld	sp,280(sp)
  6a:	6e696b63          	bltu	s2,t1,760 <STACK_SIZE+0x360>
  6e:	662d2067          	0x662d2067
  72:	7266                	ld	tp,120(sp)
  74:	6565                	lui	a0,0x19
  76:	6e617473          	csrrci	s0,0x6e6,2
  7a:	6964                	ld	s1,208(a0)
  7c:	676e                	ld	a4,216(sp)
  7e:	2d20                	fld	fs0,88(a0)
  80:	6e66                	ld	t3,88(sp)
  82:	6f632d6f          	jal	s10,32778 <STACK_SIZE+0x32378>
  86:	6d6d                	lui	s10,0x1b
  88:	2d206e6f          	jal	t3,635a <STACK_SIZE+0x5f5a>
  8c:	6666                	ld	a2,88(sp)
  8e:	6e75                	lui	t3,0x1d
  90:	6f697463          	bgeu	s2,s6,778 <STACK_SIZE+0x378>
  94:	2d6e                	fld	fs10,216(sp)
  96:	74636573          	csrrsi	a0,0x746,6
  9a:	6f69                	lui	t5,0x1a
  9c:	736e                	ld	t1,248(sp)
  9e:	2d20                	fld	fs0,88(a0)
  a0:	6466                	ld	s0,88(sp)
  a2:	7461                	lui	s0,0xffff8
  a4:	2d61                	addiw	s10,s10,24
  a6:	74636573          	csrrsi	a0,0x746,6
  aa:	6f69                	lui	t5,0x1a
  ac:	736e                	ld	t1,248(sp)
  ae:	2d20                	fld	fs0,88(a0)
  b0:	7366                	ld	t1,120(sp)
  b2:	7274                	ld	a3,224(a2)
  b4:	6369                	lui	t1,0x1a
  b6:	2d74                	fld	fa3,216(a0)
  b8:	6f76                	ld	t5,344(sp)
  ba:	616c                	ld	a1,192(a0)
  bc:	6974                	ld	a3,208(a0)
  be:	656c                	ld	a1,200(a0)
  c0:	622d                	lui	tp,0xb
  c2:	7469                	lui	s0,0xffffa
  c4:	6966                	ld	s2,88(sp)
  c6:	6c65                	lui	s8,0x19
  c8:	7364                	ld	s1,224(a4)
  ca:	7300                	ld	s0,32(a4)
  cc:	6372                	ld	t1,280(sp)
  ce:	69616d2f          	0x69616d2f
  d2:	2e6e                	fld	ft8,216(sp)
  d4:	616d0063          	beq	s10,s6,6d4 <STACK_SIZE+0x2d4>
  d8:	6e69                	lui	t3,0x1a
  da:	2f00                	fld	fs0,24(a4)
  dc:	6e6d                	lui	t3,0x1b
  de:	2f74                	fld	fa3,216(a4)
  e0:	6768                	ld	a0,200(a4)
  e2:	7366                	ld	t1,120(sp)
  e4:	6c6c612f          	0x6c6c612f
  e8:	6e6e6977          	0x6e6e6977
  ec:	7265                	lui	tp,0xffff9
  ee:	645f 2f31 6c61      	0x6c612f31645f
  f4:	776c                	ld	a1,232(a4)
  f6:	6e69                	lui	t3,0x1a
  f8:	656e                	ld	a0,216(sp)
  fa:	5f72                	lw	t5,60(sp)
  fc:	3164                	fld	fs1,224(a0)
  fe:	0068                	addi	a0,sp,12
 100:	2f637273          	csrrci	tp,0x2f6,6
 104:	72617473          	csrrci	s0,0x726,2
 108:	2e74                	fld	fa3,216(a2)
 10a:	4e470073          	0x4e470073
 10e:	2055                	0x2055
 110:	5341                	li	t1,-16
 112:	3220                	fld	fs0,96(a2)
 114:	332e                	fld	ft6,232(sp)
 116:	0032                	c.slli	zero,0xc

Disassembly of section .comment:

0000000000000000 <.comment>:
   0:	3a434347          	fmsub.d	ft6,ft6,ft4,ft7,rmm
   4:	2820                	fld	fs0,80(s0)
   6:	29554e47          	fmsub.s	ft8,fa0,fs5,ft5,rmm
   a:	3120                	fld	fs0,96(a0)
   c:	2e30                	fld	fa2,88(a2)
   e:	2e31                	addiw	t3,t3,12
  10:	0030                	addi	a2,sp,8

Disassembly of section .debug_frame:

0000000000000000 <.debug_frame>:
   0:	000c                	0xc
   2:	0000                	unimp
   4:	ffffffff          	0xffffffff
   8:	7c010003          	lb	zero,1984(sp)
   c:	0d01                	addi	s10,s10,0
   e:	0002                	c.slli64	zero
  10:	002c                	addi	a1,sp,8
	...
  1a:	4000                	lw	s0,0(s0)
  1c:	0000                	unimp
  1e:	0000                	unimp
  20:	002c                	addi	a1,sp,8
  22:	0000                	unimp
  24:	0000                	unimp
  26:	0000                	unimp
  28:	0e42                	slli	t3,t3,0x10
  2a:	4220                	lw	s0,64(a2)
  2c:	0288                	addi	a0,sp,320
  2e:	0c42                	slli	s8,s8,0x10
  30:	0008                	0x8
  32:	c862                	sw	s8,16(sp)
  34:	020c                	addi	a1,sp,256
  36:	4220                	lw	s0,64(a2)
  38:	000e                	c.slli	zero,0x3
  3a:	0000                	unimp
  3c:	0000                	unimp
	...
