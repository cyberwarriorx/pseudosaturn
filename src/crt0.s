!   Copyright 2011-2014 Theo Berkau
!
!   This file is part of PseudoSaturn.
!
!   PseudoSaturn is free software; you can redistribute it and/or modify
!   it under the terms of the GNU General Public License as published by
!   the Free Software Foundation; either version 2 of the License, or
!   (at your option) any later version.
!
!   PseudoSaturn is distributed in the hope that it will be useful,
!   but WITHOUT ANY WARRANTY; without even the implied warranty of
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!   GNU General Public License for more details.
!
!   You should have received a copy of the GNU General Public License
!   along with PseudoSaturn; if not, write to the Free Software
!   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

! Based on Bart's Custom Sega Saturn Start-Up Code

.section .text

!
! Entry point
!
.global start
start:
    ! Disable interrupts
    mov     #0xf,r0
    shll2   r0
    shll2   r0
    ldc     r0,sr

    !bra     truestart 
!.ifdef ENABLE_DEBUG
!    bra  lprog_end
!.else
    bra  wramcopy
!.endif
    nop
! Start a list of usable functions here(for future use)
    
truestart:
    ! Check for extra ram
    mov.l   cartid, r1
    mov.b   @r1, r0
    cmp/eq  #0x5A, r0
    bf      wramcopy
    cmp/eq  #0x5C, r0
    bf      wramcopy
    ! Found extra ram, now let's initialize it
    mov.l   initram, r0
    mov     #1, r1
    mov.b   r1, @r0

wramcopy:
    ! Copy program to wram
    mov.l   prog_start, r0
    mov.l   prog_end, r1
    mov     #0, r2
    mov.l   rom_start, r3
    
lprog:
    cmp/hs  r1, r0
    bt      lprog_end
    mov.l   @r3, r4
    mov.l   r4, @r0
    add     #4, r0
    add     #4, r3
    bra     lprog 
    nop   
 lprog_end:
    
    !
    ! Clear BSS
    !
    mov.l   bss_start,r0
    mov.l   bss_end,r1
    mov     #0,r2
lbss:
    cmp/hs  r1, r0
    bt      lbss_end
    mov.b   r2,@r0
    add     #1,r0
    bra     lbss
    nop
lbss_end:

    !
    ! Set initial stack pointer. Stack is from 0x6002000-0x6003FFF
    !
    mov.l   stack_ptr,r15

    !
    ! Jump to main()
    !
    mov.l   main_ptr,r0
    jsr @r0
    nop

    !
    ! Once main() has terminated, disable interrupts and loop infinitely
    !
    mov     #0xf,r0
    shll2   r0
    shll2   r0
    ldc     r0,sr
end:
    bra     end
    nop
.align 4
cartid:     .long 0x24FFFFFF
initram:    .long 0x257EFFFE
main_ptr:   .long _main
stack_ptr:  .long 0x00300000 ! stack is from 0x002FC000-0x002FFFFF
bss_start:  .long __bss_start
bss_end:    .long __bss_end
.ifdef ENABLE_DEBUG
rom_start:  .long 0x06004000
.else
rom_start:  .long 0x02000F00
.endif
prog_start: .long start
prog_end:   .long _end

! This is to keep libc happy

.global _atexit
_atexit:
   bra end
   nop

.global _ipprog
_ipprog:
    ! Setup a few variables or the ip will die
    xor     r0, r0
    mov.l   ipvar1_ptr, r1
    mov.l   r0, @r1
    mov.l   ipvar2_ptr, r1
    mov.l   r0, @r1

    ! fix stack
    mov.l   stack_ptr2,r15

    ! Change SCU Mask
    mov.l   CHGSCUMSK_ptr, r0
    mov.l   @r0, r0
    mov.l   NEWMSK, r5
    jsr     @r0
    mov     #-1, r4

    ! Clear vdp2 regs
    mov     #0x80, r4
    extu.b  r4, r4
    mov.l   VDP2REGS_ptr, r0
vdpregsloop:
    mov     #0, r1
    mov.l   r1, @r0
    add     #-1, r4
    add     #4, r0
    cmp/pl  r4
    bt      vdpregsloop

    ! execute ip program
    mov.l   _ip_ptr,r0
    jsr     @r0
    nop
endlessloop:
    bra     endlessloop
    nop
.align 4

stack_ptr2:    .long 0x06002000
.global _ip_ptr
_ip_ptr:       .long 0x06002100
CHGSCUMSK_ptr: .long 0x06000344
NEWMSK:        .long 0xFFFF7FFF
VDP2REGS_ptr:  .long 0x25F80000
ipvar1_ptr:    .long 0x06000290
ipvar2_ptr:    .long 0x060002B0
