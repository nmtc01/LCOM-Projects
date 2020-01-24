#include "rt.h"
#include <lcom/lcf.h>

int hook_rtc = 8;

int rtc_subscribe_int(uint8_t *bit_no) {
  *bit_no = hook_rtc;
  if (sys_irqsetpolicy(IRQ8, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_rtc) != OK)
    return 1; //failed
  return 0;   //success
}

int rtc_unsubscribe_int() {
  int hook_id = hook_rtc;
  if (sys_irqrmpolicy(&hook_id) != OK) {
    return 1; //failure
  }
  return 0; //success
}

int set_rtc_int() {
  uint32_t reg_b = BIT(4) | BIT(1);
  if (sys_outb(RTC_ADDR_REG, REG_B)) { // Gain access to REG_B
    return -1;
  }
  if (sys_outb(RTC_DATA_REG, reg_b)) { // Set REG_B bit 4 and bit 1
    return -1;
  }
  return 0;
}

int unset_rtc_int() {
  uint32_t reg_b = BIT(1);
  if (sys_outb(RTC_ADDR_REG, REG_B)) { // Gain access to REG_B
    return -1;
  }
  if (sys_outb(RTC_DATA_REG, reg_b)) { // Set REG_B bit 4 and bit 1
    return -1;
  }
  return 0;
}

int clear_reg_c() {
  uint32_t reg_c;
  if (sys_outb(RTC_ADDR_REG, REG_C)) { // Gain access to REG_C
    return -1;
  }
  if (sys_inb(RTC_DATA_REG, &reg_c)) { // Clear REG_C IRQF
    return -1;
  }
  return 0;
}

// GETTERS
int get_seconds() {
  uint32_t seconds;
  if (sys_outb(RTC_ADDR_REG, SECONDS)) { // Gain access to SECONDS
    return -1;
  }
  if (sys_inb(RTC_DATA_REG, &seconds)) { // Get seconds
    return -1;
  }
  // printf("\n%u\n", seconds);
  return convert_BCD(seconds);
}

int get_minutes() {
  uint32_t minutes;
  if (sys_outb(RTC_ADDR_REG, MINUTES)) { // Gain access to MINUTES
    return -1;
  }
  if (sys_inb(RTC_DATA_REG, &minutes)) { // Get minutes
    return -1;
  }
  // printf("\n%u\n", minutes);
  return convert_BCD(minutes);
}

int get_hours() {
  uint32_t hours;
  if (sys_outb(RTC_ADDR_REG, HOUR)) { // Gain access to HOUR
    return -1;
  }
  if (sys_inb(RTC_DATA_REG, &hours)) { // Gaet hours
    return -1;
  }
  // printf("\n%u\n", hours);
  return convert_BCD(hours);
}

int get_day() {
  uint32_t days;
  if (sys_outb(RTC_ADDR_REG, DAY)) { // Gain access to DAY
    return -1;
  }
  if (sys_inb(RTC_DATA_REG, &days)) { // Get days
    return -1;
  }
  // printf("\n%u\n", days);
  return convert_BCD(days);
}

int get_month() {
  uint32_t months;
  if (sys_outb(RTC_ADDR_REG, MONTH)) { // Gain access to MONTH
    return 0;
  }
  if (sys_inb(RTC_DATA_REG, &months)) { // Get months
    return 0;
  }
  // printf("\n%u\n", months);
  return convert_BCD(months);
}

int get_year() {
  uint32_t years;
  if (sys_outb(RTC_ADDR_REG, YEAR)) { // Gain access to YEAR
    return -1;
  }
  if (sys_inb(RTC_DATA_REG, &years)) { // Get years
    return -1;
  }
  // printf("\n%08x\n", years);
  return convert_BCD(years);
}

// UTILITY
int convert_BCD(uint32_t bcd) {
  return (((bcd & 0xF0) >> 4) * 10) + (bcd & 0x0F);
}
