/*
 * Top-level bring-up for the S32G399A M7_0 hello-world.
 */

extern void clock_init(void);
extern void pinmux_init(void);
extern void uart_init(void);
extern void uart_puts(const char *s);

int main(void)
{
    clock_init();
    pinmux_init();
    uart_init();
    uart_puts("hello world from M7_0\r\n");
    for (;;) { }
}
