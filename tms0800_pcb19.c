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

	MMCU=msp430g2402
	DMCU=G2402

	GCCBIN=$HOME/ti/msp430-gcc/bin
	INCL="-I $HOME/ti/msp430-gcc/include -L $HOME/ti/msp430-gcc/include"

	$GCCBIN/msp430-elf-gcc -Os -Wall -ffunction-sections -fdata-sections -fno-inline-small-functions -Wl,-Map=$PRG.map,--cref -Wl,--relax -Wl,--gc-sections,--section-start=.infomem=0x01040 $INCL -mmcu=$MMCU -o $PRG.elf $PRG.c
	-Wl,--gc-sections,--section-start=.infomem=0x01040

	$GCCBIN/msp430-elf-objdump -DS $PRG.elf > $PRG.lst
	$GCCBIN/msp430-elf-strip $PRG.elf
	$GCCBIN/msp430-elf-size --totals $PRG.elf
	$GCCBIN/msp430-elf-objcopy -O ihex $PRG.elf $PRG.hex


  September 2013 initial check-in
  June 2014 
	. pcb design
	. employs 2 x 4 digit bubble led
	. mutilplexing w/ 8 col x 8 row, no more charlieplexing
	. remove uart dis-assembler
  May 2015
    . now does LPM4 auto-off, can eliminate spdt slide switch on pcb
	. auto-off time is 60 seconds of in-activity. 'C' key to swith on
	. move setup function from press-n-hold to 'C' + 5, 6, 7, 8, 9 keys
	. setup is pressing 'C' and in very quick succession one of 6, 7, 8, 9 keys
	. C+5 = button test ('C'+'C' to end test)
	. C+6 = secret message edit (+ key to advance)
	. C+7 = toggle datamath / sinclair roms
	. C+8 = toggle slow / fast cpu
	. C+9 = show secret message
  Nov 2019
    . migrate to new pcb employing 3 x 3 digit display, full 9 digits
	. tactile buttons now uses P1 instead of P2
*/


//#include "../local/isr_compat.h"
#include <msp430.h>

#include <stdint.h>
#define G2452

#define V2

#include "led_pcb19.h"
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
/*	 [2c]  [2d]					2019 layout raw
	7[28] 8[29] 9[2a]xC[2b]
	4[21] 5[22] 6[23]/-[24]
	1[19] 2[1a] 3[1b]++[20]
   V0[10] 0[11]E.[12]-=[18]
*/
//     0     1     2     3     4     5     6     7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 2
	0x22, 0x29, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00,	// 3
	0x24, 0x11, 0x12, 0x13, 0x00, 0x00, 0x00, 0x00,	// 4
	0x23, 0x14, 0x15, 0x16, 0x26, 0x00, 0x00, 0x00,	// 5
	0x17, 0x18, 0x19, 0x25, 0x27, 0x21, 0x00, 0x00,	// 6
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 7
	/*
		KN 1 2 3 4 5 6 7 8 9	// 11...19
		KO C = + - x / E . 0	// 21...29
		KO C v + - / x ^ E 0	// 21...29	(sinclair scientific)
	*/
//     0     1     2     3     4     5     6     7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 2
	0x22, 0x29, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00,	// 3
	0x24, 0x11, 0x12, 0x13, 0x00, 0x00, 0x00, 0x00,	// 4
	0x23, 0x14, 0x15, 0x16, 0x25, 0x00, 0x00, 0x00,	// 5
	0x17, 0x18, 0x19, 0x26, 0x27, 0x21, 0x00, 0x00,	// 6
	0x11, 0x23, 0x13, 0xff, 0x00, 0x00, 0x12, 0x00,	// 7

#define FUNC_SLOW_CPU	0x14
#define FUNC_SINCLAIR	0x17
#define FUNC_MESSAGE 	0x27
#define FUNC_ENTER   	0x25
#define FUNC_SETUP   	0x26

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

#define SECS_TO_SLEEP	60*60/48

static volatile uint16_t ticks = 0;
static volatile uint8_t clicks = SECS_TO_SLEEP;
static volatile uint8_t scancode = 0x00;
static volatile uint8_t last_sc = 0;

//________________________________________________________________________________
#define ST_BLINK	BIT1
#define ST_SINCLAIR	BIT3
#define ST_SLOW    	BIT4
#define ST_FLASH	BIT5
#define ST_SLEEP	BIT6

uint8_t g_state=0;
//______________________________________________________________________
uint8_t get_key() {
	uint8_t c = 0;
	if (!clicks) c = 0x40;		// timeout
	
	if (scancode) { 
		if (clicks >= (SECS_TO_SLEEP - 1) &&
			//last_sc == (g_state&ST_SINCLAIR ? 0x26 : 0x21)) {
			last_sc == 0x21) {
			if (last_sc == scancode) {
				last_sc = scancode = 0;
				c = 0x40; // turn off
			}//if
			else {
				if (scancode >= 0x15 && scancode <= 0x19) {
					//________ shifted function 5-9
					c = 0x30 + scancode;
					last_sc = scancode = 0;
				}//if
			}//else
		}//if
		if (!c) {
			last_sc = c = scancode;
			scancode = 0;
			clicks = SECS_TO_SLEEP;
		}//if
	}//if
	return c;
}

volatile uint8_t output[9];
volatile uint8_t stays=0x40;
volatile uint16_t blink=0; 
volatile int8_t pos=0;
//________________________________________________________________________________
void led_clear() {
	uint8_t i;
	for (i=0;i<9;i++) output[i] = 0x00;
}

//__________________________________________________
void paradiso_loop() {
	//_______ these are for keyboard scanning
	if (((g_state&ST_SLOW) && !(ticks%0x07ff)) || !(ticks&0x1fff)) {
		if (clicks) clicks--;
		ticks = 0;
	}//if
	//_______ these are for keyboard scanning
	static uint8_t key_row=6, key_col=6;
	//________ stays... lsb = cycles to stay on, msb = cycles to stay off (dark)
	//         0x43 means stay on for 4 cycles, then stay off for 2 cycles
	if (stays & 0xf0) { 
		stays -= 0x10;
		if (!(stays & 0xf0)) {		// setup to use "dark" cycles for key scan
			P2DIR = 0x00;
			P2REN = ~0x00;
            P2OUT = ~0x00;
			P1DIR = 0x00;
			P1REN = ~0x00;
            P1OUT = 0x00;
            /*
			P1DIR = 0x00;
			P1REN = 0xff;
			P1OUT = 0x00;	// be inputs w/ pull-ups
			P2DIR = 0xff;
			//P2REN = P2OUT = 0x00;
			P2REN = 0xff;
			P2OUT = 0xff;
            */
		}//if
		else
		return;
	}//if
	if (stays) {
		stays--;
		//return;		// debug no keyscan
		// no msb, that means we are in the off cycles
		//____ do scan on another column
		key_col--;

		uint8_t row_bit = 1<<key_row;
		//______ key_row be output low
		P1REN &= ~row_bit;
		P1OUT |= row_bit;
		P1DIR |= row_bit;

		//______ check key_col, if low means button depressed
		if ((P1IN&(1<<key_col))) {
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

		P1DIR &= ~row_bit;
		P1OUT &= ~row_bit;
		P1REN |= row_bit;

		if (!key_col) {		// last col done, advance row
			key_row--;
			if (key_row) {
				key_col = key_row;
			}//if
			else {
				key_row = key_col = 6;
			}//else
		}//if
		return;
	}//if

	//if ((ticks&(1<<11)) || !(blink&(1<<pos))) {
	if (!(blink&(1<<pos)) ||
	((g_state&ST_SLOW) && ticks&(1<<9)) ||
	(!(g_state&ST_SLOW) && ticks&(1<<11))) {
		uint8_t seg = output[pos];

		//P1REN = ~seg;
		//P1DIR = seg;
		//P1OUT = 0xff;
		if (pos == 0) {
			P2OUT = 0x00;
			P2REN = ~0x00;
			P2DIR = 0x00;
			P1OUT = seg;
            P1REN = 0x00;
			P1DIR = BIT7|SEG_G_P1;
		}//if
		else {
            P1REN = ~seg;
            P1DIR = seg;
            P1OUT = seg;
			P2OUT = ~digit2p2[pos-1];
			P2REN = 0;
			P2DIR = digit2p2[pos-1];
		}//else

		stays = 1;
		while (seg) {
			if (seg&0x01) stays++;
			seg >>= 1;
		}//while
		stays <<= 4;
		stays++;
	}//if

	pos++;
	if (pos == 9) pos = 0;
	return;

}

//________________________________________________________________________________
void show_msg(uint8_t const *ptr) {
	uint8_t i=0;
	for (i=0;i<9;i++) output[i] = segs2port1[*(ptr+i)];
	clicks = 3; while (clicks && !get_key()) __asm(" nop");
}

void __attribute__ ((section (".infomem"))) infomem() {
	__asm(
        ".byte 0x24, 0x1d, 0x11, 0x0a, 0x17, 0x14, 0x22, 0x18, 0x1e, 0x08,\n"
	);
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


	// !!!terminate w/ 0xff, NOT null terminated
	static const uint8_t msg_slow[] = {
		POS__, POS_S, POS_L, POS_o, POS_w, POS__, POS_C, POS_P, POS_U, 0xff,
	};
	static const uint8_t msg_fast[] = {
		POS__, POS_F, POS_A, POS_S, POS_t, POS__, POS_C, POS_P, POS_U, 0xff,
	};
	static const uint8_t msg_sinclair[] = {
		POS__, POS_S, POS_I, POS_n, POS_C, POS_L, POS_A, POS_I, POS_r, 0xff,
	};
	static const uint8_t msg_datamath[] = {
		POS__, POS_d, POS_A, POS_t, POS_A, POS_m, POS_A, POS_t, POS_h, 0xff,
	};

	uint8_t i=0;
	char *flash = (char*) 0x1040;				// infomem
	uint8_t greetings[10], *p = greetings;		// load personalized message from flash
	for (i=0;i<10;i++) *p++ = *flash++;
	g_state = greetings[9] & (ST_SINCLAIR|ST_SLOW);			// load only init options
	greetings[9] = 0xff;

	load_rom(g_state&ST_SINCLAIR ? 1 : 0);		// set default ROM and cpu speed
	if (g_state&ST_SLOW) {
		BCSCTL1 = CALBC1_1MHZ;
		DCOCTL  = CALDCO_1MHZ;
		TA0CCR0  = 400;
	}//if

	led_clear();
	//_______________ code to dectect press-hold-reset
	//while (ticks<100) asm("nop");
	if (1==2 && stays == 0x0e) {
		while (stays == 0x0e) __asm(" nop");
		switch (get_key()) {
			case FUNC_SLOW_CPU: 	// 1/4 speed to show cpu operation
				show_msg(g_state&ST_SLOW ? msg_fast : msg_slow);
				g_state ^= ST_SLOW;
				g_state |= ST_FLASH;
				//BCSCTL1 = CALBC1_1MHZ;
				//DCOCTL  = CALDCO_1MHZ;
				//TA0CCR0  = 400;
				break;
			case FUNC_SINCLAIR:
				show_msg(g_state&ST_SINCLAIR ? msg_datamath : msg_sinclair);
				g_state ^= ST_SINCLAIR;
				g_state |= ST_FLASH;
				//load_rom(1);		// load alternate ROM 'sinclair scientific'
				break;
			case FUNC_MESSAGE:		// personal message
				show_msg(greetings);
				break;
			case FUNC_SETUP:		// set default startup
				{
					output[0] = segs2port1[POS_P];
					output[1] = segs2port1[POS_r];
					output[2] = segs2port1[POS_E];
					output[3] = segs2port1[POS_S];
					output[4] = segs2port1[POS_S];
					uint8_t c=0;
					while (1) {
						if ((c=get_key())) {
							output[6] = segs2port1[c>>4];
							output[7] = segs2port1[c&0x0f];
						}//if
					}//while
					//g_state |= ST_FLASH;
				}
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
                    greetings[0] = POS__;
					for (i=0;i<9;i++) output[i] = segs2port1[greetings[i]];
					uint8_t k=0, last_k=0, cpos=1;
					blink = 2;
					while (cpos<9) {
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
					g_state |= ST_FLASH;
				}
			default: 
				break;
		}//switch
		if (g_state&ST_FLASH) {
			//___________ write flash
			flash = (char*) 0x1040;
			char *src   = (char*) greetings;
			FCTL1 = FWKEY + ERASE;
			FCTL3 = FWKEY;
			*flash = 0x0000;
			FCTL1 = FWKEY + WRT;
			for (i=0;i<9;i++) *flash++ = *src++;
			*flash = g_state;
			FCTL1 = FWKEY;
			FCTL3 = FWKEY + LOCK;
		}//if
		WDTCTL = 0;		// s/w reset
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
				case 0x40:
					//_____ shutdown and allow P2.3-4 'C' button to boot
					CCTL0 &= ~CCIE;
					P2REN = 0xff;		// pull-down + input in all pins
					P2OUT = 0xff;
					P2DIR = 0x00;
					P1REN = ~BIT6;		// except P2.3
					//P2REN = 0xff;
					P1OUT = BIT6|BIT7;	// BIT7 is decimal + digit 0 share
					P1DIR = BIT6;
					//P2DIR = 0x00;
					//_BIC_SR(GIE);
					_BIC_SR(GIE);
					P1IES |= BIT5;		// hi-low trip
					P1IFG &= ~BIT5;
					P1IE  |= BIT5;
					_BIS_SR(LPM4_bits + GIE);
					P1IE  &= ~BIT5;

					clicks = SECS_TO_SLEEP;
					CCTL0 |= CCIE;
					stays=0x40;
					break;
				case 0x45:
				{
					output[0] = segs2port1[POS__];
					output[1] = segs2port1[POS_P];
					output[2] = segs2port1[POS_r];
					output[3] = segs2port1[POS_E];
					output[4] = segs2port1[POS_S];
					output[5] = segs2port1[POS_S];
					uint8_t c=0;
					while (1) {
						if ((c=get_key())) {
							if (c == 0x40) WDTCTL = 0;
							output[7] = segs2port1[c>>4];
							output[8] = segs2port1[c&0x0f];
						}//if
					}//while
				}
					break;
				case 0x46:
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
					for (i=0;i<9;i++) output[i] = segs2port1[greetings[i]];
					uint8_t k=0, last_k=0, cpos=1;
					blink = 2;
					while (cpos<9) {
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
					g_state |= ST_FLASH;
				}
					break;
				case 0x47:
					show_msg(g_state&ST_SINCLAIR ? msg_datamath : msg_sinclair);
					g_state ^= ST_SINCLAIR;
					g_state |= ST_FLASH;
					break;
				case 0x48:
					show_msg(g_state&ST_SLOW ? msg_fast : msg_slow);
					g_state ^= ST_SLOW;
					g_state |= ST_FLASH;
					break;
				case 0x49:
					show_msg(greetings);
					WDTCTL = 0;		// s/w reset
					break;
				case 99:
					infomem();
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

			if (g_state&ST_FLASH) {
				//___________ write flash
				flash = (char*) 0x1040;
				char *src   = (char*) greetings;
				FCTL1 = FWKEY + ERASE;
				FCTL3 = FWKEY;
				*flash = 0x0000;
				FCTL1 = FWKEY + WRT;
				for (i=0;i<9;i++) *flash++ = *src++;
				*flash = g_state;
				FCTL1 = FWKEY;
				FCTL3 = FWKEY + LOCK;
				WDTCTL = 0;		// s/w reset
			}//if

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
					clicks = SECS_TO_SLEEP;		// display changing, don't go to sleep
					led_clear();
					if (_sinclair) {
						output[0] = segs2port1[RegA[0] ? POSmn : POS__];		// 5..neg
						output[1] = segs2port1[RegA[4]];
						output[2] = segs2port1[RegA[5]];
						output[3] = segs2port1[RegA[6]];
						output[4] = segs2port1[RegA[7]];
						output[5] = segs2port1[RegA[8]];
						output[6] = segs2port1[RegA[1] ? POSmn : POS__];
						output[7] = segs2port1[RegA[2]];
						output[8] = segs2port1[RegA[3]];
						output[1] |= SEG_d_P1;
					}//if
					else {
						uint8_t suppress_zero_at = dot;
						uint8_t cp=0;
						for (i=0;i<9;i++) {
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
						output[dot] |= SEG_d_P1;
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
//#pragma vector=TIMER0_A0_VECTOR __interrupt
__attribute__((interrupt(TIMER0_A0_VECTOR)))
void Timer0_A0_iSR(void) {
	ticks++;
	paradiso_loop();
}

//#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
	P1IFG &= ~BIT5;
	_BIC_SR_IRQ(LPM4_bits); } 
