.text
    .global _simd_clear_block_stereo
    .global simd_clear_block_stereo

    .global _simd_grain_accumulate_stereo
    .global simd_grain_accumulate_stereo

    .global _simd_lerp_block
    .global simd_lerp_block

    .global _simd_svf_process_stereo
    .global simd_svf_process_stereo

// Пусть для маков точка входа будет с нижним подчёркиваем, а для линукса без

_simd_clear_block_stereo:
simd_clear_block_stereo:
    vxorps %ymm0, %ymm0, %ymm0
    vmovups %ymm0, (%rdi)
    vmovups %ymm0, (%rsi)
    vzeroupper
    ret

_simd_grain_accumulate_stereo:
simd_grain_accumulate_stereo:
    vbroadcastss %xmm0, %ymm0
    vbroadcastss %xmm1, %ymm1

    shr $3, %r8
    test %r8, %r8
    jz .Lend

.Lloop:
    vmovups (%rdx), %ymm2
    vmovups (%rcx), %ymm3

    vmulps %ymm3, %ymm2, %ymm2

    vmovups (%rdi), %ymm4
    vmovups (%rsi), %ymm5

    vfmadd231ps %ymm0, %ymm2, %ymm4
    vfmadd231ps %ymm1, %ymm2, %ymm5

    vmovups %ymm4, (%rdi)
    vmovups %ymm5, (%rsi)

    add $32, %rdi
    add $32, %rsi
    add $32, %rdx
    add $32, %rcx

    dec %r8
    jnz .Lloop

.Lend:
    vzeroupper
    ret


_simd_lerp_block:
simd_lerp_block:
    vbroadcastss %xmm0, %ymm0
    
    vmovaps .LC_ONE(%rip), %ymm1
    vsubps %ymm0, %ymm1, %ymm1

    shr $3, %rcx
    test %rcx, %rcx
    jz .Lend_lerp

.Lloop_lerp:
    vmovups (%rsi), %ymm2
    vmovups (%rdx), %ymm3
    vmulps %ymm1, %ymm2, %ymm4
    
    vfmadd231ps %ymm0, %ymm3, %ymm4

    vmovups %ymm4, (%rdi)

    add $32, %rdi
    add $32, %rsi
    add $32, %rdx

    dec %rcx
    jnz .Lloop_lerp

.Lend_lerp:
    vzeroupper
    ret

    .section .rodata
    .align 32
.LC_ONE:
    .float 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0


_simd_svf_process_stereo:
simd_svf_process_stereo:
    vmovss %xmm0, %xmm2
    vaddss %xmm1, %xmm2, %xmm3
    vmulss %xmm0, %xmm3, %xmm3
    vmovss .LC_ONE(%rip), %xmm4
    vaddss %xmm3, %xmm4, %xmm3
    vdivss %xmm3, %xmm4, %xmm4

    vmulss %xmm0, %xmm4, %xmm5

    vmulss %xmm0, %xmm5, %xmm6

    mov %edx, %ecx
    shr $3, %ecx
    jz .Lend_svf

.Lloop_svf:
    vmovups (%rdi), %ymm2
    vfmadd231ps %ymm4, %ymm2, %ymm2 
    vmovups %ymm2, (%rdi)
    
    add $32, %rdi
    add $32, %rsi
    dec %ecx
    jnz .Lloop_svf

.Lend_svf:
    vzeroupper
    ret

.section .rodata
    .align 32
.LC_ONE:
    .float 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0