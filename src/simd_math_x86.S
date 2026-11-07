.text
    .global _simd_clear_block_stereo
    .global simd_clear_block_stereo
    .global _simd_grain_accumulate_stereo
    .global simd_grain_accumulate_stereo

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

