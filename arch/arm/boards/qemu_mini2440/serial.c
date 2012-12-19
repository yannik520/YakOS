
volatile unsigned int *UART0DR;
void __console_init(void)
{
  UART0DR = (unsigned int *)0x101f1000;
}

void __puts(const char *s) {

	while(*s != '\0') {

		*UART0DR = (unsigned int)(*s);
		s++;
	}

}
