/* **************** LDD:2.0 s_21/HINT_turnon.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein, 2012
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@coopj.com
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */
#define BASE_BAUD ( 1843200 / 16 )
#define QUOT ( BASE_BAUD / 1200 )

static void mymouse_turnon(void)
{
	int quot = QUOT;
/* byte 0: Transmit Buffer */
	outb(0x0, IOSTART + UART_TX);
/* byte 1: Interrupt Enable Register */
	outb(UART_IER_RDI, IOSTART + UART_IER);
/* byte 2: FIFO Control Register */
	outb(UART_FCR_ENABLE_FIFO, IOSTART + UART_FCR);
/* byte 3: Line Control Register */
	outb(UART_LCR_WLEN7, IOSTART + UART_LCR);
/* byte 4: Modem Control Register */
	outb(UART_MCR_RTS | UART_MCR_OUT2, IOSTART + UART_MCR);

	/* set baud rate */
	outb(UART_LCR_WLEN7 | UART_LCR_DLAB, IOSTART + UART_LCR);
	outb(quot & 0xff, IOSTART + UART_DLL);	/* LS of divisor */
	outb(quot >> 8, IOSTART + UART_DLM);	/* MS of divisor */
	outb(UART_LCR_WLEN7, IOSTART + UART_LCR);
}
