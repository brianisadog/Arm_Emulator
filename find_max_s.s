.global find_max_s
.func find_max_s

find_max_s:
        ldr r2, [r0, #0]
        mov r3, #1
loop:
        cmp r3, r1
        beq loop_end
        ldr r12, [r0, r3, lsl #2]
        cmp r12, r2
        movgt r2, r12
        add r3, r3, #1
        b loop
loop_end:
        mov r0, r2
        bx lr
