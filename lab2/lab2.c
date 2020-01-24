#include <lcom/lcf.h>

#include <lcom/lab2.h>
#include <lcom/timer.h>

#include <stdbool.h>
#include <stdint.h>
#include "i8254.h"

extern uint32_t counter; //global variable counter

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {
	//Read and display configuration
	uint8_t var = 0;
	uint8_t *st = &var;

	if(timer_get_conf(timer, st))
		return 1; //failure

	if(timer_display_conf(timer, *st, field))
		return 1; //failure
	return 0; //success
}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) {
	//Change timer's frequency
	if(timer_set_frequency(timer, freq)) 
		return 1; //failed
	return 0; //success
}

int(timer_test_int)(uint8_t time) {
	
	if(timer_set_frequency(0, sys_hz())) //setting timer0's frequency to 60Hz
		return 1; //failure

	int r;
	uint8_t bit_no;
	if(timer_subscribe_int(&bit_no)) 
		return 1; //failure
	int ipc_status, irq_set = BIT(bit_no); //irq_set is an integer with the bit specified by the variable bit_no at 1
	message msg; 

	while( counter < time*sys_hz() )  //while the time interval (given by argument) has not elapsed
	{
		if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
			printf("driver_receive failed with: %d", r); 
		continue;
		}
		if (is_ipc_notify(ipc_status)) {
			if (_ENDPOINT_P(msg.m_source) == HARDWARE)
				if (msg.m_notify.interrupts & irq_set) {
					timer_int_handler();               //increment counter (60 times per second)
					if(!(counter % sys_hz()))          //print message at each second
						timer_print_elapsed_time();
				} 
		}
	}
	
		
	if(timer_unsubscribe_int())
		return 1; //failure
	return 0; //success
}

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
	//Get 8 less significant bits from a 16-bit variable (val)

	uint8_t tempLSB = 0;
	uint16_t tempVal = 0;	

	tempVal = val << 8;        
	tempLSB = tempVal >> 8;	   
	
	*lsb = tempLSB; //in order to pass LSB through the function's input argument *lsb;

  return 0; 
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
	//Get 8 most significant bits from a 16-bit variable (val)

	uint8_t tempMSB = 0;	
	
	tempMSB = val >> 8;	
	
	*msb = tempMSB; //in order to pass MSB through the function's input argument *msb;

  return 0;
}
