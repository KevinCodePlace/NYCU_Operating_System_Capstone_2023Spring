#define CORE0_TIMER_IRQ_CTRL 0xFFFF000040000040
.global core_timer_enable
.global core_timer_disable


core_timer_enable:
  mov x0, 1
  msr cntp_ctl_el0, x0 // enable
  mrs x0, cntfrq_el0
  msr cntp_tval_el0, x0 // set expired time
  mov x0, 2
  ldr x1, =CORE0_TIMER_IRQ_CTRL
  str w0, [x1] // unmask timer interrupt
  ret

core_timer_disable:
  mov x0, 0
  msr cntp_ctl_el0, x0 // disable
  mov x0, 0
  ldr x1, =CORE0_TIMER_IRQ_CTRL
  str w0, [x1] // unmask timer interrupt
  ret

  