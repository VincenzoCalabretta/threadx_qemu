extern void uart_init(void);
extern void uart_puts(const char *s);

int main(void)
{
    uart_init();
    uart_puts("hello world from M7 on QEMU mps2-an500\r\n");
    for (;;) { }
}
