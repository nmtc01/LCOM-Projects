#include <lcom/lcf.h>
#include <minix/sysutil.h>
#include "kb.h"
#include "i8254.h"
#include <lcom/timer.h>

//Global variables
uint32_t data;
uint32_t counter1;

int main(int argc, char *argv[]) {
	// sets the language of LCF messages (can be either EN-US or PT-PT)
	lcf_set_language("EN-US");

	// enables to log function invocations that are being "wrapped" by LCF
	// [comment this out if you don't want/need/ it]
	lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

	// enables to save the output of printf function calls on a file
	// [comment this out if you don't want/need it]
	lcf_log_output("/home/lcom/labs/lab3/output.txt");

	// handles control over to LCF
	// [LCF handles command line arguments and invokes the right function]
	if (lcf_start(argc, argv))
		return 1;

	// LCF clean up tasks
	// [must be the last statement before return]
	lcf_cleanup();

	return 0;
}

int (kbd_test_scan)(bool assembly) {
	uint8_t bit_no = 1;

	if (kb_subscribe_int(&bit_no))   //subscribes keyboard interrupt
		return 1; //failure

	int ipc_status, r, irq_set = BIT(bit_no); //irq_set is an integer with the bit specified by the variable bit_no at 1
	message msg;

	while (data != ESC_BREAK) {  //while escape key is not pressed
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.m_notify.interrupts & irq_set) {
					if (assembly) {                            //to distinguish between interrupt handler in assembly and not
						kbc_asm_ih();
						data = code;						   //code has the same functionality as data in the assembly mode
						if (data != 0)						   //data is passed by the variable code in assembly
							kbd_data_handler();
					}
					else {
						kbc_ih();
						if (data != 0)
							kbd_data_handler();
					}
				}
				break;
			default:
				break;
			}
		}
	}

	if (kb_unsubscribe_int())  //unsubscribes keyboard interrupt
		return 1; //failure
	if (!assembly)
		if (kbd_print_no_sysinb(counter1))  //prints number of sys_inb() calls
			return 1; //failure
	return 0; //success
}

int (kbd_test_poll)() {  //same functionality of kbd_test_scan, but with polling, not with interrupts
	while (data != ESC_BREAK) { //while escape key is not pressed
		kbc_ih();
		if (data != 0)
			kbd_data_handler();
		tickdelay(micros_to_ticks(DELAY_US));
	}

	if (enable_interrupts()) //enables interrupts so that the keyboard can be used again
		return 1;

	if (kbd_print_no_sysinb(counter1))  //prints number of sys_inb() calls
		return 1;

	return 0; //success
}

int (kbd_test_timed_scan)(uint8_t n) {
	uint8_t bit_kb = 1;
	uint8_t bit_timer = 1;

	if (kb_subscribe_int(&bit_kb)) //subscribes keyboard interrupt
		return 1;

	if (timer_subscribe_int(&bit_timer)) //subscribes Timer 0 interrupt
		return 1;

	int ipc_status, r;
	int irq_set_kb = BIT(bit_kb);
	int irq_set_timer = BIT(bit_timer);
	message msg;

	while ((data != ESC_BREAK) && ((counter / sys_hz()) != n)) {
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				//if to distinguish between keyboard interrupts and Timer 0 interrupts
				if (msg.m_notify.interrupts & irq_set_kb) {
					kbc_ih();
					if (data != 0)
						kbd_data_handler();
					counter = 0;
				}
				if (msg.m_notify.interrupts & irq_set_timer) {
					timer_int_handler();               //increment counter (60 times per second)
				}
				break;
			default:
				break;
			}
		}
	}

	if (kb_unsubscribe_int()) //unsubscribes keyboard interrupt
		return 1;

	if (timer_unsubscribe_int()) //unsubscribes Timer 0 interrupt
		return 1;

	if (enable_interrupts()) //enables interrupts so that the keyboard can be used again
		return 1;

	return 0;
}


