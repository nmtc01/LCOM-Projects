#include <machine/asm.h>
#include "lab3.h"

.global _C_LABEL(code)
.global _C_LABEL(kbc_asm_ih)

.data
_C_LABEL(code): 
	.byte 0

.text
_C_LABEL(kbc_asm_ih) :	
	inb $STAT_REG              // read status 
	test $OBF, %al             // check if Out_Buffer full
	jz  FIM                    // if not
	mov %al, %bl	           // make a copy of al
	inb $OUT_BUF               // try to read data inside Out_Buffer
		
	test $TO_ERR, %bl		   // check for Timeout Error
	jnz  ZERO				   // if there is an error
	test $PAR_ERR, %bl         // check for Parity Error
	jnz  ZERO                  // if there is an error
	 
	movb %al, _C_LABEL(code)   // data is passed by the variable code
	jmp FIM
ZERO:
	xorb %al, %al              // reset data
	movb %al, _C_LABEL(code)   // data is passed by the variable code
FIM:
	ret
