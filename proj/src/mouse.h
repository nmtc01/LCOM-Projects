#pragma once

/** @defgroup mouse mouse
* @{
*
*/

#include <lcom/lcf.h>
#include <stdint.h>
#include "i8254.h"

//Macros
#define IRQ12				12
#define STAT_REG			0x64
#define IBF					0x02
#define KBC_COMMAND_PORT	0x64
#define WRT_BYTE_TO_MOUSE	0xD4
#define WRT_COMMAND_BYTE	0x60
#define OUT_BUF				0x60
#define IN_BUF				0x60
#define ENABLE_DATA			0xF4
#define DISABLE_DATA		0xF5
#define READ_DATA			0xEB
#define ACK					0xFA
#define NACK				0xFE
#define ERROR				0xFC
#define OBF					0x01
#define AUX					0x20
#define PAR_ERR				0x80
#define TO_ERR				0x40
#define PACKET_SIZE			3
#define	SIGN_EXTEND		    0xFF00
#define DIS_INT				0xFD
#define SET_REMOTE			0xF0
#define SET_STREAM			0xEA


#define LB					0x01
#define RB					0x02
#define MB					0x04
#define THIRD				0x08
#define X_SIGN				0x10
#define Y_SIGN				0x20
#define X_OV				0x40
#define Y_OV				0x80

//Global Variables
extern int hook_mouse;
extern int Nbyte;
extern struct packet pp;
extern uint32_t dados;
extern uint32_t timer_count;

/**
* @brief Subscribes and enables Mouse interrupts
*
* @param bit_no address of memory to be initialized with the
*        bit number to be set in the mask returned upon an interrupt
* @return Return 0 upon success and non-zero otherwise
*/
int mouse_subscribe_int(uint8_t *bit_no);

/**
* @brief Unsubscribes Mouse interrupts
*
* @return Return 0 upon success and non-zero otherwise
*/
int mouse_unsubscribe_int();							

/**
* @brief Enables Mouse data report
*
* @return Return 0 upon success and non-zero otherwise
*/
int mouse_enable_data_report();

/**
* @brief Recieves data from mouse and put it into packets
*
* @return Return 0 upon success
*/
int mouse_data_handler();

/**
* @brief Disables Mouse data report
*
* @return Return 0 upon success and non-zero otherwise
*/
int mouse_disable_data_report();

/**
* @brief Disables default Mouse interrupts
*
* @param default_byte KBC command byte default value
*
* @return Return 0 upon success and non-zero otherwise
*/
int disable_mouse_ih(uint8_t default_byte);

/**
* @brief Set Mouse to remote mode
*
* @return Return 0 upon success and non-zero otherwise
*/
int set_remote_mode();

/**
* @brief Set Mouse to stream mode
*
* @return Return 0 upon success and non-zero otherwise
*/
int set_stream_mode();

/**
* @brief Enable default Mouse interrupts
*
* @return Return 0 upon success and non-zero otherwise
*/
int enable_mouse_ih();

/**
* @brief Get data from KBC output buffer
*
* @return Return 0 upon success and non-zero otherwise
*/
int read_data();

/**
* @brief Check if there are no mouse buttons pressed
*
* @return Return true upon success and false otherwise
*/
bool is_no_button_pressed();

/**
* @brief Check if only left mouse button is being pressed
*
* @return Return true upon success and false otherwise
*/
bool is_only_left_pressed();

/**
* @brief Check if only right mouse button is being pressed
*
* @return Return true upon success and false otherwise
*/
bool is_only_right_pressed();

/**
* @brief Check if mouse movement forms a diagonal line from down-left to up-right and if movement is inside tolerance
*
* @param tolerance maximum value mouse can move outside of pretended direction 
*
* @return Return true upon success and false otherwise
*/
bool draw_left(uint8_t tolerance);

/**
* @brief Check if mouse movement forms a diagonal line from up-left to down-right and if movement is inside tolerance
*
* @param tolerance maximum value mouse can move outside of pretended direction
*
* @return Return true upon success and false otherwise
*/
bool draw_right(uint8_t tolerance);

/**
* @brief Calculates slope of given line and checks if it is positive
*
* @param delta_x total diference in x axis between starter and final point
*		 delta_y total diference in y axis between starter and final point
*
* @return Return true if slope is positive and false otherwise
*/
bool slope(uint16_t delta_x, uint16_t delta_y);

/**
* @brief Check if mouse movement in x axis is greater or equal to given length
*
* @param delta_x total diference in x axis between starter and final point
*		 delta_y total diference in y axis between starter and final point
*
* @return Return true if delta_x >= x_len and false otherwise
*/
bool is_deltax_enough(uint8_t x_len, uint16_t delta_x);

/**
* @brief Manages the whole movement of a line from down-left to up-right
*
* @param started	true if no buttons were pressed
*		 second_mov true if second movement was started
*		 tolerance	maximum value mouse can move outside of pretended direction
*		 mov_x		total diference in x axis between starter and final point
*		 mov_y		total diference in y axis between starter and final point
*		 x_len		minimum value for mov_x
*
* @return Return 0
*/
int left_mov(bool *started, bool *second_mov, uint8_t tolerance, uint16_t *mov_x, uint16_t *mov_y, uint8_t x_len);

/**
* @brief Manages the whole movement of a line from up-left to down-right
*
* @param started	true if no buttons were pressed
*		 second_mov true if second movement was started
*		 tolerance	maximum value mouse can move outside of pretended direction
*		 mov_x		total diference in x axis between starter and final point
*		 mov_y		total diference in y axis between starter and final point
*		 x_len		minimum value for mov_x
*		 finished	true if movement was ended successfuly
*
* @return Return 0
*/
int right_mov(bool *started, bool *second_mov, uint8_t tolerance, uint16_t *mov_x, uint16_t *mov_y, uint8_t x_len, bool *finished);

/**@}*/
