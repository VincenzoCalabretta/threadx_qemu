#include "app/board.h"
#include "tx_api.h"

int main(void)
{
    board_init();
    uart_init();
    tx_kernel_enter();
    for (;;) { }
}
