.global branchAddr
branchAddr:
   br x0 
   

.globl get_el
get_el:
   mrs x0, CurrentEl
   lsr x0, x0, #2
   ret

