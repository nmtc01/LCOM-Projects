#pragma once

/** @defgroup rtc rtc
* @{
*
*/

#include <lcom/lcf.h>
#include <stdint.h>
#include "i8254.h"

#define RTC_ADDR_REG 0x70	    /**< @brief Control Word for ADDR_REG */
#define RTC_DATA_REG 0x71	    /**< @brief Control word for DATA_REG*/
#define SECONDS 0x00	        /**< @brief Second REG */
#define MINUTES 0x02	        /**< @brief Minutes REG*/
#define HOUR 0x04	            /**< @brief Hour REG*/
#define DAY 0x07	            /**< @brief Days REG*/
#define MONTH 0x08	          /**< @brief Months REG*/
#define YEAR 0x09	            /**< @brief Year REG*/
#define REG_A 0x0A	          /**< @brief A REG*/
#define REG_B 0x0B	          /**< @brief B REG */
#define REG_C 0x0C	          /**< @brief C REG*/
#define IRQ8 8                /**< @brief IRQ line*/ 

//Global Variables
extern int hook_rtc;


//
/**
* @brief Subscribes to RTC interrupts
*
* @param bit_no address of memory to be initialized with the bit number to be set in the mask   returned upon an interrupt
*
* @return Return 0 upon success and non-zero otherwise
*/
int rtc_subscribe_int(uint8_t *bit_no);

/**
* @brief Unsubscribes RTC interrupts
*
* @return Return 0 upon success and non-zero otherwise
*/
int rtc_unsubscribe_int();

/**
* @brief Enables RTC interupts on update
*
* @return Return 0 upon success and non-zero otherwise
*/
int set_rtc_int();

/**
* @brief Disables RTC interupts on update
*
* @return Return 0 upon success and non-zero otherwise
*/
int unset_rtc_int();

/**
* @brief Clear interrupt flag
*/
int clear_reg_c();


// GETTERS
/**
* @brief Returns seconds in real time
*
* @return Return seconds in decimal value
*/
int get_seconds();

/**
* @brief Returns minutes in real time
*
* @return Return minutes in decimal value
*/
int get_minutes();

/**
* @brief Returns hours in real time
*
* @return Return hours in decimal value
*/
int get_hours();

/**
* @brief Returns days in real time
*
* @return Return days in decimal value
*/
int get_day();

/**
* @brief Returns months in real time
*
* @return Return months in decimal value
*/
int get_month();

/**
* @brief Returns years in real time
*
* @return Return years in decimal value
*/
int get_year();

// UTILITY
/**
* @brief Converts BCD value to decimal
* @param bcd BCD value to convert
* @return Return decimal vale
*/
int convert_BCD (uint32_t bcd);

/**@}*/
