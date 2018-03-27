.global find_str_s
.func find_str_s

find_str_s:
        sub sp, sp, #16
        ldrb r2, [r1]           /* char start_of_sub = *sub */
        str r2, [sp]            /* put start_of_sub into stack #0 */
        str r1, [sp, #4]        /* put sub into stack #4 */
        mov r2, #-1             /* int result = -1 */
        str r2, [sp, #8]        /* put result into stack #8 */
        mov r12, #0             /* int i = 0 */
outer_loop:
        ldr r2, [sp, #8]
        cmp r2, #-1             /* if (result != -1) */
        bne outer_loop_end
        ldrb r2, [r0]
        cmp r2, #0              /* if (*s == '\0') */
        beq outer_loop_end
        ldr r3, [sp]
        cmp r2, r3              /* if (*s != start_of_sub) */
        bne not_match
        ldr r1, [sp, #4]        /* char *pointer_sub = sub */
        str r12, [sp, #12]      /* j = i and put j into stack #12 */
inner_loop:
        ldrb r2, [r0]           /* set r2 to *s */
        ldrb r3, [r1]           /* set r3 to *pointer_sub */
        cmp r2, r3              /* if (*s != *pointer_sub) */
        bne inner_loop_end
        cmp r3, #0              /* if (*pointer_sub == '\0') */
        beq inner_loop_end
        add r0, r0, #1          
        add r1, r1, #1
        add r12, r12, #1
        b inner_loop
inner_loop_end:
        ldr r2, [sp, #12]       /* read j from stack #12 into r2 */
        cmp r3, #0              /* if (*pointer_sub == '\0') */
        bne not_end_of_sub
        str r2, [sp, #8]        /* update result on stack #8 with j */
        b outer_loop_end
not_end_of_sub:
        sub r0, r0, #1
        sub r12, r12, #1
not_match:
        add r0, r0, #1
        add r12, r12, #1
        b outer_loop
outer_loop_end:
        ldr r0, [sp, #8]        /* set r0 to result on stack #8 */
        add sp, sp, #16
        bx lr
