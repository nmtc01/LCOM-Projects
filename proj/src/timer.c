#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

uint32_t counter = 0; //initializing counter to 0
int hook = 2; //declaring and initializing hook to 2

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
	//Changes timer's (1st argument) frequency with freq

	if (freq < 19 || freq > TIMER_FREQ)    //in case of overflow
		return 1;

	uint8_t st;
	if (timer < 0 || timer > 2 || timer_get_conf(timer, &st))   //if invalid timer or timer_get_conf() fail
		return 1;

	//Making a control word in order to configure the timer:
	uint8_t control = st & (BIT(3) | BIT(2) | BIT(1) | BIT(0)); // bitwise AND between the configuration given by timer_get_conf (st) and 00001111
																// in order to put control's MSB = 0x0, leaving control's LSB equal to st's
	control = control | TIMER_LSB_MSB; //Initialization Mode: control's bits 4 and 5 equal to 1
	//Timer's selection
	if (timer == 0)
		control = control | TIMER_SEL0;
	else if (timer == 1)
		control = control | TIMER_SEL1;
	else control = control | TIMER_SEL2;

	sys_outb(TIMER_CTRL, control); //Launching word 'control' to the controller;

	uint16_t div = TIMER_FREQ / freq; //getting counter value (div)
	uint8_t LSB, MSB;

	//Split of div into LSB and MSB
	util_get_LSB(div, &LSB);
	util_get_MSB(div, &MSB);

	//Launch of div into the corresponding timer (given by argument)
	if (timer == 0) {
		sys_outb(TIMER_0, LSB);
		sys_outb(TIMER_0, MSB);
	}
	else if (timer == 1) {
		sys_outb(TIMER_1, LSB);
		sys_outb(TIMER_1, MSB);
	}
	else {
		sys_outb(TIMER_2, LSB);
		sys_outb(TIMER_2, MSB);
	}

	return 0; //success
}

int (timer_subscribe_int)(uint8_t *bit_no) {
	//Subscribes and enables Timer 0 interrupts.

	*bit_no = hook; //passing hook's value through the function's input argument
	if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook) != OK) //returns OK on success and 3 other values on failure
																 //subscribes a notification (on every interrupt in the input TIMER0_IRQ)
		return 1; //failed

	return 0; //success
}

int (timer_unsubscribe_int)() {
	if (sys_irqrmpolicy(&hook) != OK) //returns OK on success and 3 other values on failure
									  //unsubscribes a previous subscription of the interrupt notification associated with the specified argument &hook
		return 1; //failure
	return 0; //success
}

void (timer_int_handler)() {
	counter++; //incrementing counter (at each interrupt)
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {

	uint32_t stTemp; //temporary variable

	/*
		1.Timer's selection;
		2.Making a Read-Back command word in order to read a timer's configuration (variable controln, where n is the number of the timer);
		3.Launching that word into the controller (sys_outb() fucntion);
		4.Receiving configuration due to the function sys_inb() -> configuration passed to stTemp;
		5.*st = stTemp, in order to pass the configuration through the function's input argument *st;
		6. return 0 -> success.
	*/

	//Timer 0
	if (timer == 0) {
		uint32_t control0 = (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer)); 
		sys_outb(TIMER_CTRL, control0);
		sys_inb(TIMER_0, &stTemp);
		*st = stTemp;
		return 0;
	}

	//Timer 1
	if (timer == 1) {
		uint32_t control1 = (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer));
		sys_outb(TIMER_CTRL, control1);
		sys_inb(TIMER_1, &stTemp);
		*st = stTemp;
		return 0;
	}

	//Timer 2
	if (timer == 2) {
		uint32_t control2 = (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer));
		sys_outb(TIMER_CTRL, control2);
		sys_inb(TIMER_2, &stTemp);
		*st = stTemp;
		return 0;
	}

	return 1; //failure
}

int (timer_display_conf)(uint8_t timer, uint8_t conf,
	enum timer_status_field field) {

	uint8_t word;
	union timer_status_field_val config;

	if (timer < 0 || timer > 2)  //if invalid timer 
		return 1; 

	//Choosing wich field to print
	//all
	if (field == 0)
		config.byte = conf; //using all control word's bits
	//init_mode
	else if (field == 1) {
		word = conf << 2;
		word = word >> 6;
		config.in_mode = word; //using bits 4 and 5
	}
	//count_mode
	else if (field == 2) {
		word = conf << 4;
		word = word >> 5;
		/* If counting mode = 2 or 3, to prevent errors caused by these cases: 110, 111 -> subtract 4.
		Results of the subtraction: 010, 011 -> don't cause errors and this subtraction doesn't chenge the counting mode. */
		if (word >= 6)
			word -= 4; 
		config.count_mode = word; //using bits 1, 2 and 3
	}
	//binary or bcd
	else if (field == 3) {
		word = conf << 7;
		word = word >> 7;
		config.bcd = word; //using bit 0
	}
	else return 1; //failure

	timer_print_config(timer, field, config); //function provided that prints the configuration in a human friendly way
	return 0; //success
}
