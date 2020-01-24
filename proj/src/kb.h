#pragma once

/** @defgroup keyboard keyboard
* @{
*
*/

#include <lcom/lcf.h>
#include <stdint.h>
#include "i8254.h"

//Macros
#define STAT_REG        0x64
#define OBF				0x01
#define IBF				0x02
#define PAR_ERR			0x80
#define TO_ERR			0x40
#define KBC_CMD_REG     0x64
#define OUT_BUF         0x60

#define ESC_BREAK       0x81
#define DELAY_US        20000

#define TWO_LONG		0xE0
#define MK_OR_BRK		0x0080
#define READ_CMD		0x20
#define WRT_CMD		    0x60

#define IRQ1			0x01
#define IRQ2			0x02

#define UP_MAKE			0x48
#define	UP_BREAK		0xC8
#define LEFT_MAKE		0x4B
#define	LEFT_BREAK		0xCB
#define DOWN_MAKE		0x50
#define DOWN_BREAK		0xD0
#define RIGHT_MAKE		0x4D
#define RIGHT_BREAK		0xCD
#define SPACE_MAKE		0x39
#define SPACE_BREAK		0xB9
#define ENTER_MAKE		0x1C


//Global variables
extern uint32_t data;
extern uint8_t code;
extern uint32_t counter1;
extern uint32_t counter;
extern int hook2;

/**
* @brief Subscribes and enables Keyboard interrupts
*
* @param bit_no address of memory to be initialized with the
*         bit number to be set in the mask returned upon an interrupt
* @return Return 0 upon success and non-zero otherwise
*/
int kb_subscribe_int(uint8_t *bit_no);

/**
* @brief Unsubscribes Keyboard interrupts
*
* @return Return 0 upon success and non-zero otherwise
*/
int kb_unsubscribe_int();

/**
* @brief Checks whether a scancode is two or one byte long
*		 Sets size (1 or 2), makecode (true or false) and code[2]
*		 Prints the scancodes, calling kbd_print_scancode(makecode, size, codes)
*
* @return It doesn't return anything
*/
void kbd_data_handler();

/**
* @brief Sends a read command to status register
*		 Passes the value on the output buffer to a variable
*		 Changes this variable so that its first bit would be 1
*        Sends a write command to status register
*		 Passes the value on the output buffer to the previous variable
*
* @return Return 0 upon success and non-zero otherwise
*/
int enable_interrupts();

/**@}*/
