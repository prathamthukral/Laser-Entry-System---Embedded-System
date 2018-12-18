#include "gpiolib_reg.h"
#include "gpiolib_addr.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <stdlib.h>

#define GPIO_MEM_FILE "/dev/gpiomem"

GPIO_Handle gpiolib_init_gpio(void)
{
  int fd;
  if((fd = open(GPIO_MEM_FILE, O_RDWR | O_SYNC)) == -1)
    return NULL;
  
  GPIO_Handle ret;
  ret = mmap(GPIO_BASE, GPIO_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  close(fd);
  
  if (ret == MAP_FAILED)
    return NULL;

  return ret;
}

void gpiolib_free_gpio(GPIO_Handle handle)
{
  munmap(handle, GPIO_LEN);
}

void gpiolib_write_reg(GPIO_Handle handle, uint32_t offst, uint32_t data)
{
  *(handle + offst) = data;
}

uint32_t gpiolib_read_reg(GPIO_Handle handle, uint32_t offst)
{
  return *(handle + offst);
}
