/*
  tms080x calculation chip emulation, w/ TI-1500 and Sinclair Scientific ROMs

  code provided as is, no warranty
  chris chung (www.simpleavr.com)
  September 2013

  this is a hobby project

  you should not use code for commercial purpose w/o permission from creators
  nice if you give credit, mention source(s) if you adopt much of the code

  based on work of Ken Shirriff, and others, please visit

  http://righto.com/ti and http://righto.com/sinclair for details

  . h/w (UI booster) based on another project (ezs430trainer) 

    http://www.simpleavr.com/msp430-projects/ez430trainer

  . test compiled and run on win7 w/ cygwin, mint 14 linux.
  . NOT tested w/ CCS, IAR, should also work w/ tweaks.
  . compile w/ msp430-gcc like so

	msp430-gcc -Os -Wall -ffunction-sections -fdata-sections -fno-inline-small-functions -Wl,-Map=tms0800.map,--cref -Wl,--relax -Wl,--gc-sections -I<your mpsgcc>/msp430/include -mmcu=msp430g2553 -o tms0800.elf tms0800.c

  September 2013 initial check-in


*/


#include <msp430.h>

#include <stdint.h>
#define G2553
#define MHZ 8

//#include "hw_uart.h"
#include "uart.h"


void led_putc(uint8_t c);
void led_dot(uint8_t pos);
void seg2port(uint8_t c);
uint8_t led_pos=0;

#include "led.h"
#include "cpu.h"

	/*
		0x17 0x18 0x19 0x1f 0x20	// 0x0 0x1 0x2 0x3 0x4
		0x14 0x15 0x16 0x1e 0x30	// 1x0 1x1 1x2 1x3 1x4
		0x11 0x12 0x13 0x1d 0x40	// 2x0 2x1 2x2 2x3 2x4
		0x10 0x1a 0x1b 0x1c 0x50	// 3x0 3x1 3x2 3x3 3x4

		KN 1 2 3 4 5 6 7 8 9	// 0x11...0x19
		KO C = + - * / E . 0	// 0x21...0x29
		KO C v + - / * ^ E 0	// 0x21...0x29	(sinclair scientific)
		KP
		 0   1   2   3   4  (TI-1500) key X switches ROMs
		[7] [8] [9] [/] [C]
		[4] [5] [6] [*] [X]
		[1] [2] [3] [-] [E]
		[0] [0] [.] [+] [=]

		[7] [8] [9] [*] [C] (Sinclair Scienticfic)
		[4] [5] [6] [/] [X]
		[1] [2] [3] [-] [^]
		[0] [0] [E] [+] [V]

      +-----------------------------------------+
	  | +-----------------+ +-----------------+ |
	  | |  _   _   _   _  | |  _   _   _   _  | |
	  | | | | | | | | | | | | | | | | | | | | | |
	  | | |-| |-| |-| |-| | | |-| |-| |-| |-| | |
	  | | |_|.|_|.|_|.|_|.| | |_|.|_|.|_|.|_|.| |
	  | +-----------------+ +-----------------+ |
	  |                                         |
	  |           [  ] [7 ] [8 ] [9 ] [/ ] [C ] |
	  |                                         |
	  |           [  ] [4 ] [5 ] [6 ] [* ] [X ] |
	  |                                         |
	  |                [1 ] [2 ] [3 ] [- ] [E ] |
	  |                                         |
	  |                [0 ] [0 ] [E ] [+ ] [= ] |
	  |                                         |
      +-----------------------------------------+

	*/

static const char hwkey_map[] = {
	0x19, 0x21, 0x26, 0x20, 0x21, 0x18, 0x17,	// 0x2 0x4 0x3 -x- -x- 0x1 0x0
	0x00, 0x2f, 0x25, 0x11, 0x14, 0x16, 0x15,	// --- 1x4 1x3 2x0 1x0 1x2 1x1
	0x00, 0x00, 0x27, 0x29, 0x28, 0x22, 0x23,	// --- --- 2x4 3x1 3x2 3x4 3x3
	0x00, 0x00, 0x00, 0x29, 0x12, 0x24, 0x13,	// --- --- --- 3x0 2x1 2x3 2x2
};

static volatile uint8_t ticks = 0;
static volatile uint8_t clicks = 1;
static volatile uint8_t buzz = 0;
static volatile uint8_t hwkey = 0x00;

union u_data {
	uint16_t w;
	struct {
		uint8_t l;
		uint8_t h;
	} b;
};



//________________________________________________________________________________
uint8_t getc_via_uart(uint8_t *cp) {
	uint8_t c = 0;
	if (uart_getc(&c)) {
		if (c >= '1' && c <= '9') {
			c -= '0' - 0x10;
		}//if
		else {
			// C = + - * / E . 0	// 0x21...0x29
			// C v + - / * ^ E 0	// 0x21...0x29	(sinclair scientific)
			switch (c) {
				case 'c':  c = 0x21; break;
				case '=':  c = 0x22; break;		// down key in sinclair
				case '+':  c = 0x23; break;
				case '-':  c = 0x24; break;
				case '*':  c = _sinclair ? 0x26 : 0x25; break;
				case '/':  c = _sinclair ? 0x25 : 0x26; break;
				case 'e':  c = _sinclair ? 0x28 : 0x27; break;
				case '.':  c = _sinclair ? 0x27 : 0x28; break;		// up key in sinclair
				case '0':  c = 0x29; break;
				//default: c = 0x00; break;
				default: break;
			}//swtich
		}//else
	}//if 
	*cp = c;
	return c;
}
//________________________________________________________________________________

#define ST_LED		BIT0
#define ST_DASM		BIT1
#define ST_HOLD		BIT2
#define ST_STEP		BIT3
#define ST_MINUS	BIT4

uint8_t g_state=ST_LED;
//uint8_t g_state=0x07;
uint16_t blinks=0;

#ifdef __HW_UART_H
static volatile uint8_t inc=0;

//________________________________________________________________________________
void uart_rx_isr(uint8_t c) {
	//uart_putc(c);
	inc = c;
	//P1OUT ^= BIT0;		// toggle P1.0 (red led) for debug
}
#endif
//______________________________________________________________________
uint8_t get_key() {
	uint8_t c = 0;
	if (hwkey) { 
		c = hwkey;
		hwkey = 0;
		buzz = 6;
		//__blink_debug(0);
	}//if
	else {
#ifdef __HW_UART_H
		if (inc) {
			c = inc;
			inc = 0;
		}//if
#else
		getc_via_uart(&c);
#endif
	}//else
	return c;
}


//________________________________________________________________________________
void led_clear() {
	uint8_t i;
	for (i=0;i<8;i++) seg2port(POS__);
}

// storage for multiplexing, 5 bytes output x 8 digits
// loads data into output[] for immediate led multiplexing


uint8_t output[5 * 8];
uint8_t  pos=0, stays=0x40; 
//__________________________________________________
void seg2port(uint8_t c) {

	static const uint8_t digit_map[][4] = {
		{ DIGIT_0_P1, DIGIT_1_P1, DIGIT_2_P1, DIGIT_3_P1, },
		{ DIGIT_0_P2, DIGIT_1_P2, DIGIT_2_P2, DIGIT_3_P2, },
		};

	if (led_pos>=8) led_pos = 0;

	uint8_t *pp = output + (led_pos+1) * 5;
	uint8_t i, dec=0;

	if (c&0x80) { // add decimal point on demand
		dec = SEG_d_P1;
		c &= 0x7f;
	}//if

	const uint8_t *cp = digit2ports[c];

	*--pp = *cp++;				// load stay time

	for (i=0;i<2;i++) {
		*--pp = (*cp | dec) | digit_map[i][led_pos&0x03];
		*--pp = (*cp & dec) | ~digit_map[i][led_pos&0x03];
		if (led_pos&0x04) *pp = ~(*pp);
		dec = 0x00;
		cp++;
	}//for
	led_pos++;

	// 5 bytes port value; p2out, p2dir, p1out, p1dir, stay

}

//__________________________________________________
uint8_t paradiso_loop() {

	//_______ these are for keyboard scanning
	static uint8_t key_row=7, key_col=7;
	static const uint8_t seg_a_d[] = { 
		SEGS_BIT_P1(SEG_A_P1) + SEGS_BIT_P2(SEG_A_P2),
		SEGS_BIT_P1(SEG_B_P1) + SEGS_BIT_P2(SEG_B_P2),
		SEGS_BIT_P1(SEG_C_P1) + SEGS_BIT_P2(SEG_C_P2),
		SEGS_BIT_P1(SEG_D_P1) + SEGS_BIT_P2(SEG_D_P2),
		SEGS_BIT_P1(SEG_E_P1) + SEGS_BIT_P2(SEG_E_P2),
		SEGS_BIT_P1(SEG_F_P1) + SEGS_BIT_P2(SEG_F_P2),
		SEGS_BIT_P1(SEG_G_P1) + SEGS_BIT_P2(SEG_G_P2),
		SEGS_BIT_P1(SEG_d_P1) + SEGS_BIT_P2(SEG_d_P2), };

	//________ stays... lsb = cycles to stay on, msb = cycles to stay off (dark)
	//         0x43 means stay on for 3 cycles, then stay off for 4 cycles
	if (stays & 0x0f) { 
		stays--; 
		//__________ use off time to check for keys
		if (!(stays&0xf0)) {		
			// no msb, that means we are in the off cycles
			//____ do scan on another column
			key_col--;

			uint8_t *tp = (uint8_t*) 0x27;
			tp += (seg_a_d[key_row]&0x08);
			uint8_t bit = 1<<(seg_a_d[key_row]&0x07);
			*tp &= ~bit; tp -= 6; 	// REN
			*tp &= ~bit; tp++; 		// OUT
			*tp |= bit;				// DIR

			uint8_t volatile *cp = (uint8_t*) 0x20;
			cp += (seg_a_d[key_col]&0x08);

			uint8_t bitc = 1<<(seg_a_d[key_col]&0x07);
			if (!(*cp&bitc)) {
				stays = 0x0e;	// key pressed, mark stay w/ 0x0e so we will visit again
				key_col++;		// and have the same table (row / col)
			}//if
			else {
				if (stays == 0x0d) {		
					// key released (was pressed before)
					// translate to more managable code
					hwkey = (key_row<<4) | key_col;
					hwkey -= 0x10;
					hwkey = hwkey_map[(hwkey&0x0f)*7+(hwkey>>4)];
				}//if
			}//else

			*tp &= ~bit; tp--;
			*tp |= bit;	tp += 6;
			*tp |= bit;
			if (!key_col) {		// last col done, advance row
				key_row--;
				if (key_row) {
					key_col = key_row;
				}//if
				else {
					key_row = key_col = 7;
				}//else
			}//if
			//
		}//if
		return 0; 
	}
	stays >>= 4;
	P2DIR = 0; P2OUT = 0; 
	P1DIR &= ~USED_1; P1OUT &= ~USED_1;
	if (stays) { 
		stays--; 
		//_______________ set ports up for key scanning
		P2OUT |= SEGS_2;
		P2REN |= SEGS_2;
		//P2REN |= USED_2;
		P1OUT |= SEGS_1;
		P1REN |= SEGS_1;
		return 0; 
	}//if

	//___________ return if we are in dark
	if (!(g_state&ST_LED)) stays = 0x80;
	/*
    a7 b3 b0 a4 b6 a6 b1 a5
    (A) B  C  D  E  F  G  d ..0 (7)
     A (B) C  D  E  F  G  d ..1 (6)
     A  B (C) D  E  F  G  d ..2 (5)
     A  B  C (D) E  F  G  d ..3 (4)
     A  B  C  D (E) F  G  d ..4 (3)
     A  B  C  D  E (F) G  d ..5 (2)
     A  B  C  D  E  F (G) d ..6 (1)
     A  B  C  D  E  F  G (d)..7 (0)
	 0  1  2  3  4  5  6  7

     b7 a7 a6 b5 b4 b3
     (0)-A--F-(1)(2)-B
      E  D (.) C  G (3)
     b6 a4 a5 b0 b1 b2
	*/

	P2REN = 0;
	P1REN &= ~USED_1;
	P2DIR = 0; 
	P1DIR &= ~SEGS_1; 
	P2OUT = 0; 
	P1OUT &= ~SEGS_1;

	if (!stays) {
		uint8_t *ioptr = output + (pos*5);

		P2OUT |= *ioptr++;
		P2DIR |= *ioptr++;
		P1OUT |= (*ioptr++ & USED_1);
		P1DIR |= (*ioptr++ & USED_1);
		stays = *ioptr;
		// uncomment to compensate for mix-match led pairs
		//else if (pos >= 4) stays += 8;
	}//if

	pos++;
	pos &= 0x07;

	return 0;

}

//________________________________________________________________________________
void led_dot(uint8_t pos) {
	uint8_t *sp = output;
	sp += pos*5+2;

	if (pos&0x04)
		*sp &= ~SEG_d_P1;
	else
		*sp |= SEG_d_P1;
	sp++;
	*sp |= SEG_d_P1;
}

//________________________________________________________________________________
void led_putc(uint8_t c) {
	static uint8_t hide=0;

	switch (c) {
		case '.':
			if (led_pos) led_dot(led_pos-1);
			return;
		case '_': 
			c = POS__; 
			break;
		case '(': case ')': 
			hide ^= 1; 
			return;
		case '-': 
			c = POSmn; break;
		case '>': 
			led_clear();
		case '\n': case '\r': 
			led_pos = 16;
			return;
		default: 
			if (c>='A' && c<='Z') c -= 'A' - 10;
			else if (c>='a' && c<='z') c -= 'a' - 10;
			else if (c>='0' && c<='9') c -= '0';
			else return;
			break;
	}//switch

	if (led_pos<16 && !hide) seg2port(c);
}

#define BUZZER_PIN	BIT3
#define MINUS_PIN	BIT0

//________________________________________________________________________________
int main(void) {
	WDTCTL = WDTPW + WDTHOLD;
	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL  = CALDCO_8MHZ;

	//led_init();
	uart_init();
#ifdef __HW_UART_H
    uart_set_rx_isr_ptr(uart_rx_isr);
#endif

	//led_off();

	TA1CCR0  = 800;		// 10khz
	TA1CCTL0 = CCIE;
	TA1CTL = TASSEL_2 + MC_1;                  // SMCLK, upmode

	//cb_putc = led_putc;		// when we want to show on uart

	P1SEL &= 0x0f;
	P2SEL  = 0;
	P1DIR |= MINUS_PIN|BUZZER_PIN;
	P1OUT &= ~(MINUS_PIN|BUZZER_PIN);

	_BIS_SR(GIE);

	/* code to dectect press-hold-reset if we need it
	while (clicks) asm("nop");
	if (stays == 0x0e) {
		__blink_debug(0);
	}//if
	*/




	uint8_t first=1;

	load_rom(0);
	led_clear();
	/*
			0x17 0x18 0x19 0x1f 0x20
			0x14 0x15 0x16 0x1e 0x30
			0x11 0x12 0x13 0x1d 0x40
			0x10 0x1a 0x1b 0x1c 0x50
		KN 1 2 3 4 5 6 7 8 9
		KO C = + - * / E . 0
		KP
	*/
	uint16_t *prev_addr = 0;
	uint16_t *last_show_addr = (uint16_t *) 1;
	uint32_t last_show = 0xffffffff;
	uint8_t last_dot=0, spin=50;
	uint8_t key_presist=0;

	uint8_t c = 0x30;

	while (1) {
		if (c || (c=get_key())) {
			if (first) {
				first = 0;
				uart_puts("\nWelcome to Paradiso Sweetener\n");
			}//if
			uint8_t next = 0;
			uint8_t show_regs = 0;
			switch (c) {
				case 'x': 		
				case 0x2f: 		// toggle cpu rom (system key from calculator)
					load_rom(1);
				case 0x21: 		// reset cpu ('c' key from calculator)
					load_rom(0);
					next = 0x30;
					prev_addr = _address;
					spin = 0;
					break;
				case ' ': 		
					//g_state |= ST_DASM;
					g_state &= ~ST_STEP;
					g_state ^= ST_HOLD;
					if (g_state&ST_STEP) g_state &= ~ST_HOLD;
					if (!(g_state&ST_HOLD)) next = 0x30;
					break;
				case 'l': 		// toggle led displaying
					g_state ^= ST_LED;
					break;
				case 'd': 		// toggle dasm
					g_state ^= ST_DASM;
					uart_puts("\ndisassembler ");
					if (g_state&ST_DASM)
						uart_puts("on\n");
					else
						uart_puts("off\n");
					break;
				case 'n':		// continue w/o dasm
					g_state &= ~ST_DASM;
					break;
				case '\r': 		
					g_state |= ST_DASM|ST_STEP;
				case 0x30:
					_keyStrobe = 0;
					if (key_presist) {
						key_presist--;
						if (_keyPressed && ((_keyPressed&0x0f) == _dActive)) {
							_keyStrobe = _keyPressed>>4;
						}//if
					}//if
					else {
						_keyPressed = 0;
					}//else

					if (spin <= 100) {
						if (prev_addr == _address)
							spin++;
						else
							spin = 0;
						prev_addr = _address;
						step();
					}//if

					if (g_state&ST_STEP) show_regs = 1;
					if (!show_regs) next = 0x30;
					break;
				default: 
					if (c < 0x30) {			// calculator keys
						key_presist = 100;	// emulate bouncing delay
						_keyPressed = c;
						//prev_addr = 0;
						spin = 0;
						next = 0x30;
					}//if
					else {
						uart_puts("\n\ncalculator keys.. [0]...[9][c][e][.][+][-][*][/]\n");
						uart_puts("option toggles... [l]ed [d]is-assembler [x]ROM\n");
						uart_puts("commands          [ENTER] step [SPACE] stop/go\n\n");
					}//else
					break;
			}//switch

			//if (c != 0x30 || (g_state&ST_DASM && spin<12)) {
			if (g_state&ST_DASM && spin<12) {
				//uart_putc(c); uart_putc('('); uart_puthex8(spin); uart_putc(')');
				if (c == 0x30 && last_show_addr == prev_addr) {
					uart_putc('.');
				}//if
				else {
					uart_putc('\n');
					last_show_addr = prev_addr;
					uart_puthex16(prev_addr - _base_addr);
					uart_putc(_cc ? '+' : '-');
					uart_puthex16(_instruction);
					uart_putc(' ');
					uart_puts(_opCode);
					if (_branch_addr) {
						uart_puthex16(_branch_addr);
						uart_putc(' ');
					}//if
					//uart_puts("\r[2K\r");
				}//else
				//if (spin == 11) show_regs = 1;
			}//if
			if (spin == 11) show_regs = 1;

			if (show_regs) {
				uint8_t i, r;
				//____________ flags A, B + K
				uart_putc('\n');
				const char cr[] = { 'D', 'A', 'B', 'C', 'K', };
				for (r=0;r<5;r++) {
					uart_puts("  ");
					uart_putc(cr[r]);
					uart_puts(" [");
					for (i=0;i<=10;i++) 
						if (r) uart_putc(regs[r][i]<10 ? regs[r][i]+'0' : regs[r][i]);
						else   uart_putc(_dActive == (i+1) ? '0' : '1');
					uart_putc(']');
					if (!r) {
						uart_puts(" Flags, CC [");
						uart_putc(_cc ? '1' : '0');
						uart_putc(']');
					}//if
					else {
						if (r != 3) {
							uart_puts(" [");
							uint8_t fi = r - 1;
							uint16_t f = 1<<10;
							if (r == 4) fi--;
							while (f) {
								uart_putc(_flgs[r-1]&f ? '1' : '0');
								f >>= 1;
							}//while
							uart_putc(']');
						}//if
					}//else
					uart_putc('\n');
				}//for
			}//if

			if (g_state&ST_DASM || (spin>16)) {	// disply check only on waitkey or stepping
				uint8_t i, dot=0;
				uint32_t show=0;
				for (i=1;i<9;i++) {
					show <<= 4;
					show += RegA[i];
					if (RegB[i]) dot = i;
				}//for
				if (show != last_show || dot != last_dot) {
					//uart_putc('['); uart_puthex8(spin); uart_putc(']');
					led_clear();
					if (_sinclair) {
						for (i=4;i<9;i++) led_putc(RegA[i] + '0');
						led_putc(RegA[1] ? '-' : '_');
						led_putc(RegA[2] + '0');
						led_putc(RegA[3] + '0');
						led_dot(0);
						if (RegA[0] == 5)
							g_state |= ST_MINUS;
						else
							g_state &= ~ST_MINUS;
					}//if
					else {
						uint8_t suppress_zero_at = dot;
						for (i=1;i<9;i++) {
							if (i<suppress_zero_at && !RegA[i]) {
								led_putc('_');
							}//if
							else {
								if (RegA[i] >= 10)
									led_putc('-');
								else
									led_putc(RegA[i] + '0');
								suppress_zero_at = 0;
							}//else
						}//for
						if (RegA[0] >= 10)
							g_state |= ST_MINUS;
						else
							g_state &= ~ST_MINUS;
						led_dot(dot-1);
					}//else
					last_show = show;
					last_dot = dot;
				}//if
			}//if

			c = get_key();
			if (!c) c = next;
		}//if
	}//while


}

#ifdef __UART_H
//________________________________________________________________________________
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void) {
	uart_port1_isr();
}

//________________________________________________________________________________
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void) {
	uart_timera0_isr();
}
#endif

//________________________________________________________________________________
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0_iSR(void) {
	ticks++;
	//if (ticks > 2048) {
	if (ticks > 100) {
		ticks = 0;
		clicks--;
		if (buzz) {
			P1OUT |= BUZZER_PIN;
			buzz--;
			if (!buzz) P1OUT &= ~BUZZER_PIN;
		}//if
	}//if
	P1OUT &= ~MINUS_PIN;
	if (g_state&ST_MINUS && !(ticks&0x0f))
		P1OUT |= MINUS_PIN;
	paradiso_loop();
}
