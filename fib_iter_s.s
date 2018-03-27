.global fib_iter_s
.func fib_iter_s

fib_iter_s:
        cmp r0, #0      /* if (n <= 0) */
        ble return
        mov r1, #0      /* int temp = 0 */
        mov r2, #0      /* int a = 0 */
        mov r3, #1      /* int b = 1 */
        mov r12, #2     /* int i = 2 */
loop:
        cmp r12, r0
        bgt loop_end
        add r1, r2, r3
        mov r2, r3
        mov r3, r1
        add r12, r12, #1
        b loop
loop_end:
        mov r0, r3
return:
        bx lr
