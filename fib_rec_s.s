.global fib_rec_s
.func fib_rec_s

fib_rec_s:
        sub sp, sp, #16
        str lr, [sp]
        cmp r0, #0      /* if (n <= 0) */
        bne base
        mov r0, #0
        b fib_rec_s_end
base:
        cmp r0, #1      /* if (n == 1) */
        bne rec
        b fib_rec_s_end
rec:
        str r0, [sp, #4]
        sub r0, r0, #1
        bl fib_rec_s
        str r0, [sp, #8]
        ldr r1, [sp, #4]
        sub r0, r1, #2
        bl fib_rec_s
        ldr r2, [sp, #8]
        add r0, r2, r0
fib_rec_s_end:
        ldr lr, [sp]
        add sp, sp, #16
        bx lr
