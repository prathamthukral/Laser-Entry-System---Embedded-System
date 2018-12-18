#ifndef GPIO_REG_H
#define GPIO_REG_H

#include <stdint.h>

typedef uint32_t* GPIO_Handle;

GPIO_Handle gpiolib_init_gpio(void);
void        gpiolib_free_gpio(GPIO_Handle handle);

void        gpiolib_write_reg(GPIO_Handle handle,uint32_t offst, uint32_t data);
uint32_t    gpiolib_read_reg (GPIO_Handle handle, uint32_t offst);

#endif /* GPIO_REG_H */
