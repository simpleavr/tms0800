/*

               MSP430G2452
             -----------------    
         /|\|                 |  
          | |                 | 
          --|RST              |
            |                 |       3 x 3 digit bubble led
            |     digit 0 P1.7|----- +-------+ +-------+ +-------+
            |     digit 1 P2.0|----- | % % % | | % % % | | % % % |
            |     digit 2 P2.1|----- +-------+ +-------+ +-------+
            |     digit 3 P2.2|--/-
            |     digit 4 P2.3|--/-
            |     digit 5 P2.4|--/-                                            
            |     digit 6 P2.5|--/-   _ _                                      
            |     digit 7 P2.6|--/-   ^ ^  segment a to g + dot........
            |                 |       /                                 ....\
            |                 |      /                                       \
            |   segment A P1.0|-----+-----+-----+-----+-----+-----+-----+-----+
            |                 | _=_ | _=_ | _=_ | _=_ | _=_ | _=_ | _=_ | _=_ |
            |   segment B P1.1|-o o-+-o o-+-o o-+-o o-+-o o-+-o o-+-o o-+-o o-+
            |                 | _=_ | _=_ | _=_ | _=_ | _=_ | _=_ | _=_ |
            |   segment c P1.2|-o o-+-o o-+-o o-+-o o-+-o o-+-o o-+-o o-+
            |                 | _=_ | _=_ | _=_ | _=_ | _=_ | _=_ |
            |   segment D P1.3|-o o-+-o o-+-o o-+-o o-+-o o-+-o o-+
            |                 | _=_ | _=_ | _=_ | _=_ | _=_ |
            |   segment E P1.4|-o o-+-o o-+-o o-+-o o-+-o o-+
            |                 | _=_ | _=_ | _=_ | _=_ |
            |   segment F P1.5|-o o-+-o o-+-o o-+-o o-+
            |                 | _=_ | _=_ | _=_ |
            |   segment G P1.6|-o o-+-o o-+-o o-+
            |                 | _=_ |
            |   segment H P1.7|-o o-+		(not all buttons populated)
            |                 |  
             -----------------


    mapping of buttons. Ex. 70 .. button across column 0 and 7 pins

	[  ]  [  ]  [  ]  [  ]
	[  ]  [  ]  [  ]  [  ]
	[10]  [61]  [21]  [43]

	[40]  [64]  [42]  [41]

	[70]  [76]  [72]  [71]

	[30]  [63]  [32]  [31]

	[50]  [65]  [52]  [51]
*/

#define SEG_A_P1	(1<<0)
#define SEG_B_P1	(1<<1)
#define SEG_C_P1	(1<<2)
#define SEG_D_P1	(1<<3)
#define SEG_E_P1	(1<<4)
#define SEG_F_P1	(1<<5)
#define SEG_G_P1	(1<<6)
#define SEG_d_P1	(1<<7)
#define DIGIT_0_P1	0x00
#define DIGIT_1_P1	0x00
#define DIGIT_2_P1	0x00
#define DIGIT_3_P1	0x00
#define DIGIT_4_P1	0x00
#define DIGIT_5_P1	0x00
#define DIGIT_6_P1	0x00
#define DIGIT_7_P1	0x00

#define SEG_A_P2	0x00
#define SEG_B_P2	0x00
#define SEG_C_P2	0x00
#define SEG_D_P2	0x00
#define SEG_E_P2	0x00
#define SEG_F_P2	0x00
#define SEG_G_P2	0x00
#define SEG_d_P2	0x00

#define DIGIT_0_P2	(1<<0)
#define DIGIT_1_P2	(1<<1)
#define DIGIT_2_P2	(1<<2)
#define DIGIT_3_P2	(1<<3)
#define DIGIT_4_P2	(1<<4)
#define DIGIT_5_P2	(1<<5)
#define DIGIT_6_P2	(1<<6)
#define DIGIT_7_P2	(1<<7)

#define SEGS_BIT_P1(v) \
   (((v & (1<<7)) ? 7 : 0) +\
    ((v & (1<<6)) ? 6 : 0) +\
	((v & (1<<5)) ? 5 : 0) +\
	((v & (1<<4)) ? 4 : 0) +\
	((v & (1<<3)) ? 3 : 0) +\
	((v & (1<<2)) ? 2 : 0) +\
	((v & (1<<1)) ? 1 : 0) +\
	((v & (1<<0)) ? 0 : 0))

#define SEGS_BIT_P2(v) \
   (((v & (1<<7)) ? (7+8) : 0) +\
    ((v & (1<<6)) ? (6+8) : 0) +\
	((v & (1<<5)) ? (5+8) : 0) +\
	((v & (1<<4)) ? (4+8) : 0) +\
	((v & (1<<3)) ? (3+8) : 0) +\
	((v & (1<<2)) ? (2+8) : 0) +\
	((v & (1<<1)) ? (1+8) : 0) +\
	((v & (1<<0)) ? (0+8) : 0))


// calcuate number of segments on individual digits, letters show
// will use to decide how long a digit / letter stays "on"
#define SEGS_STAY(v) \
   (((v & (1<<6)) ? 1 : 0) +\
	((v & (1<<5)) ? 1 : 0) +\
	((v & (1<<4)) ? 1 : 0) +\
	((v & (1<<3)) ? 1 : 0) +\
	((v & (1<<2)) ? 1 : 0) +\
	((v & (1<<1)) ? 1 : 0) +\
	((v & (1<<0)) ? 1 : 0)) | 0x20

// macro magic
// what the io ports output for individual digits / letters
// we do this at compile time so that we don't need to use runtime cycles
// to map segment and port pins
#define SEGS_PORT_DET(p, v) \
   (((v & (1<<6)) ? SEG_A_P##p : 0) |	\
	((v & (1<<5)) ? SEG_B_P##p : 0) |	\
	((v & (1<<4)) ? SEG_C_P##p : 0) |	\
	((v & (1<<3)) ? SEG_D_P##p : 0) |	\
	((v & (1<<2)) ? SEG_E_P##p : 0) |	\
	((v & (1<<1)) ? SEG_F_P##p : 0) |	\
	((v & (1<<0)) ? SEG_G_P##p : 0))

#define SEGS_PORT(v)	{SEGS_STAY(v),SEGS_PORT_DET(1, v),SEGS_PORT_DET(2, v)}
// port 1/2 pins used to turn segments on, led anodes
#define SEGS_1 (SEG_A_P1|SEG_B_P1|SEG_C_P1|SEG_D_P1|SEG_E_P1|SEG_F_P1|SEG_G_P1|SEG_d_P1)
#define SEGS_2 (SEG_A_P2|SEG_B_P2|SEG_C_P2|SEG_D_P2|SEG_E_P2|SEG_F_P2|SEG_G_P2|SEG_d_P2)

// port 1/2 pins used turn digits on, led cathodes
#define DIGITS_1 (DIGIT_0_P1|DIGIT_1_P1|DIGIT_2_P1|DIGIT_3_P1)
#define DIGITS_2 (DIGIT_0_P2|DIGIT_1_P2|DIGIT_2_P2|DIGIT_3_P2)

// port 1/2 pins used
#define USED_1 (SEGS_1|DIGITS_1)
#define USED_2 (SEGS_2|DIGITS_2)

/*
       ___a__
      |      |
     f|      | b
       ___g__
     e|      | c
      |      |
       ___d__

*/
// composition of digits and letters we need
//______________ abcdefg
#define LTR_0 0x7e	// 0111 1110
#define LTR_1 0x30	// 0011 0000
#define LTR_2 0x6d	// 0110 1101
#define LTR_3 0x79	// 0111 1001
#define LTR_4 0x33	// 0011 0011
#define LTR_5 0x5b	// 0101 1011
#define LTR_6 0x5f	// 0101 1111
#define LTR_7 0x70	// 0111 0000
#define LTR_8 0x7f	// 0111 1111
#define LTR_9 0x7b	// 0111 1011
#define BLANK 0x00	// 0000 0000

#define BAR_1 0x40	// 0100 0000
#define BAR_2 0x01	// 0000 0001
#define BAR_3 (BAR_1|BAR_2)
#define LTRdg 0x63	// 0110 0011

#define LTR_A 0x77 // 0111 0111
#define LTR_b 0x1f // 0001 1111
#define LTR_C 0x4e // 0100 1110
#define LTR_c 0x0d // 0000 1101
#define LTR_d 0x3d // 0011 1101
#define LTR_E 0x4f // 0100 1111
#define LTR_e 0x6f // 0110 1111
#define LTR_F 0x47 // 0100 0111
#define LTR_G 0x5e // 0101 1110
#define LTR_g 0x7b // 0111 1011	//(as 9)
#define LTR_H 0x37 // 0011 0111	//(dup)
#define LTR_h 0x17 // 0001 0111
#define LTR_I 0x06 // 0000 0110
#define LTR_i 0x02 // 0000 0010
#define LTR_J 0x3c // 0011 1100
#define LTR_K 0x37 // 0011 0111	//(as H)
#define LTR_k 0x0f // 0000 1111	
#define LTR_L 0x0e // 0000 1110
#define LTR_m 0x54 // 0101 0100
#define LTR_N 0x76 // 0111 0110
#define LTR_n 0x15 // 0001 0101
#define LTR_o 0x1d // 0001 1101
#define LTR_P 0x67 // 0110 0111
#define LTR_q 0x73 // 0111 0011
#define LTR_r 0x05 // 0000 0101
#define LTR_S 0x5b // 0101 1011	//(as 5)
#define LTR_t 0x0f // 0000 1111
#define LTR_U 0x3e // 0011 1110
#define LTR_u 0x1c // 0001 1100
#define LTR_N 0x76 // 0111 0110
#define LTR_v 0x1c // 0001 1100	//(as u)
//_____________  abcdefg
#define LTR_w 0x2a // 0010 1010
#define LTR_X 0x37 // 0011 0111	//(as H)
#define LTR_y 0x3b // 0011 1011
#define LTR_Z 0x6d // 0110 1101	//(as 2)



#define LTRml 0x66 // 0110 0110
#define LTRmr 0x72 // 0111 0010
#define LTR__ 0x00 // 0000 0000
#define LTRmn 0x01 // 0000 0001


#define LTRm1 0x08 // 0000 1000
#define LTRm2 0x09 // 0000 1001
#define LTRm3 0x49 // 0100 1001


// port io values for individual digits / letters
// 1st byte cycles to stay
// 2nd byte port 1 value
// 3rd byte port 2 value
/*
static const uint8_t digit2ports[][3] = { 
	SEGS_PORT(LTR_0), SEGS_PORT(LTR_1), SEGS_PORT(LTR_2), SEGS_PORT(LTR_3),
	SEGS_PORT(LTR_4), SEGS_PORT(LTR_5), SEGS_PORT(LTR_6), SEGS_PORT(LTR_7),
	SEGS_PORT(LTR_8), SEGS_PORT(LTR_9), SEGS_PORT(LTR_A), SEGS_PORT(LTR_b), 
	SEGS_PORT(LTR_C), SEGS_PORT(LTR_d), SEGS_PORT(LTR_E), SEGS_PORT(LTR_F), 

	SEGS_PORT(LTR_g), SEGS_PORT(LTR_h), SEGS_PORT(LTR_I), SEGS_PORT(LTR_J), 
	SEGS_PORT(LTR_k), SEGS_PORT(LTR_L), SEGS_PORT(LTR_m), SEGS_PORT(LTR_n), 
	SEGS_PORT(LTR_o), SEGS_PORT(LTR_P), SEGS_PORT(LTR_q), SEGS_PORT(LTR_r), 
	SEGS_PORT(LTR_S), SEGS_PORT(LTR_t), SEGS_PORT(LTR_u), SEGS_PORT(LTR_v), 

	SEGS_PORT(LTR_w), SEGS_PORT(LTR_X), SEGS_PORT(LTR_y), SEGS_PORT(LTR_Z), 
	SEGS_PORT(BLANK), SEGS_PORT(LTRmn), SEGS_PORT(LTRm1), SEGS_PORT(LTRm2), SEGS_PORT(LTRm3), 

};
*/

static const uint8_t digit2p2[] = { 
	DIGIT_0_P2, DIGIT_1_P2, DIGIT_2_P2, DIGIT_3_P2,
	DIGIT_4_P2, DIGIT_5_P2, DIGIT_6_P2, DIGIT_7_P2,
};

static const uint8_t segs2port1[] = { 
	SEGS_PORT_DET(1,LTR_0), SEGS_PORT_DET(1,LTR_1), SEGS_PORT_DET(1,LTR_2), SEGS_PORT_DET(1,LTR_3),
	SEGS_PORT_DET(1,LTR_4), SEGS_PORT_DET(1,LTR_5), SEGS_PORT_DET(1,LTR_6), SEGS_PORT_DET(1,LTR_7),
	SEGS_PORT_DET(1,LTR_8), SEGS_PORT_DET(1,LTR_9), SEGS_PORT_DET(1,LTR_A), SEGS_PORT_DET(1,LTR_b), 
	SEGS_PORT_DET(1,LTR_C), SEGS_PORT_DET(1,LTR_d), SEGS_PORT_DET(1,LTR_E), SEGS_PORT_DET(1,LTR_F), 

	SEGS_PORT_DET(1,LTR_g), SEGS_PORT_DET(1,LTR_h), SEGS_PORT_DET(1,LTR_I), SEGS_PORT_DET(1,LTR_J), 
	SEGS_PORT_DET(1,LTR_k), SEGS_PORT_DET(1,LTR_L), SEGS_PORT_DET(1,LTR_m), SEGS_PORT_DET(1,LTR_n), 
	SEGS_PORT_DET(1,LTR_o), SEGS_PORT_DET(1,LTR_P), SEGS_PORT_DET(1,LTR_q), SEGS_PORT_DET(1,LTR_r), 
	SEGS_PORT_DET(1,LTR_S), SEGS_PORT_DET(1,LTR_t), SEGS_PORT_DET(1,LTR_U), SEGS_PORT_DET(1,LTR_v), 

	SEGS_PORT_DET(1,LTR_w), SEGS_PORT_DET(1,LTR_X), SEGS_PORT_DET(1,LTR_y), SEGS_PORT_DET(1,LTR_Z), 
	SEGS_PORT_DET(1,BLANK), SEGS_PORT_DET(1,LTRmn), SEGS_PORT_DET(1,LTRm1), SEGS_PORT_DET(1,LTRm2), 
	SEGS_PORT_DET(1,LTRm3), 

};

// digits / letters we are using
enum {
	POS_0, POS_1, POS_2, POS_3, POS_4, POS_5, POS_6, POS_7,
	POS_8, POS_9, POS_A, POS_b, POS_C, POS_d, POS_E, POS_F, 

	POS_g, POS_h, POS_I, POS_J, POS_k, POS_L, POS_m, POS_n,
	POS_o, POS_P, POS_q, POS_r, POS_S, POS_t, POS_U, POS_v, 

	POS_w, POS_x, POS_y, POS_z, POS__, POSmn, POSm1, POSm2, POSm3, 
};

