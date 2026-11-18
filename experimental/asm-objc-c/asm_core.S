/* =========================================================================
 * Looping Experimental Runtime - Pure Assembly Core (ARM64 & x86_64)
 * Zero external runtime dependencies: strlen, FNV-1a 64, pixel clear, rect fill.
 * ========================================================================= */

#if defined(__APPLE__)
#define CSYM(name) _##name
#else
#define CSYM(name) name
#endif

.text
.p2align 4

#if defined(__aarch64__) || defined(_M_ARM64)

/* size_t exp_asm_strlen(const char* s) */
.globl CSYM(exp_asm_strlen)
CSYM(exp_asm_strlen):
    mov     x1, x0
    cbz     x0, .Llen_zero_a64
.Llen_loop_a64:
    ldrb    w2, [x1], #1
    cbnz    w2, .Llen_loop_a64
    sub     x1, x1, #1
    sub     x0, x1, x0
    ret
.Llen_zero_a64:
    mov     x0, #0
    ret

/* uint64_t exp_asm_fnv1a64(const void* data, size_t len) */
.globl CSYM(exp_asm_fnv1a64)
CSYM(exp_asm_fnv1a64):
    movz    x2, #0x2325
    movk    x2, #0x8422, lsl #16
    movk    x2, #0x9ce4, lsl #32
    movk    x2, #0xcbf2, lsl #48
    movz    x3, #0x01b3
    movk    x3, #0x0100, lsl #32
    cbz     x1, .Lfnv_done_a64
.Lfnv_loop_a64:
    ldrb    w4, [x0], #1
    eor     x2, x2, x4
    mul     x2, x2, x3
    subs    x1, x1, #1
    b.ne    .Lfnv_loop_a64
.Lfnv_done_a64:
    mov     x0, x2
    ret

/* void exp_asm_clear_rgba32(uint32_t* dst, size_t count, uint32_t px) */
.globl CSYM(exp_asm_clear_rgba32)
CSYM(exp_asm_clear_rgba32):
    cbz     x1, .Lclr_done_a64
    mov     w3, w2
    bfi     x3, x2, #32, #32
.Lclr_pair_a64:
    cmp     x1, #2
    b.lo    .Lclr_tail_a64
    str     x3, [x0], #8
    sub     x1, x1, #2
    b       .Lclr_pair_a64
.Lclr_tail_a64:
    cbz     x1, .Lclr_done_a64
    str     w2, [x0]
.Lclr_done_a64:
    ret

/* void exp_asm_fill_rect_rgba32(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h, uint32_t px) */
.globl CSYM(exp_asm_fill_rect_rgba32)
CSYM(exp_asm_fill_rect_rgba32):
    cmp     w5, #0
    b.le    .Lrect_done_a64
    cmp     w6, #0
    b.le    .Lrect_done_a64
    add     w8, w3, w5
    cmp     w3, #0
    csel    w3, wzr, w3, lt
    cmp     w8, w1
    csel    w8, w1, w8, gt
    sub     w5, w8, w3
    cmp     w5, #0
    b.le    .Lrect_done_a64
    add     w9, w4, w6
    cmp     w4, #0
    csel    w4, wzr, w4, lt
    cmp     w9, w2
    csel    w9, w2, w9, gt
    sub     w6, w9, w4
    cmp     w6, #0
    b.le    .Lrect_done_a64
    sxtw    x10, w1
    sxtw    x11, w4
    sxtw    x12, w3
    mul     x11, x11, x10
    add     x11, x11, x12
    lsl     x11, x11, #2
    add     x0, x0, x11
    lsl     x10, x10, #2
.Lrect_row_a64:
    mov     x13, x0
    mov     w14, w5
.Lrect_col_a64:
    str     w7, [x13], #4
    subs    w14, w14, #1
    b.ne    .Lrect_col_a64
    add     x0, x0, x10
    subs    w6, w6, #1
    b.ne    .Lrect_row_a64
.Lrect_done_a64:
    ret

#elif defined(__x86_64__) || defined(_M_X64)

/* size_t exp_asm_strlen(const char* s) */
.globl CSYM(exp_asm_strlen)
CSYM(exp_asm_strlen):
    xor     %rax, %rax
    test    %rdi, %rdi
    jz      .Llen_done_x64
.Llen_loop_x64:
    cmpb    $0, (%rdi, %rax, 1)
    je      .Llen_done_x64
    inc     %rax
    jmp     .Llen_loop_x64
.Llen_done_x64:
    ret

/* uint64_t exp_asm_fnv1a64(const void* data, size_t len) */
.globl CSYM(exp_asm_fnv1a64)
CSYM(exp_asm_fnv1a64):
    movabs  $0xcbf29ce484222325, %rax
    movabs  $0x00000100000001b3, %r8
    test    %rsi, %rsi
    jz      .Lfnv_done_x64
.Lfnv_loop_x64:
    movzbl  (%rdi), %ecx
    xor     %rcx, %rax
    imul    %r8, %rax
    inc     %rdi
    dec     %rsi
    jnz     .Lfnv_loop_x64
.Lfnv_done_x64:
    ret

/* void exp_asm_clear_rgba32(uint32_t* dst, size_t count, uint32_t px) */
.globl CSYM(exp_asm_clear_rgba32)
CSYM(exp_asm_clear_rgba32):
    test    %rsi, %rsi
    jz      .Lclr_done_x64
    mov     %edx, %eax
    mov     %rsi, %rcx
    cld
    rep     stosl
.Lclr_done_x64:
    ret

/* void exp_asm_fill_rect_rgba32(uint32_t* fb, int fb_w, int fb_h, int x, int y, int w, int h, uint32_t px) */
.globl CSYM(exp_asm_fill_rect_rgba32)
CSYM(exp_asm_fill_rect_rgba32):
    push    %rbx
    push    %r12
    mov     24(%rsp), %r10d
    mov     32(%rsp), %eax
    test    %r9d, %r9d
    jle     .Lrect_done_x64
    test    %r10d, %r10d
    jle     .Lrect_done_x64
    mov     %ecx, %r11d
    add     %r9d, %r11d
    test    %ecx, %ecx
    jge     .Lx0_x64
    xor     %ecx, %ecx
.Lx0_x64:
    cmp     %esi, %r11d
    jle     .Lx1_x64
    mov     %esi, %r11d
.Lx1_x64:
    sub     %ecx, %r11d
    test    %r11d, %r11d
    jle     .Lrect_done_x64
    mov     %r8d, %r12d
    add     %r10d, %r12d
    test    %r8d, %r8d
    jge     .Ly0_x64
    xor     %r8d, %r8d
.Ly0_x64:
    cmp     %edx, %r12d
    jle     .Ly1_x64
    mov     %edx, %r12d
.Ly1_x64:
    sub     %r8d, %r12d
    test    %r12d, %r12d
    jle     .Lrect_done_x64
    movslq  %esi, %rsi
    movslq  %r8d, %r8
    movslq  %ecx, %rcx
    imul    %rsi, %r8
    add     %rcx, %r8
    lea     (%rdi, %r8, 4), %rbx
    shl     $2, %rsi
    cld
.Lrect_row_x64:
    mov     %rbx, %rdi
    mov     %r11d, %ecx
    rep     stosl
    add     %rsi, %rbx
    dec     %r12d
    jnz     .Lrect_row_x64
.Lrect_done_x64:
    pop     %r12
    pop     %rbx
    ret

#endif
