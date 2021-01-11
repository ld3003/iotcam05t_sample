
#ifndef __HI3516EV200_GPIO__
#define __HI3516EV200_GPIO__

int hi3516ev200_gpio_in(unsigned int gpio_chip_num, unsigned int gpio_offset_num);
int hi3516ev200_gpio_out(unsigned int gpio_chip_num, unsigned int gpio_offset_num,unsigned int gpio_out_val);

#endif