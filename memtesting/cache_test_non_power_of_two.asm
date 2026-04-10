;  ========================================================================
;
;  (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
;
;  This software is provided 'as-is', without any express or implied
;  warranty. In no event will the authors be held liable for any damages
;  arising from the use of this software.
;
;  Please see https://computerenhance.com for more information
;
;  ========================================================================

;  ========================================================================
;  LISTING 152
;  ========================================================================

global Read_32x8

section .text

;
; NOTE: This ASM routine is written for the Linux 64-bit ABI.
;
;    rdi: test_size (must be evenly divisible by 256)
;    rsi: data pointer
;    rdx: repetition_count
;

Read_32x8:
.outer_loop:
    ; Update the read base pointer to point to the start of the block
    mov rax, rsi
    ; Reset the inner repetition_count
    mov r8, rdi

	align 64
    .inner_loop:
        ; Read 256 bytes
        vmovdqu ymm0, [rax]
        vmovdqu ymm0, [rax + 0x20]
        vmovdqu ymm0, [rax + 0x40]
        vmovdqu ymm0, [rax + 0x60]
        vmovdqu ymm0, [rax + 0x80]
        vmovdqu ymm0, [rax + 0xa0]
        vmovdqu ymm0, [rax + 0xc0]
        vmovdqu ymm0, [rax + 0xe0]

        ; Update the read base pointer to point to the new offset
        add rax, 0x100

        ; Repeat until we covered the entire test_size in 256 byte chunks
        sub r8, 0x100
        jnz .inner_loop

    ; Loop until we read the `test_size` an amount of `repetition_count` times
    ; This means that we will read a total of `test_size` * `repetition_count` bytes
    dec rdx
    jnz .outer_loop
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
