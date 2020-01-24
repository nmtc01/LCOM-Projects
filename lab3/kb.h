#pragma once
#include <lcom/lcf.h>
#include <stdint.h>
#include "lab3.h"
#include "i8254.h"

//Global variables
extern uint32_t data;
extern uint8_t code;
extern uint32_t counter1;
extern uint32_t counter;
extern int hook2;

#ifdef LAB3
int sys_inb_cnt(int port, uint32_t *byte);
#else
#define sys_inb_cnt(p,q) sys_inb(p,q)
#endif

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


