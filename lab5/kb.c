#include <lcom/lcf.h>
#include <stdint.h>
#include "kb.h"
#include <minix/sysutil.h>
#include <machine/asm.h>

//Global variables
uint32_t data;
int hook2 = 4;
uint32_t counter1;
bool second_code = false;
uint8_t codes[2];
uint8_t size = 1;

int sys_inb_cnt(int port, uint32_t *byte) {       //sys_inb() but with a counter increase
	counter1++;
	return sys_inb(port, byte);
}

void (kbc_ih)() {

	uint32_t stat;
	data = 0;

	if (sys_inb_cnt(STAT_REG, &stat) != OK) {						// Try to read status and check if any error
		return;
	}

	if ((uint8_t)stat & OBF) {									    // Check if Out_Buffer full
		if (sys_inb_cnt(OUT_BUF, &data) != OK) { 				    // Try to read data inside Out_Buffer
			data = 0;												// Resets data
			return;
		}

		if ((uint8_t)stat & (PAR_ERR | TO_ERR)) {			// Check for Parity or Timeout Error
			data = 0;
			return;
		}
	}
}

int (kb_subscribe_int)(uint8_t *bit_no) {
	*bit_no = hook2;
	if (sys_irqsetpolicy(IRQ1, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook2) != OK)
		return 1; //failed
	return 0; //success
}

int (kb_unsubscribe_int)() {
	int hook_id = hook2;
	if (sys_irqrmpolicy(&hook_id) != OK)
		return 1; //failure
	return 0; //success
}

void kbd_data_handler()
{
	bool makecode = true;

	if (second_code) {	// code after 0xE0
		codes[1] = data;
		if (data & MK_OR_BRK)
			makecode = false;
		//Print Makecode/Breakcode
		if (kbd_print_scancode(makecode, size, codes))	//print failed
			return;
		//Reset global variables
		codes[0] = 0; codes[1] = 0;
		size = 1;
		second_code = false;
		return;
	}

	if (data == TWO_LONG) {
		//Prepare for second byte
		codes[0] = data;
		second_code = true;
		size = 2;
		return;
	}

	if (data != TWO_LONG && !second_code) {
		//Set variables
		codes[0] = data;
		size = 1;
		if (data & MK_OR_BRK)
			makecode = false;
		//Print Makecode/Breakcode
		if (kbd_print_scancode(makecode, size, codes))	//print failed
			return;
		//Reset global variables
		codes[0] = 0; codes[1] = 0;
		size = 1;
		return;
	}
	return;
}

int (util_get_LSB)(uint16_t val, uint8_t *lsb) {
	//Get 8 less significant bits from a 16-bit variable (val)

	uint8_t tempLSB = 0;
	uint16_t tempVal = 0;

	tempVal = val << 8;
	tempLSB = tempVal >> 8;

	*lsb = tempLSB; //in order to pass LSB through the function's input argument *lsb;

	return 0;
}

int (util_get_MSB)(uint16_t val, uint8_t *msb) {
	//Get 8 most significant bits from a 16-bit variable (val)

	uint8_t tempMSB = 0;

	tempMSB = val >> 8;

	*msb = tempMSB; //in order to pass MSB through the function's input argument *msb;

	return 0;
}

int enable_interrupts()
{
	uint32_t stat;

	if (sys_outb(STAT_REG, READ_CMD))   //Sends a read command to status register
		return 1; //failed

	if (sys_inb_cnt(OUT_BUF, &stat))    //Passes the value on the output buffer to a variable
		return 1; //failed

	stat = stat | BIT(0);               //Changes this variable so that its first bit would be 1
	if (sys_outb(STAT_REG, WRT_CMD))    //Sends a write command to status register
		return 1; //failed

	if (sys_outb(OUT_BUF, stat))        //Passes the value on the output buffer to the previous variable
		return 1; //failed

	return 0; //success
}
