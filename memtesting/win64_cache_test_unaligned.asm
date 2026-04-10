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
; NOTE: This ASM routine is written for the Windows 64-bit ABI.
;
;    rcx: test_size (must be evenly divisible by 512)
;    rdx: data pointer
;     r8: repetition_count
;

Read_32x8:
align 64
.outer_loop:
    ; Update the read base pointer to point to the start of the block
    mov rax, rdx
    ; Reset the inner repetition_count
    mov r9, rcx

    .inner_loop:
        ; Read 512 bytes
        vmovdqu64 zmm0, [rax]
        vmovdqu64 zmm0, [rax + 0x40]
        vmovdqu64 zmm0, [rax + 0x80]
        vmovdqu64 zmm0, [rax + 0xc0]
        vmovdqu64 zmm0, [rax + 0x100]
        vmovdqu64 zmm0, [rax + 0x140]
        vmovdqu64 zmm0, [rax + 0x180]
        vmovdqu64 zmm0, [rax + 0x1c0]

        ; Update the read base pointer to point to the new offset
        add rax, 0x200

        ; Repeat until we covered the entire test_size in 512 byte chunks
        sub r9, 0x200
        jnz .inner_loop

    ; Loop until we read the `test_size` an amount of `repetition_count` times
    ; This means that we will read a total of `test_size` * `repetition_count` bytes
    dec r8
    jnz .outer_loop
    ret
