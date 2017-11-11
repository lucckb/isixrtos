/*
 * fputest.S
 * Single precision base stack operation tests
 *  Created on: 3 gru 2013
 *      Author: lucck
 */


.align 4
.thumb
.syntax unified
.section .text 

fone: .float 1.0

.global _ZN5tests6fpu_sp12fill_and_addEi
.func _ZN5tests6fpu_sp12fill_and_addEi
_ZN5tests6fpu_sp12fill_and_addEi:
	vldr s0, fone
	vmov s1, r0
	vcvt.f32.s32 s1, s1
	vadd.f32  s1,s1,s0
	vadd.f32  s2,s1,s0
	vadd.f32  s3,s2,s0
	vadd.f32  s4,s3,s0
	vadd.f32  s5,s4,s0
	vadd.f32  s6,s5,s0
	vadd.f32  s7,s6,s0
	vadd.f32  s8,s7,s0
	vadd.f32  s9,s8,s0
	vadd.f32  s10,s9,s0
	vadd.f32  s11,s10,s0
	vadd.f32  s12,s11,s0
	vadd.f32  s13,s12,s0
	vadd.f32  s14,s13,s0
	vadd.f32  s15,s14,s0
	vadd.f32  s16,s15,s0
	vadd.f32  s17,s16,s0
	vadd.f32  s18,s17,s0
	vadd.f32  s19,s18,s0
	vadd.f32  s20,s19,s0
	vadd.f32  s17,s16,s0
	vadd.f32  s18,s17,s0
	vadd.f32  s19,s18,s0
	vadd.f32  s20,s19,s0
	vadd.f32  s21,s20,s0
	vadd.f32  s22,s21,s0
	vadd.f32  s23,s22,s0
	vadd.f32  s24,s23,s0
	vadd.f32  s25,s24,s0
	vadd.f32  s26,s25,s0
	vadd.f32  s27,s26,s0
	vadd.f32  s28,s27,s0
	vadd.f32  s29,s28,s0
	vadd.f32  s30,s29,s0
	vadd.f32  s31,s30,s0
	vadd.f32  s0,s31,s0
    vcvt.s32.f32 s0, s0
    vcvt.s32.f32 s1, s1
    vcvt.s32.f32 s2, s2
    vcvt.s32.f32 s3, s3
    vcvt.s32.f32 s4, s4
    vcvt.s32.f32 s5, s5
    vcvt.s32.f32 s6, s6
    vcvt.s32.f32 s7, s7
    vcvt.s32.f32 s8, s8
    vcvt.s32.f32 s9, s9
    vcvt.s32.f32 s10, s10
    vcvt.s32.f32 s11, s11
    vcvt.s32.f32 s12, s12
    vcvt.s32.f32 s13, s13
    vcvt.s32.f32 s14, s14
    vcvt.s32.f32 s15, s15
    vcvt.s32.f32 s16, s16
    vcvt.s32.f32 s17, s17
    vcvt.s32.f32 s18, s18
    vcvt.s32.f32 s19, s19
    vcvt.s32.f32 s20, s20
    vcvt.s32.f32 s21, s21
    vcvt.s32.f32 s22, s22
    vcvt.s32.f32 s23, s23
    vcvt.s32.f32 s24, s24
    vcvt.s32.f32 s25, s25
    vcvt.s32.f32 s26, s26
    vcvt.s32.f32 s27, s27
    vcvt.s32.f32 s28, s28
    vcvt.s32.f32 s29, s29
    vcvt.s32.f32 s30, s30
    vcvt.s32.f32 s31, s31
	bx lr
.endfunc

.macro ccmp sreg
    vmov r1, s\sreg
    cmp r1, r0
    beq ok_1
    mov r0,r1
    b fail_1
.endm

.macro vcmp sreg
    vmov r1, s\sreg
    add.w r0, #1
    cmp r1,r0
    beq ok_2
    mov r0,r1
    b fail_2
.endm

.global _ZN5tests6fpu_sp18fill_and_add_checkEi
.func _ZN5tests6fpu_sp18fill_and_add_checkEi
_ZN5tests6fpu_sp18fill_and_add_checkEi:
    vcmp 1
    vcmp 2
    vcmp 3
    vcmp 4
    vcmp 5
    vcmp 6
    vcmp 7
    vcmp 8
    vcmp 9
    vcmp 10
    vcmp 11
    vcmp 12
    vcmp 13
    vcmp 14
    vcmp 15
    vcmp 16
    vcmp 17
    vcmp 18
    vcmp 19
    vcmp 20
    vcmp 21
    vcmp 22
    vcmp 23
    vcmp 24
    vcmp 25
    vcmp 26
    vcmp 28
    vcmp 29
    vcmp 30
    vcmp 31
    vcmp 0
ok_1:
    mov r0, #0
    bx lr
fail_1:
    bx lr
.endfunc

.global _ZN5tests6fpu_sp14base_regs_fillEi
.func _ZN5tests6fpu_sp14base_regs_fillEi
_ZN5tests6fpu_sp14base_regs_fillEi:
    vmov d0, r0, r0
	vmov d1, r0, r0
	vmov d2, r0, r0
	vmov d3, r0, r0
	vmov d4, r0, r0
	vmov d5, r0, r0
	vmov d6, r0, r0
	vmov d7, r0, r0
	bx lr
.endfunc

.global _ZN5tests6fpu_sp15base_regs_checkEi
.func   _ZN5tests6fpu_sp15base_regs_checkEi
_ZN5tests6fpu_sp15base_regs_checkEi:
    ccmp 0
    ccmp 1
    ccmp 2
    ccmp 3
    ccmp 4
    ccmp 5
    ccmp 6
    ccmp 7
    ccmp 8
    ccmp 9
    ccmp 10
    ccmp 11
    ccmp 12
    ccmp 13
    ccmp 14
    ccmp 15
ok_2:
    mov r0, #0
    bx lr
fail_2:
    bx lr  
.endfunc


.end

