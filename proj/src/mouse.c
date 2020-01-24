#include <lcom/lcf.h>
#include "mouse.h"

//Global variables
int hook_mouse = 5;
uint32_t dados = 0;
int Nbyte = 0;
struct packet pp;
bool x_sign;
bool y_sign;

int mouse_subscribe_int(uint8_t *bit_no) {
	*bit_no = hook_mouse;
	if (sys_irqsetpolicy(IRQ12, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_mouse) != OK)
		return 1; //failed
	return 0; //success
}

int mouse_unsubscribe_int() {
	int hook_id = hook_mouse;
	if (sys_irqrmpolicy(&hook_id) != OK)
		return 1; //failure
	return 0; //success
}

int mouse_enable_data_report() {
	uint32_t stat;
	uint32_t ack = 0;

	do {
		if (sys_outb(KBC_COMMAND_PORT, WRT_BYTE_TO_MOUSE) != OK)    //Sends the command "Write Byte To Mouse"
			return 1;

		if (sys_inb(STAT_REG, &stat) != OK) 						//Tries to read status and check if any error		
			return 1;

		if ((uint8_t)stat & IBF)									//Checks if input buffer is full
			continue;

		if (sys_outb(IN_BUF, ENABLE_DATA) != OK)					//Enables dados reporting
			return 1; 

		if (sys_inb(OUT_BUF, &ack) != OK)							//Checks acknowlodgement from the mouse
			return 1;
	} while (ack == NACK);

	if (ack == ERROR)
		return 1;

	return 0;
}

void (mouse_ih)() {
	uint32_t stat;

	if (sys_inb(STAT_REG, &stat) != OK) {						//Tries to read status and check if any error
		return;
	}

	if ((uint8_t)stat & (OBF | AUX)) {							//Checks if Out_Buffer is full and that dados comes from the mouse
		if (sys_inb(OUT_BUF, &dados) != OK) { 				    //Tries to read dados inside Out_Buffer
			dados = 0;											//Resets dados
			return;
		}

		if ((uint8_t)stat & (PAR_ERR | TO_ERR)) {				//Checks for Parity or Timeout Error
			dados = 0;
			return;
		}
	}
}

int mouse_data_handler(){
	if (Nbyte == 0 && !((uint8_t)dados & THIRD))						//Checks if bit 3 is set to 1
		return 1;

	pp.bytes[Nbyte] = (uint8_t)dados;									//Passes dados-bytes to struct packet pp

	if (Nbyte == 0)														//Parses first byte
	{
		pp.rb = ((uint8_t)dados & RB);
		pp.mb = ((uint8_t)dados & MB);
		pp.lb = ((uint8_t)dados & LB);
		x_sign = ((uint8_t)dados & X_SIGN);
		y_sign = ((uint8_t)dados & Y_SIGN);
		pp.x_ov = ((uint8_t)dados & X_OV);
		pp.y_ov = ((uint8_t)dados & Y_OV);
	}

	if (Nbyte == 1)														//Checks signal of x
	{
		if (x_sign)
			pp.delta_x = SIGN_EXTEND | (uint16_t)dados;
		else pp.delta_x = dados;
	}

	if (Nbyte == 2)														//Checks signal of y
	{
		if (y_sign)
			pp.delta_y = SIGN_EXTEND | (uint16_t)dados;
		else pp.delta_y = dados;
	}

	Nbyte++;															//increments Nbyte

	return 0;
}

int mouse_disable_data_report(){
	uint32_t stat;
	uint32_t ack = 0;

	do {
		if (sys_outb(KBC_COMMAND_PORT, WRT_BYTE_TO_MOUSE) != OK)	//Sends the command "Write Byte To Mouse"
			return 1;

		if (sys_inb(STAT_REG, &stat) != OK) 						//Tries to read status and check if any error
			return 1;

		if ((uint8_t)stat & IBF)									//Checks if input buffer is full
			continue;

		if (sys_outb(IN_BUF, DISABLE_DATA) != OK)					//Disables dados reporting
			return 1; //failed

		if (sys_inb(OUT_BUF, &ack) != OK)							//Checks acknowlodgement from the mouse
			return 1;
	} while (ack == NACK);

	if (ack == ERROR)
		return 1;
	return 0;
}

int disable_mouse_ih(uint8_t default_byte){
	uint32_t stat;

	if (sys_outb(KBC_COMMAND_PORT, WRT_COMMAND_BYTE) != OK)			//Sends the command "Write Byte To Mouse"
		return 1;
	do {
		if (sys_inb(STAT_REG, &stat) != OK) 						//Tries to read status and check if any error
			return 1;
	} while (((uint8_t)stat & IBF));								//Checks if input buffer is full

	if (sys_outb(IN_BUF, default_byte) != OK)						//Sends to input buffer the KBC command byte default value
		return 1; 

	return 0;
}

int set_remote_mode(){
	uint32_t stat;
	uint32_t ack = 0;

	do {
		if (sys_outb(KBC_COMMAND_PORT, WRT_BYTE_TO_MOUSE) != OK)	//Sends the command "Write Byte To Mouse"
			return 1;

		if (sys_inb(STAT_REG, &stat) != OK) 						//Tries to read status and check if any error		
			return 1;

		if ((uint8_t)stat & IBF)									//Checks if input buffer is full
			continue;

		if (sys_outb(IN_BUF, SET_REMOTE) != OK)						//Sets remote mode
			return 1; //failed

		if (sys_inb(OUT_BUF, &ack) != OK)							//Checks acknowlodgement from the mouse
			return 1;
	} while (ack == NACK);

	if (ack == ERROR)
		return 1;

	return 0;
}

int set_stream_mode(){
	uint32_t stat;
	uint32_t ack = 0;

	do {
		if (sys_outb(KBC_COMMAND_PORT, WRT_BYTE_TO_MOUSE) != OK)	//Sends the command "Write Byte To Mouse"
			return 1;

		if (sys_inb(STAT_REG, &stat) != OK) 						//Tries to read status and check if any error		
			return 1;

		if ((uint8_t)stat & IBF)									//Checks if input buffer is full
			continue;

		if (sys_outb(IN_BUF, SET_STREAM) != OK)						//Sets stream mode
			return 1; //failed

		if (sys_inb(OUT_BUF, &ack) != OK)							//Checks acknowlodgement from the mouse
			return 1;
	} while (ack == NACK);

	if (ack == ERROR)
		return 1;

	return 0;
}

int enable_mouse_ih(){
	uint32_t stat;
	uint8_t default_byte = minix_get_dflt_kbc_cmd_byte();

	if (sys_outb(KBC_COMMAND_PORT, WRT_COMMAND_BYTE) != OK)			//Sends the command "Write Command Byte"
		return 1;
	do {
		if (sys_inb(STAT_REG, &stat) != OK) 						//Tries to read status and check if any error
			return 1;
	} while (((uint8_t)stat & IBF));								//Checks if input buffer is full

	if (sys_outb(IN_BUF, default_byte) != OK)						//Sends to input buffer the KBC command byte default value
		return 1; //failed

	return 0;
}

int read_data(){
	uint32_t stat;
	uint32_t ack = 0;

	do {
		if (sys_outb(KBC_COMMAND_PORT, WRT_BYTE_TO_MOUSE) != OK)	//Sends the command "Write Byte To Mouse"
			return 1;

		if (sys_inb(STAT_REG, &stat) != OK) 						//Tries to read status and check if any error		
			return 1;

		if ((uint8_t)stat & IBF)									//Checks if input buffer is full
			continue;

		if (sys_outb(IN_BUF, READ_DATA) != OK)						//Reads dados
			return 1; //failed

		if (sys_inb(OUT_BUF, &ack) != OK)							//Checks acknowlodgement from the mouse
			return 1;
	} while (ack == NACK);

	if (ack == ERROR)
		return 1;

	return 0;
}

bool is_no_button_pressed(){				//verify if no buttons are being pressed
	return (!pp.lb && !pp.mb && !pp.rb);
}

bool is_only_left_pressed(){				//verify if only LMB is being pressed
	return (pp.lb && !pp.mb && !pp.rb);
}

bool is_only_right_pressed(){				//verify if only RMB is being pressed
	return (!pp.lb && !pp.mb && pp.rb);
}

bool draw_left(uint8_t tolerance)
{
	return ((pp.delta_x < 0 && abs(pp.delta_x) < tolerance) ||			//Checks if x is inside the tolerance
		(pp.delta_x >= 0 && pp.delta_y >= 0) ||							//Checks if x and y is not negative
		(pp.delta_y < 0 && abs(pp.delta_y) < tolerance));				//Checks if y is inside the tolerance
}

bool draw_right(uint8_t tolerance)
{
	return ((pp.delta_x < 0 && abs(pp.delta_x) < tolerance) ||			//Checks if x is inside the tolerance
		(pp.delta_x >= 0 && pp.delta_y <= 0) ||							//Checks if x is not negative and y is not positive
		(pp.delta_y > 0 && abs(pp.delta_y) < tolerance));				//Checks if y is inside the tolerance
}

bool slope(uint16_t delta_x, uint16_t delta_y){		//verify if mouse movement makes a slope bigger than 1 (absolute value)
	int m = abs(delta_y / delta_x);
	return (m > 1);
}

bool is_deltax_enough(uint8_t x_len, uint16_t delta_x){		//verify if the change in x is enough for gesture
	return ( delta_x >= x_len );
}

int left_mov(bool *started, bool *second_mov, uint8_t tolerance, uint16_t *mov_x, uint16_t *mov_y, uint8_t x_len)
{
	if (!*started)															//initiate movement
		if (is_no_button_pressed() || is_only_left_pressed())
			*started = true;

	if (*started && is_only_left_pressed() && draw_left(tolerance))			//movement from down_left to up_right
	{
		*mov_x += pp.delta_x;
		*mov_y += pp.delta_y;
	}
	else if (!is_no_button_pressed())										//another random button pressed
	{
		*started = false;
		*mov_x = 0;
		*mov_y = 0;
	}

	if (*started && is_no_button_pressed() && *mov_x != 0)					//finish of fisrt movement (no buttons pressed)
	{
		if (slope(*mov_x, *mov_y) && is_deltax_enough(x_len, *mov_x))		//check slope and x_len
			*second_mov = true;
		else *started = false;
		*mov_x = 0;
		*mov_y = 0;
	}

	return 0;
}

int right_mov(bool *started, bool *second_mov, uint8_t tolerance, uint16_t *mov_x, uint16_t *mov_y, uint8_t x_len, bool *finished)
{
	if (*started && is_only_left_pressed())									//special case, where left movement is restarted		
	{
		*second_mov = false;
		left_mov(started, second_mov, tolerance, mov_x, mov_y, x_len);
	}

	if (*started && is_only_right_pressed() && draw_right(tolerance))		//movement from up_left to down_right
	{
		*mov_x += pp.delta_x;
		*mov_y += pp.delta_y;
	}
	else if (!is_no_button_pressed())										//another random button pressed
	{
		*started = false;
		*second_mov = false;
		*mov_x = 0;
		*mov_y = 0;
	}

	if (*started && is_no_button_pressed() && *mov_x != 0)					//finish of second movement (no buttons pressed)
	{
		if (slope(*mov_x, *mov_y) && is_deltax_enough(x_len, *mov_x))		//check slope and x_len
			*finished = true;
		else {
			*started = false;
			*second_mov = false;
		}
		*mov_x = 0;
		*mov_y = 0;
	}

	return 0;
}













