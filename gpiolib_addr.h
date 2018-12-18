#ifndef GPIO_ADDRS_H
#define GPIO_ADDRS_H

#define GPIO_BASE 0x0
#define GPIO_LEN  0xB4

#define GPFSEL(_x) (_x)
#define GPSET(_x)  (7  + _x)
#define GPCLR(_x)  (10 + _x)
#define GPLEV(_x)  (13 + _x)

#endif /* GPIO_ADDRS_H */

