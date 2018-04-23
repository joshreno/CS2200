!=================================================================
! General conventions:
!   1) Stack grows from high addresses to low addresses, and the
!      top of the stack points to valid data
!
!   2) Register usage is as implied by assembler names and manual
!
!   3) Function Calling Convention:
!
!       Setup)
!       * Immediately upon entering a function, push the RA on the stack.
!       * Next, push all the registers used by the function on the stack.
!
!       Teardown)
!       * Load the return value in $v0.
!       * Pop any saved registers from the stack back into the registers.
!       * Pop the RA back into $ra.
!       * Return by executing jalr $ra, $zero.
!=================================================================

        ! vector table
vector0:
        .fill 0x00000000            ! device ID 0
        .fill 0x00000000            ! device ID 1
        .fill 0x00000000            ! ...
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000            ! device ID 15
        ! end vector table

main:   lea $sp, stack              ! initialize the stack pointer
        lw $sp, 0($sp)              ! finish initialization

                                    ! Install timer interrupt handlers into vector table
        lea $t0, vector0
        lea $t1, t1_handler
        sw $t1, 1($t0)
        lea $t1, t2_handler
        sw $t1, 2($t0)
        lea $t1, keyboard
        sw $t1, 3($t0)

                                    ! Dont forget to enable interrupts
        ei

        lea $a0, BASE               ! load base for pow
        lw $a0, 0($a0)

        lea $a1, EXP                ! load exp for pow
        lw $a1, 0($a1)

        lea $at, POW                ! load address of pow
        jalr $at, $ra               ! run pow

        lea $a2, ANS               ! store answer from pow
        sw $v0, 0($a2)

        halt

BASE:   .fill 2
EXP:    .fill 12
ANS:    .fill 0                     ! should come out to 4096

POW:    addi $sp, $sp, -1           ! allocate space for old frame pointer
        sw $fp, 0($sp)
        addi $fp, $sp, 0            ! set new frame pinter

        skpe $a1, $zero             ! check if $a1 is zero
        goto CONT1
        goto RET1                   ! if the power is 0 return 1

CONT1:  skpe $a0, $zero
        goto CONT2
        goto RET0                   ! if the base is 0 return 0

CONT2:  addi $a1, $a1, -1           ! decrement the power
        lea $at, POW                ! load the address of POW
        addi $sp, $sp, -2           ! push 2 slots onto the stack
        sw $ra, -1($fp)             ! save RA to stack
        sw $a0, -2($fp)             ! save arg 0 to stack
        jalr $at, $ra               ! recursively call POW
        add $a1, $v0, $zero         ! store return value in arg 1
        lw $a0, -2($fp)             ! load the base into arg 0
        lea $at, MULT               ! load the address of MULT
        jalr $at, $ra               ! multiply arg 0 (base) and arg 1 (running product)
        lw $ra, -1($fp)             ! load RA from the stack
        addi $sp, $sp, 2
        goto FIN                    ! return

RET1:   addi $v0, $zero, 1          ! return a value of 1
        goto FIN

RET0:   add $v0, $zero, $zero       ! return a value of 0

FIN:    lw $fp, 0($fp)              ! restore old frame pointer
        addi $sp, $sp, 1            ! pop off the stack
        jalr $ra, $zero


MULT:   add $v0, $zero, $zero       ! zero out return value
AGAIN:  add $v0, $v0, $a0           ! multiply loop
        addi $a1, $a1, -1           ! a1 -= 1

        skpe $a1, $zero             ! finished multiplying
        goto AGAIN                  ! loop again

DONE:   jalr $ra, $zero

t1_handler:
        addi $sp, $sp, -14
        sw $k0, 11($sp)
        ei
        sw $at, 0($sp)
        sw $v0, 1($sp)
        sw $a0, 2($sp)
        sw $a1, 3($sp)
        sw $a2, 4($sp)
        sw $t0, 5($sp)
        sw $t1, 6($sp)
        sw $t2, 7($sp)
        sw $s0, 8($sp)
        sw $s1, 9($sp)
        sw $s2, 10($sp)
        sw $fp, 12($sp)
        sw $ra, 13($sp)

        lea $t0, ticks1
        lw $t0, 0($t0)
        lw $t1, 0($t0)
        addi $t1, $t1, 1
        sw $t1, 0($t0)

        lw $at, 0($sp)
        lw $v0, 1($sp)
        lw $a0, 2($sp)
        lw $a1, 3($sp)
        lw $a2, 4($sp)
        lw $t0, 5($sp)
        lw $t1, 6($sp)
        lw $t2, 7($sp)
        lw $s0, 8($sp)
        lw $s1, 9($sp)
        lw $s2, 10($sp)
        lw $fp, 12($sp)
        lw $ra, 13($sp)

        di
        lw $k0, 11($sp)
        addi $sp, $sp, 14
        reti


t2_handler:
        addi $sp, $sp, -14
        sw $k0, 11($sp)
        ei
        sw $at, 0($sp)
        sw $v0, 1($sp)
        sw $a0, 2($sp)
        sw $a1, 3($sp)
        sw $a2, 4($sp)
        sw $t0, 5($sp)
        sw $t1, 6($sp)
        sw $t2, 7($sp)
        sw $s0, 8($sp)
        sw $s1, 9($sp)
        sw $s2, 10($sp)
        sw $fp, 12($sp)
        sw $ra, 13($sp)

        lea $t0, ticks2
        lw $t0, 0($t0)
        lw $t1, 0($t0)
        addi $t1, $t1, 1
        sw $t1, 0($t0)

        lw $at, 0($sp)
        lw $v0, 1($sp)
        lw $a0, 2($sp)
        lw $a1, 3($sp)
        lw $a2, 4($sp)
        lw $t0, 5($sp)
        lw $t1, 6($sp)
        lw $t2, 7($sp)
        lw $s0, 8($sp)
        lw $s1, 9($sp)
        lw $s2, 10($sp)
        lw $fp, 12($sp)
        lw $ra, 13($sp)

        di
        lw $k0, 11($sp)
        addi $sp, $sp, 14
        reti

keyboard:
        addi $sp, $sp, -14
        sw $k0, 11($sp)
        ei
        sw $at, 0($sp)
        sw $v0, 1($sp)
        sw $a0, 2($sp)
        sw $a1, 3($sp)
        sw $a2, 4($sp)
        sw $t0, 5($sp)
        sw $t1, 6($sp)
        sw $t2, 7($sp)
        sw $s0, 8($sp)
        sw $s1, 9($sp)
        sw $s2, 10($sp)
        sw $fp, 12($sp)
        sw $ra, 13($sp)

        lea $s0, start
        lw $t0, 0($s0)
        lea $s1, count
        lw $t1, 0($s1)
        lea $s2, end
        lw $t2, 0($s2)
        add $t0, $t0, $t1       !start + count

        skpe $t2, $t0
        goto thing
        goto zero

thing:
        skpgt $t2, $t0
        sw $zero, 0($s1)
        lw $t1, 0($s1)

        IN $s1, 0x3
        sw $s1, 0($t0)
        !store s1 where current is start + count DONE
        !increment count and store that in count
        ! mod correctly - not storing
        addi $t1, $t1, 1
        lea $t0, count
        sw $t1, 0($t0)
        goto restore

zero:
        IN $s1, 0x3
        sw $s1, 0($t0)
        addi $t1, $zero, 0
        lea $t0, count
        sw $t1, 0($t0)

restore:
        lw $at, 0($sp)
        lw $v0, 1($sp)
        lw $a0, 2($sp)
        lw $a1, 3($sp)
        lw $a2, 4($sp)
        lw $t0, 5($sp)
        lw $t1, 6($sp)
        lw $t2, 7($sp)
        lw $s0, 8($sp)
        lw $s1, 9($sp)
        lw $s2, 10($sp)
        lw $fp, 12($sp)
        lw $ra, 13($sp)

        di
        lw $k0, 11($sp)
        addi $sp, $sp, 14
        reti



stack:  .fill 0xA00000

ticks1: .fill 0xFFFFFD
ticks2: .fill 0xFFFFFE

start:  .fill 0xFFFF00
end:    .fill 0xFFFF0F
count:  .fill 0x000000
