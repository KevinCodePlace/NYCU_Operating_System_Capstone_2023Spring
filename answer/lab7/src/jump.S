.global setjump
.global longjump
setjump:
    stp x1, x2, [x0 ,16 * 0]
    stp x3, x4, [x0 ,16 * 1]
    stp x5, x6, [x0 ,16 * 2]
    stp x7, x8, [x0 ,16 * 3]
    stp x9, x10, [x0 ,16 * 4]
    stp x11, x12, [x0 ,16 * 5]
    stp x13, x14, [x0 ,16 * 6]
    stp x15, x16, [x0 ,16 * 7]
    stp x17, x18, [x0 ,16 * 8]
    stp x19, x20, [x0 ,16 * 9]
    stp x21, x22, [x0 ,16 * 10]
    stp x23, x24, [x0 ,16 * 11]
    stp x25, x26, [x0 ,16 * 12]
    stp x27, x28, [x0 ,16 * 13]
    stp x29, x30, [x0 ,16 * 14]
    mov x1, lr
    mov x2, sp
    stp x1, x2, [x0 ,16 * 15]

    ldr x0, =0
    ret

longjump:
    ldp x2, x3, [x0,16 * 15]
    mov sp, x3
    mov lr, x2

    ldp x3, x4, [x0 ,16 * 1]
    ldp x5, x6, [x0 ,16 * 2]
    ldp x7, x8, [x0 ,16 * 3]
    ldp x9, x10, [x0 ,16 * 4]
    ldp x11, x12, [x0 ,16 * 5]
    ldp x13, x14, [x0 ,16 * 6]
    ldp x15, x16, [x0 ,16 * 7]
    ldp x17, x18, [x0 ,16 * 8]
    ldp x19, x20, [x0 ,16 * 9]
    ldp x21, x22, [x0 ,16 * 10]
    ldp x23, x24, [x0 ,16 * 11]
    ldp x25, x26, [x0 ,16 * 12]
    ldp x27, x28, [x0 ,16 * 13]
    ldp x29, x30, [x0 ,16 * 14]

    mov x2, x0
    mov x0, x1
    ldp x1, x2, [x2 , 16 * 0]

    ret


