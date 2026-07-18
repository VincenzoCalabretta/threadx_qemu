#include "app/board.h"

int main(void)
{
    board_init();
    uart_init();
    uart_puts("hello world\r\n");
    for (;;) { }
}
