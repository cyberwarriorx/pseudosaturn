!   Copyright 2011-2014 Theo Berkau
!
!   This file is part of PseudoSaturn.
!
!   PseudoSaturn is free software; you can redistribute it and/or modify
!   it under the terms of the GNU General Public License as published by
!   the Free Software Foundation; either version 2 of the License, or
!   (at your option) any later version.

!   PseudoSaturn is distributed in the hope that it will be useful,
!   but WITHOUT ANY WARRANTY; without even the implied warranty of
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!   GNU General Public License for more details.

!   You should have received a copy of the GNU General Public License
!   along with PseudoSaturn; if not, write to the Free Software
!   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

! This needs rewriting

cheathandler:
	bra     loc_200304C
	bra     loc_2003060

intexithandler:
    nop
	nop
	nop
	lds.l   @r15+, pr
	rte
	nop

	nop
	nop
	nop
	nop
	nop
	mov.l   r0, @-r15
	mov.l   r1, @-r15
	mov.l   off_2003128, r1
	mov.l   @r1, r0
	mov.l   @r15+, r1
	jmp     @r0
	nop

loc_200304C:
	mov.l   r1, @-r15
	sts.l   pr, @-r15
	mov.l   @r15+, r0
	add     #6, r0
	mov.l   off_2003128, r1
	mov.l   r0, @r1
	mov.l   @r15+, r1
	lds.l   @r15+, pr
	mov.l   @r15+, r0
	nop
loc_2003060:
	sts.l   pr, @-r15
	mov.l   r0, @-r15
	mov.l   r1, @-r15
	mov.l   r2, @-r15
	mov.l   r3, @-r15
	mov.l   r4, @-r15
	mov.l   r5, @-r15
	mov.l   r6, @-r15
	mov.l   dword_2003138, r1
	add     #4, r1
	mov.l   @r1, r0
	shlr16  r0
	cmp/eq  #0, r0
	bf      loc_20030D6
	mov     #1, r0
	mov.w   r0, @r1
	mov.l   dword_2003138, r5
	mov.l   dword_200313C, r3
	mov     r3, r0
	cmp/eq  #0, r0
	bt      loc_20030C6
	mov     #1, r6
	mov.l   dword_2003140, r2
loc_200308E:
	mov.l   @r5, r0
	mov     r5, r1
	add     #4, r1
	mov.l   @r1, r4
	and     r2, r0
	cmp/eq  #0, r0
	bf      loc_20030EC
	mov.l   @r5, r0
	mov.l   dword_2003134, r1
	and     r1, r0
	mov.w   r4, @r0
loc_20030A4:
	add     #8, r5
	dt      r3
	bf      loc_200308E
	nop
	mov     r6, r0
	cmp/eq  #0, r0
	bt      loc_20030C6
	mov.l   off_200312C, r5
	mov.l   @r5, r0
	add     #1, r0
	cmp/eq  #0, r0
	bf      loc_20030C6
	add     #-0x20, r5
	mov     #4, r3
	mov     #0, r6
	bra     loc_200308E
	nop
loc_20030C6:
	mov.l   dword_2003138, r1
	mov.l   __cl_check2, r0
	jsr     @r0 ! cl_check2
	nop
	mov.l   dword_2003138, r1
	add     #4, r1
	mov     #0, r0
	mov.w   r0, @r1
loc_20030D6:
	mov.l   @r15+, r6
	mov.l   @r15+, r5
	mov.l   @r15+, r4
	mov.l   @r15+, r3
	mov.l   @r15+, r2
	mov.l   @r15+, r1
	mov.l   @r15+, r0
	lds.l   @r15+, pr
	sts.l   pr, @-r15
	bra     intexithandler
	nop
loc_20030EC:
	mov.l   dword_2003130, r1
	cmp/eq  r1, r0
	bt      loc_20030FE
	mov.l   @r5, r0
	mov.l   dword_2003134, r1
	and     r1, r0
	mov.b   r4, @r0
	bra     loc_20030A4
	nop
loc_20030FE:
	mov.l   @r5, r0
	shll16  r4
	shlr16  r4
	mov.l   dword_2003134, r1
	and     r1, r0
	mov.w   @r0, r0
	shll16  r4
	shlr16  r4
	shll16  r0
	shlr16  r0
	cmp/eq  r4, r0
	bf      loc_20030C6
	bra     loc_20030A4
	nop
.align 4
off_2003128:	.long 0x06000E28
off_200312C:    .long 0x06000E20
dword_2003130:  .long 0xD0000000
dword_2003134:  .long 0x0FFFFFFF
dword_2003138:  .long 0
dword_200313C:  .long 0
dword_2003140:  .long 0xF0000000
__cl_check2:       .long _cl_check2

