// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>
#include "i8254.h"
#include "mouse.h"

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
	// sets the language of LCF messages (can be either EN-US or PT-PT)
	lcf_set_language("EN-US");

	// enables to log function invocations that are being "wrapped" by LCF
	// [comment this out if you don't want/need/ it]
	lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

	// enables to save the output of printf function calls on a file
	// [comment this out if you don't want/need it]
	lcf_log_output("/home/lcom/labs/lab4/output.txt");

	// handles control over to LCF
	// [LCF handles command line arguments and invokes the right function]
	if (lcf_start(argc, argv))
		return 1;

	// LCF clean up tasks
	// [must be the last statement before return]
	lcf_cleanup();

	return 0;
}


int (mouse_test_packet)(uint32_t cnt) {
	uint8_t bit_no = 0;
	uint32_t counter = cnt * PACKET_SIZE;

	if (mouse_enable_data_report())		// Enables mouse data report
		return 1;

	if (mouse_subscribe_int(&bit_no))   //subscribes mouse interrupt
		return 1; //failure

	int ipc_status, r, irq_set = BIT(bit_no); //irq_set is an integer with the bit specified by the variable bit_no at 1
	message msg;

	while (counter != 0) {
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.m_notify.interrupts & irq_set) {
					mouse_ih();
					if (mouse_data_handler())			// After recieving a byte of a packet process it
						return 1;
					counter--;
					if (Nbyte == 3) {					// After recieving a packet of 3 bytes print it
						mouse_print_packet(&pp);
						Nbyte = 0;
					}
				}
				break;
			default:
				break;
			}
		}
	}

	if (mouse_unsubscribe_int())  //unsubscribes mouse interrupt
		return 1; //failure

	if (mouse_disable_data_report())		// Set mouse data report to default
		return 1;

	return 0; //success
}

int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
	uint8_t Npackets = cnt;
	Nbyte = 0;
	uint8_t default_byte = minix_get_dflt_kbc_cmd_byte();
	default_byte = default_byte & DIS_INT;

	if (mouse_disable_data_report())		// Set mouse data report to default
		return 1;

	if (set_remote_mode())					// Enable mouse remote mode
		return 1;

	if (disable_mouse_ih(default_byte))		// Disable default mouse interrupt handler
		return 1;

	while (Npackets != 0) {
		if (Nbyte == 0)						// If waiting for first byte tell KBC to put the packet in output buffer
			read_data();
		mouse_ih();							// Recieve data from KBC output buffer
		if (mouse_data_handler())			// Process packet
			return 1;
		if (Nbyte == 3) {
			mouse_print_packet(&pp);		// After getting 3 byte packet print it
			Nbyte = 0;
			Npackets--;
			tickdelay(micros_to_ticks(period * 1000));			// Wait for next packet
		}
	}

	if (set_stream_mode())					// Set mouse to default stream mode
		return 1;

	if (mouse_disable_data_report())		// Set mouse data report to default
		return 1;

	if (enable_mouse_ih())					// Enable default mouse interrupt handler
		return 1;

	return 1;
}

int (mouse_test_async)(uint8_t idle_time) {
	uint8_t bit_no_mouse = 0;
	uint8_t bit_no_timer = 1;

	if (mouse_enable_data_report())				// Enable mouse data report
		return 1;

	if (mouse_subscribe_int(&bit_no_mouse))   //subscribes mouse interrupt
		return 1; //failure

	if (timer_subscribe_int(&bit_no_timer)) //subscribes Timer 0 interrupt
		return 1;

	int ipc_status, r;
	int irq_set_mouse = BIT(bit_no_mouse); 
	int irq_set_timer = BIT(bit_no_timer);
	message msg;

	while ((timer_count / sys_hz()) < idle_time) {						// Stops if enough time has passed
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.m_notify.interrupts & irq_set_mouse) {
					timer_count = 0;
					mouse_ih();											// Recieve data from KBC
					if (mouse_data_handler())							// Process data
						return 1;
					if (Nbyte == 3) {
						mouse_print_packet(&pp);						// Print packet of 3 bytes
						Nbyte = 0;
					}
				}
				if (msg.m_notify.interrupts & irq_set_timer) {
					timer_int_handler();								 //increment counter (60 times per second)
				}
				break;
			default:
				break;
			}
		}
	}

	if (timer_unsubscribe_int()) //unsubscribes Timer 0 interrupt
		return 1;

	if (mouse_unsubscribe_int())  //unsubscribes mouse interrupt
		return 1; //failure

	if (mouse_disable_data_report())		// Set mouse data report to default
		return 1;

	return 0; //success
}

int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
	uint8_t bit_no = 0;
	bool finished = false;
	bool started = false;
	bool second_mov = false;
	uint16_t mov_x = 0, mov_y = 0;

	if (mouse_enable_data_report())		// Enable mouse data report
		return 1;

	if (mouse_subscribe_int(&bit_no))   //subscribes mouse interrupt
		return 1; //failure

	int ipc_status, r, irq_set = BIT(bit_no); //irq_set is an integer with the bit specified by the variable bit_no at 1
	message msg;

	while (!finished) 
	{
		if ((r = driver_receive(ANY, &msg, &ipc_status)) != OK) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.m_notify.interrupts & irq_set) {
					mouse_ih();								// Recieve data from mouse
					if (mouse_data_handler())				// Process bytes from packet
						return 1;

					if (Nbyte == 3) {
						mouse_print_packet(&pp);			// Print 3 byte packet

						if (!second_mov)					// If second movement has not started check if first movement started or 
															// if it started, check if it is valid
															// Also check if first movement has ended
							left_mov(&started, &second_mov, tolerance, &mov_x, &mov_y, x_len);
						
						//Start of second movement
						if (second_mov)						// If second movement started, check if it is valid or if it ended
							right_mov(&started, &second_mov, tolerance, &mov_x, &mov_y, x_len, &finished);

						Nbyte = 0;
					}
				}
				break;
			default:
				break;
			}
		}
	}

	if (mouse_unsubscribe_int())  //unsubscribes mouse interrupt
		return 1; //failure

	if (mouse_disable_data_report())		// Set mouse data report to default
		return 1;

	return 0;
}
