#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

void power_management_init(uint64_t abs_time_ms);
int power_management_off_until_gpio_high(int gpio);
int power_management_off_until_time(uint64_t abs_time_ms);
int power_management_off_for_ms(uint64_t duration_ms);

#endif 