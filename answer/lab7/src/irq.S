.global irq_enable
irq_enable:
    msr daifclr, 0xf
    ret

.global irq_disable
irq_disable:
    msr daifset, 0xf
    ret