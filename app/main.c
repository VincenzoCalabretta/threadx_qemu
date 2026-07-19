#include "app/board.h"
#include <rtems/bspIo.h>

int main(void)
{
    board_init();
    uart_init();
    printk("hello %d from printk\n", 42);
    uart_puts("hello world\r\n");
    for (;;) { }
}
