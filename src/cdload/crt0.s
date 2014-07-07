!
! Bart's Custom Sega Saturn Start-Up Code
! Bart Trzynadlowski, 2001
! Public domain
!
! Modifications for Position Independent Code made by Theo Berkau, 2012
!
! For use with the GNU C Compiler.
!
! Make sure this is the first file linked into your project, so that it is at
! the very beginning. Use the BART.LD linker script. Load the resulting
! binary at any address on the Sega Saturn will begin execution there. 
!
! Please note that stack will be defined as the area before your start address!
! e.g. If you loaded the binary to 0x06004000, 0x06003FFF and below will be 
! used for stack. So don't load the binary to an address with no memory above
! (such as 0x00200000 for example) or where important data may be residing.
!

.section .text

!
! Entry point
!
.global start
start:
    ! Calculate real start address and store for future use
    mov.l   start_ptr,r0

    !
    ! Set initial stack pointer. Stack is from (Start Address - 0x2000) -> (Start Address - 1)
    !

    mov     r0,r15

    !
    ! Clear BSS
    !    
    ! We need to add the start address since bss_start and bss_end will be offsets
    mov.l   bss_start,r0
    mov.l   bss_end,r1
    mov     #0,r2
lbss:
    cmp/ge  r1,r0
    bt      lbss_end
    mov.b   r2,@r0
    add     #1,r0
    bra     lbss
    nop
lbss_end:
       
    !
    ! Jump to _main()
    !
    mov.l   main_ptr,r0
    jsr @r0
    nop

    !
    ! When _main() terminates, jump back to the ARP entry point
    !
    mov.l   arp_ptr,r0
    jmp @r0
    nop
    nop

    !
    ! Following is never reached, it was the from the original
    ! version of crt0.s
    !

    !
    ! Once _main() has terminated, disable interrupts and loop infinitely
    !
    mov     #0xf,r0
    shll2   r0
    shll2   r0
    ldc     r0,sr
end:
    bra     end
    nop
.align 2
start_ptr:  .long start
bss_start:  .long __bss_start
bss_end:    .long __bss_end
main_ptr:   .long _main
arp_ptr:    .long 0x02000100

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
    mov.l   ip_ptr,r0
    jsr     @r0
    nop
endlessloop:
    bra     endlessloop
    nop
.align 4

stack_ptr2:    .long 0x06002000
ip_ptr:        .long 0x06002100
CHGSCUMSK_ptr: .long 0x06000344
NEWMSK:        .long 0xFFFF7FFF
VDP2REGS_ptr:  .long 0x25F80000
ipvar1_ptr:    .long 0x06000290
ipvar2_ptr:    .long 0x060002B0
