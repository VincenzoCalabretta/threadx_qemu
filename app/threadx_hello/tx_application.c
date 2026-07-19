#include "app/board.h"
#include "tx_api.h"

#define HELLO_STACK_SIZE 1024

static TX_THREAD hello_thread;
static UCHAR hello_stack[HELLO_STACK_SIZE];

static void hello_entry(ULONG arg)
{
    (void)arg;
    uart_puts("hello world from threadx\r\n");
    for (;;) {
        tx_thread_sleep(100);
    }
}

void tx_application_define(void *first_unused_memory)
{
    (void)first_unused_memory;
    tx_thread_create(
        &hello_thread,
        "hello",
        hello_entry,
        0,
        hello_stack,
        HELLO_STACK_SIZE,
        1, 1,
        TX_NO_TIME_SLICE,
        TX_AUTO_START);
}
