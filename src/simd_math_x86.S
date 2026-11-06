.text
    .global _simd_grain_accumulate
    .global simd_grain_accumulate

// Пусть для маков точка входа будет с нижним подчёркиваем, а для линукса без

_simd_grain_accumulate:
simd_grain_accumulate:
    vbroadcastss %xmm0, %ymm0

    shr $3, %rcx

    test %rcx, %rcx
    jz .Lend

.Llopp:
    vmovups (%rsi), %ymm1
    vmovups (%rdx), %ymm2
    vmulps %ymm2, %ymm1, %ymm1

    vmovups (%rdi), %ymm3
    vfmadd231ps %ymm0, %ymm1, %ymm3
    vmovups %ymm3, (%rdi)

    add $32, %rdi
    add $32, %rsi
    add $32, %rdx

    dec %rcx
    jnz .Lloop

.Lend:
    vzeroupper
    ret
