/* Originally version from:
http://www.msp430launchpad.com/2010/08/half-duplex-software-uart-on-launchpad.html
http://blog.hodgepig.org/2010/09/09/573
2011-09-19 <simpleavr@gmail.com>


*/
#ifndef __UART_H
#define __UART_H

#define TXD BIT1 // TXD on P1.1
#define RXD BIT2 // RXD on P1.2

#define FCPU    MHZ*1000000
#define USEC    MHZ

#define BAUDRATE 9600
#define BIT_TIME        (FCPU / BAUDRATE)
//#define HALF_BIT_TIME   (BIT_TIME / 2)
#define HALF_BIT_TIME   (BIT_TIME - (BIT_TIME / 12))

static volatile uint8_t toSend = 0;

static volatile uint8_t bitCount = 0; // Bit count, used when transmitting byte
static volatile unsigned int TXByte; // Value sent over UART when uart_putc() is called
static volatile unsigned int RXByte; // Value recieved once hasRecieved is set

static volatile char isReceiving = 0; // Status for when the device is receiving
static volatile char hasReceived = 0; // Lets the program know when a byte is received

static void (*cb_putc)(uint8_t c) = 0;

#define UART_P1SEL TXD
#define UART_P1DIR TXD
//______________________________________________________________________
void uart_init(void) {
    P1SEL |= TXD;
    P1DIR |= TXD;

    P1IES |= RXD; // RXD Hi/lo edge interrupt
    P1IFG &= ~RXD; // Clear RXD (flag) before enabling interrupt
    P1IE  |= RXD; // Enable RXD interrupt
}

//______________________________________________________________________
char uart_getc(uint8_t *c) {
    if (!hasReceived)
        return 0;
    *c = RXByte;
    hasReceived = 0;
	toSend = 1;
    return 1;
}

//______________________________________________________________________
void uart_putc(uint8_t c) {
/*
#ifdef lcd_4bit_h
		if (c >= ' ')
			lcd_write(c);
		else
			lcd_home(0);
#endif
*/
	if (toSend) {
		TXByte = c;
		TXByte |= 0x100; // Add stop bit to TXByte (which is logical 1)
		TXByte = TXByte << 1; // Add start bit (which is logical 0)

		while (isReceiving); // Wait for RX completion

		CCTL0 = OUT; // TXD Idle as Mark
		TACTL = TASSEL_2 + MC_2; // SMCLK, continuous mode

		bitCount = 0xA; // Load Bit counter, 8 bits + ST/SP
		CCR0 = TAR; // Initialize compare register
		CCR0 += BIT_TIME; // Set time till first bit

		CCTL0 = CCIS0 + OUTMOD0 + CCIE; // Set signal, intial value, enable interrupts

		while ( CCTL0 & CCIE ); // Wait for previous TX completion
	}//if
	if (c == '\n') uart_putc('\r');
	if (cb_putc) (*cb_putc)(c);
}

#define uart_newline() uart_putc('\n'); 
//______________________________________________________________________
void uart_puts(const char *str) {
    while(*str!=0)
        uart_putc((uint8_t) *str++);
}

//______________________________________________________________________
static char nibble_to_char(uint8_t nibble) {
    if (nibble < 0xA)
        return nibble + '0';
    return nibble - 0xA + 'a';
}

//______________________________________________________________________
void uart_putdec32(uint32_t d) {
	uint8_t buf[12]="";
	uint8_t i=0;

	if (d) {
		while (d) {
			buf[i++] = d % 10 + '0';
			d /= 10;
		}//while
	}//if
	else {
		buf[i++] = '0';
	}//else

	while (i) {
		i--;
		uart_putc(buf[i]);
	}//while
}

//______________________________________________________________________
void uart_puthex8(uint8_t h) {
    uart_putc(nibble_to_char((h & 0xF0)>>4));
    uart_putc(nibble_to_char(h & 0x0F));
}

//______________________________________________________________________
void uart_puthex32(uint32_t h) {
    uart_puthex8((h & 0xFF000000) >> 24);
    uart_puthex8((h & 0x00FF0000) >> 16);
    uart_puthex8((h & 0x0000FF00) >> 8);
    uart_puthex8(h & 0xFF);
}

//______________________________________________________________________
void uart_puthex16(uint16_t h) {
    uart_puthex8((h & 0xFF00) >> 8);
    uart_puthex8(h & 0xFF);
}

//______________________________________________________________________
//interrupt(PORT1_VECTOR) PORT1_ISR(void)
// *** this pin interrupt must be setup and called from main program
void uart_port1_isr(void) {
        //TAR = 0;
    isReceiving = 1;
    P1IE &= ~RXD; // Disable RXD interrupt
    P1IFG &= ~RXD; // Clear RXD IFG (interrupt flag)

    RXByte = 0; // Initialize RXByte
    bitCount = 9; // Load Bit counter, 8 bits + start bit
    TACTL = TASSEL_2 + MC_2; // SMCLK, continuous mode
    CCR0 = TAR; // Initialize compare register
    CCR0 += HALF_BIT_TIME; // Set time till first bit
    CCTL0 = OUTMOD1 + CCIE; // Disable TX and enable interrupts

}

//______________________________________________________________________
void uart_timera0_isr(void) {
        CCR0 += BIT_TIME; // Add Offset to CCR0
    if (!isReceiving) {
        if (bitCount == 0) { // If all bits TXed
            TACTL = TASSEL_2; // SMCLK, timer off (for power consumption)
            CCTL0 &= ~ CCIE ; // Disable interrupt
        }//if
        else {
            CCTL0 |= OUTMOD2; // Set TX bit to 0
            if (TXByte & 0x01)
                CCTL0 &= ~ OUTMOD2; // If it should be 1, set it to 1
            TXByte = TXByte >> 1;
            bitCount --;
        }//else
    }//if
    else {
		if ((P1IN & RXD) == RXD) // If bit is set?
				RXByte |= 0x400; // Set the value in the RXByte
		RXByte = RXByte >> 1; // Shift the bits down
		bitCount --;
        if (bitCount == 0) {
            TACTL = TASSEL_2; // SMCLK, timer off (for power consumption)
            CCTL0 &= ~ CCIE ; // Disable interrupt

            isReceiving = 0;

            P1IFG &= ~RXD; // clear RXD IFG (interrupt flag)
            P1IE |= RXD; // enabled RXD interrupt

            if ((RXByte & 0x201) == 0x200) { // Validate the start and stop bits are correct
                RXByte = RXByte >> 1; // Remove start bit
                RXByte &= 0xFF; // Remove stop bit
                hasReceived = 1;
            }//if
        }//if
    }//else
}

#endif
