.global getpid1
getpid1:
    mov x8, #0
    svc #0
    ret

.global uart_read1
uart_read1:
    mov x8, #1
    svc #0
    ret

.global uart_write1
uart_write1:
    mov x8, #2
    svc #0
    ret

.global exec1
exec1:
    mov x8, #3
    svc #0
    ret

.global fork1
fork1:
    mov x8, #4
    svc #0
    ret

.global exit1
exit1:
    mov x8, #5
    svc #0
    ret

.global mbox_call1
mbox_call1:
    mov x8, #6
    svc #0
    ret

.global kill1
kill1:
    mov x8, #7
    svc #0
    ret


