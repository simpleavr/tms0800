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

	msp430-gcc -Os -Wall -ffunction-sections -fdata-sections -fno-inline-small-functions -Wl,-Map=tms0800x.map,--cref -Wl,--relax -Wl,--gc-sections -I<your mpsgcc>/msp430/include -mmcu=msp430g2553 -o tms0800.elf tms0800x.c

  September 2013 initial check-in
  June 2014 
	. pcb design
	. employs 2 x 4 digit bubble led
	. mutilplexing w/ 8 col x 8 row, no more charlieplexing
	. remove uart dis-assembler

*/


#include <msp430.h>

#include <stdint.h>
#define G2452

#define V2

#include "led_pcb.h"
#include "cpu.h"

/*

		KN 1 2 3 4 5 6 7 8 9	// 0x11...0x19
		KO C = + - * / E . 0	// 0x21...0x29
		KO C v + - / * ^ E 0	// 0x21...0x29	(sinclair scientific)
		KP

	KB layout
	datamath layout            sinclair layout

	[  ]  [  ]  [  ]  [  ]     [  ]  [  ]  [  ]  [  ]
	[  ]  [  ]  [  ]  [  ]     [  ]  [  ]  [  ]  [  ]
	 CE    x     /     (8       ^     C    (9
	[10]  [61]  [21]  [43]     [10]  [61]  [21]  [43]
     7     8     9     C        7     8     9     x log
	[40]  [64]  [42]  [41]     [40]  [64]  [42]  [41]
     4     5     6     -        4     5     6     / tan
	[70]  [76]  [72]  [71]     [70]  [76]  [72]  [71]
     1     2     3     +        1     2     3     + sin
	[30]  [63]  [32]  [31]     [30]  [63]  [32]  [31]
     0     0     .     =        V     0     E     - cos
	[50]  [65]  [52]  [51]     [50]  [65]  [52]  [51]

*/
static const char hwkey_map[] = {		// converts to tms080x scan code

#ifdef V2
/*	 [60]  [73]
   ^E[20]Cx[62] /[74]
	7[10] 8[61] 9[21]xC[43]
	4[40] 5[64] 6[42]/-[41]
	1[70] 2[76] 3[72]++[71]
   V0[30] 0[63]E.[32]-=[31]
*/
//     0     1     2     3     4     5     6     7
	0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 1
	0x27, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 2
	0x29, 0x22, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00,	// 3
	0x14, 0x24, 0x16, 0x21, 0x00, 0x00, 0x00, 0x00,	// 4
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 5
	0x00, 0x18, 0x25, 0x29, 0x15, 0x00, 0x00, 0x00,	// 6
	0x11, 0x23, 0x13, 0x00, 0x26, 0x00, 0x12, 0x00,	// 7
	/*
		KN 1 2 3 4 5 6 7 8 9	// 11...19
		KO C = + - x / E . 0	// 21...29
		KO C v + - / x ^ E 0	// 21...29	(sinclair scientific)
	*/
//     0     1     2     3     4     5     6     7
	0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 1
	0x27, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 2
	0x22, 0x24, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00,	// 3
	0x14, 0x25, 0x16, 0x26, 0x00, 0x00, 0x00, 0x00,	// 4
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 5
	0x00, 0x18, 0x21, 0x29, 0x15, 0x00, 0x00, 0x00,	// 6
	0x11, 0x23, 0x13, 0x00, 0x00, 0x00, 0x12, 0x00,	// 7

#define FUNC_SLOW_CPU	0x14
#define FUNC_SINCLAIR	0x17
#define FUNC_MESSAGE 	0x27
#define FUNC_ENTER   	0x25

#else
//     0     1     2     3     4     5     6     7
	0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 1
	0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 2
	0x11, 0x23, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00,	// 3
	0x17, 0x21, 0x19, 0x2f, 0x00, 0x00, 0x00, 0x00,	// 4
	0x29, 0x22, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00,	// 5
	0x00, 0x25, 0x00, 0x12, 0x18, 0x29, 0x00, 0x00,	// 6
	0x14, 0x24, 0x16, 0x00, 0x00, 0x00, 0x15, 0x00,	// 7
	/*
		KN 1 2 3 4 5 6 7 8 9	// 11...19
		KO C = + - x / E . 0	// 21...29
		KO C v + - / x ^ E 0	// 21...29	(sinclair scientific)
	*/
//     0     1     2     3     4     5     6     7
	0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 2
	0x11, 0x23, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00,	// 3
	0x17, 0x26, 0x19, 0x2f, 0x00, 0x00, 0x00, 0x00,	// 4
	0x22, 0x24, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00,	// 5
	0x00, 0x21, 0x00, 0x12, 0x18, 0x29, 0x00, 0x00,	// 6
	0x14, 0x25, 0x16, 0x00, 0x00, 0x00, 0x15, 0x00,	// 7

#define FUNC_SLOW_CPU	0x2f
#define FUNC_SINCLAIR	0x21
#define FUNC_MESSAGE 	0x24
#define FUNC_ENTER   	0x26

#endif
};


static volatile uint16_t ticks = 0;
static volatile uint8_t clicks = 1;
static volatile uint8_t buzz = 0;
static volatile uint8_t scancode = 0x00;

//________________________________________________________________________________
#define ST_BLINK	BIT1
#define ST_MINUS	BIT2

uint8_t g_state=0;
//______________________________________________________________________
uint8_t get_key() {
	uint8_t c = 0;
	if (scancode) { 
		c = scancode;
		scancode = 0;
		buzz = 6;
	}//if
	return c;
}

volatile uint8_t output[8];
volatile uint8_t pos=0, stays=0x40, blink=0; 
//________________________________________________________________________________
void led_clear() {
	uint8_t i;
	for (i=0;i<8;i++) output[i] = 0x00;
}

//__________________________________________________
void paradiso_loop() {
	//_______ these are for keyboard scanning
	static uint8_t key_row=7, key_col=7;
	//________ stays... lsb = cycles to stay on, msb = cycles to stay off (dark)
	//         0x43 means stay on for 4 cycles, then stay off for 2 cycles
	if (stays & 0xf0) { 
		stays -= 0x10;
		if (!(stays & 0xf0)) {		// setup to use "dark" cycles for key scan
			P2DIR = 0x00;
			P2REN = P2OUT = 0xff;	// be inputs w/ pull-ups
			P1DIR = 0xff;
			P1REN = P1OUT = 0x00;
		}//if
		return;
	}//if
	if (stays) {
		stays--;
		//return;
		// no msb, that means we are in the off cycles
		//____ do scan on another column
		key_col--;

		uint8_t row_bit = 1<<key_row;
		//______ key_row be output low
		P2REN &= ~row_bit;
		P2OUT &= ~row_bit;
		P2DIR |= row_bit;

		//______ check key_col, if low means button depressed
		if (!(P2IN&(1<<key_col))) {
			stays = 0x0e;	// key pressed, mark stay w/ 0x0e so we will visit again
			key_col++;		// and have the same table (row / col)
		}//if
		else {
			if (stays == 0x0d) {		
				// key released (was pressed before)
				// translate to more managable code
				scancode = ((key_row-1)<<3) | key_col;
				if (_sinclair) scancode += (7*8);
				scancode = hwkey_map[scancode];
			}//if
		}//else

		P2DIR &= ~row_bit;
		P2OUT |= row_bit;
		P2REN |= row_bit;

		if (!key_col) {		// last col done, advance row
			key_row--;
			if (key_row) {
				key_col = key_row;
			}//if
			else {
				key_row = key_col = 7;
			}//else
		}//if
		return;
	}//if

	if (pos==9) {		// we have only 8 digits display, negative sign via single led
		if (g_state&ST_MINUS) {
			P1OUT = BIT3;
			P1REN = 0x00;
			P1DIR = BIT2|BIT3;
			stays = 0x1c;
		}//if
		pos = 0;
		return;
	}//if

	if ((ticks&(1<<11)) || !(blink&(1<<pos))) {
		uint8_t seg = output[pos];
		P2OUT = ~digit2p2[pos];
		P2REN = 0;
		P2DIR = digit2p2[pos];

		P1OUT = 0xff;
		P1REN = ~seg;
		P1DIR = seg;

		stays = 1;
		while (seg) {
			if (seg&0x01) stays++;
			seg >>= 1;
		}//while
		stays <<= 4;
		stays++;
	}//if

	pos++;
	return;

}

//________________________________________________________________________________
void rotate_msg(uint8_t const *ptr) {
	uint8_t i=0;
	while (*(ptr+7) != 0xff) {
		for (i=0;i<8;i++) output[i] = segs2port1[*(ptr+i)];
		clicks = 50;
		while (clicks) asm("nop");
		ptr++;
	}//while
	clicks = 200; while (clicks) asm("nop");
	//while (!get_key()) asm("nop");
}

//________________________________________________________________________________
int main(void) {
	WDTCTL = WDTPW + WDTHOLD;
	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL  = CALDCO_8MHZ;

	FCTL2 = FWKEY + FSSEL0 + FN1;	// MCLK/3 for Flash Timing Generator

	TA0CCR0  = 800;		// 10khz
	TA0CCTL0 = CCIE;
	TA0CTL = TASSEL_2 + MC_1;                  // SMCLK, upmode

	P1SEL  = 0;
	P2SEL  = 0;

	_BIS_SR(GIE);

	load_rom(0);			// default ROM is TI Datamath 2500II
	led_clear();
	// !!!terminate w/ 0xff, NOT null terminated
	static const uint8_t msg_slow[] = {
		POS_S, POS_L, POS_o, POS_w, POS__, POS_C, POS_P, POS_u, 0xff,
	};
	static const uint8_t msg_sinclair[] = {
		POS_S, POS_I, POS_n, POS_C, POS_L, POS_A, POS_I, POS_r, 0xff,
	};

	uint8_t i=0;
	char *flash = (char*) 0x1040;				// infomem
	uint8_t greetings[9], *p = greetings;		// load personalized message from flash
	for (i=0;i<8;i++) *p++ = *flash++;
	greetings[8] = 0xff;

	//_______________ code to dectect press-hold-reset
	while (ticks<100) asm("nop");
	if (stays == 0x0e) {
		while (stays == 0x0e) asm("nop");
		switch (get_key()) {
			case FUNC_SLOW_CPU: 	// 1/8 speed to show cpu operation
				rotate_msg(msg_slow);
				BCSCTL1 = CALBC1_1MHZ;
				DCOCTL  = CALDCO_1MHZ;
				TA0CCR0  = 400;
				break;
			case FUNC_SINCLAIR:
				rotate_msg(msg_sinclair);
				load_rom(1);		// load alternate ROM 'sinclair scientific'
				break;
			case FUNC_MESSAGE:		// personal message
				rotate_msg(greetings);
				break;
			//case 0x23: break;
			case FUNC_ENTER:		// enter greeting text and flash
				{
					const uint8_t mkeys[] = {
						POS_A,
						POS_d,
						POS_g,
						POS_J,
						POS_m,
						POS_P,
						POS_t,
						POS_w,
						POS__,
					};
					for (i=0;i<8;i++) output[i] = segs2port1[greetings[i]];
					uint8_t k=0, last_k=0, cpos=0;
					blink = 1;
					while (cpos<8) {
						if ((k=get_key())) {
							switch (k) {
								case 0x23:
									cpos++;
									blink <<= 1;
									break;
								case 0x29:
									greetings[cpos] = greetings[cpos] ? 0 : POS__;
									break;
								default:
									{
									if (k>0x10 && k<0x1a) {
										uint8_t v = k - 0x10;
										if (v>1) {
											if (last_k == k) {
												if (greetings[cpos]<10) {
													greetings[cpos] = mkeys[v-2];
												}//if
												else {
													greetings[cpos]++;
												}//else
												if (greetings[cpos] >= mkeys[v-1])
													greetings[cpos] = v;
											}//if
											else {
												greetings[cpos] = v;
											}//else
										}//if
										else {
											greetings[cpos] = v;
										}//else
									}//if
									}
									break;
							}//switch
							last_k = k;
							output[cpos] = segs2port1[greetings[cpos]];
						}//if
					}//while
					//___________ flash example
					flash = (char*) 0x1040;
					char *src   = (char*) greetings;
					FCTL1 = FWKEY + ERASE;
					FCTL3 = FWKEY;
					*flash = 0x0000;
					FCTL1 = FWKEY + WRT;
					for (i=0;i<8;i++) *flash++ = *src++;
					FCTL1 = FWKEY;
					FCTL3 = FWKEY + LOCK;
					//
				}
			default: 
				break;
		}//switch
		//while (1) {};
	}//if
	//

	led_clear();
	uint8_t first=1;

	uint32_t last_show = 0xffffffff;
	uint8_t last_dot=0;//, spin=50;
	uint8_t key_presist=0;

	uint8_t c = 0x21;

	while (1) {
		if (c || (c=get_key())) {
			if (first) {
				first = 0;
			}//if
			uint8_t next = 0;
			switch (c) {
				case 0x21: 		// reset cpu ('c' key from calculator)
					load_rom(0);
					next = 0x30;
					break;
				case 0x30:		// stepping cpu, check hwkey
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
					step();
					next = 0x30;
					break;
				default: 
					if (c < 0x30) {			// calculator keys
						key_presist = 100;	// emulate bouncing delay
						_keyPressed = c;
						next = 0x30;
					}//if
					else {
					}//else
					break;
			}//switch

			//___________________ refresh display
			if (1==1) {	
				uint8_t i, dot=0;
				uint32_t show=0;
				for (i=1;i<9;i++) {
					show <<= 4;
					show += RegA[i];
					if (RegB[i]) dot = i;
				}//for
				if (show != last_show || dot != last_dot) {
					led_clear();
					if (_sinclair) {
						output[0] = segs2port1[RegA[4]];
						output[1] = segs2port1[RegA[5]];
						output[2] = segs2port1[RegA[6]];
						output[3] = segs2port1[RegA[7]];
						output[4] = segs2port1[RegA[8]];
						output[5] = segs2port1[RegA[1] ? POSmn : POS__];
						output[6] = segs2port1[RegA[2]];
						output[7] = segs2port1[RegA[3]];
						output[0] |= SEG_d_P1;

						if (RegA[0] == 5)
							g_state |= ST_MINUS;
						else
							g_state &= ~ST_MINUS;
					}//if
					else {
						uint8_t suppress_zero_at = dot;
						uint8_t cp=0;
						for (i=1;i<9;i++) {
							if (i<suppress_zero_at && !RegA[i]) {
								output[cp++] = 0x00;
							}//if
							else {
								if (RegA[i] >= 10)
									output[cp++] = segs2port1[POSmn];
								else
									output[cp++] = segs2port1[RegA[i]];
								suppress_zero_at = 0;
							}//else
						}//for
						if (RegA[0] >= 10)
							g_state |= ST_MINUS;
						else
							g_state &= ~ST_MINUS;
						output[dot-1] |= SEG_d_P1;
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

//________________________________________________________________________________
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0_iSR(void) {
	ticks++;
	if (!(ticks%100)) {
		if (clicks) clicks--;
	}//if
	paradiso_loop();
}
