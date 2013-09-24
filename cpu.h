// TI calculator simulator
// Ken Shirriff, http://righto.com/ti
// Based on patent US3934233
//
// This file simulates the CPU chip

const uint16_t _rom[2][320] = { {
	1423, 1407, 1807, 1839, 5, 1755, 531, 1446,
	12, 1992, 1992, 12, 1746, 21, 1757, 533,
	1916, 1693, 12, 1807, 1414, 1855, 1803, 1563,
	1852, 1598, 1693, 542, 1884, 26, 1598, 1432,
	553, 1360, 1376, 545, 1360, 1376, 545, 1646,
	1838, 1400, 1319, 1360, 1376, 616, 2013, 1777,
	564, 1838, 1360, 1297, 1429, 42, 1336, 1836,
	1723, 58, 1644, 2013, 1320, 1329, 1443, 77,
	1444, 589, 1440, 77, 1393, 1350, 1807, 1351,
	1552, 1411, 7, 1329, 1328, 1330, 1443, 594,
	1444, 86, 1412, 1349, 86, 1348, 1755, 670,
	1752, 299, 1868, 1565, 88, 1352, 1415, 1443,
	21, 1411, 19, 1443, 533, 1347, 1416, 19,
	1614, 1429, 545, 1360, 1360, 1376, 40, 1823,
	1360, 1024, 1109, 1101, 1085, 1100, 1099, 1123,
	1117, 1150, 1855, 1954, 1855, 533, 1443, 131,
	1414, 1807, 1411, 1446, 148, 1976, 1976, 1992,
	1992, 533, 1447, 152, 1448, 658, 1855, 1755,
	1855, 533, 1415, 156, 1754, 21, 1744, 21,
	1448, 155, 1565, 1868, 1586, 21, 1806, 1414,
	1552, 1418, 1445, 765, 1442, 253, 1647, 1427,
	173, 1395, 1855, 1615, 1526, 1440, 719, 1430,
	178, 1349, 1441, 181, 1462, 1742, 185, 1855,
	1526, 1693, 1742, 702, 1916, 185, 1565, 192,
	1740, 196, 1855, 1526, 1494, 200, 1660, 201,
	1548, 1398, 1445, 253, 1334, 1413, 253, 1494,
	1414, 211, 1350, 1441, 740, 1583, 1804, 1550,
	1714, 732, 1548, 216, 1948, 1787, 764, 1916,
	1693, 216, 1333, 216, 1787, 762, 1583, 1804,
	1787, 764, 1662, 237, 1333, 1772, 754, 1708,
	1554, 237, 1754, 252, 1900, 1868, 1565, 237,
	1397, 237, 1333, 19, 1615, 1822, 1808, 1752,
	776, 1916, 1693, 264, 1333, 1550, 1565, 825,
	1429, 773, 1755, 786, 1754, 276, 1868, 1565,
	268, 43, 1598, 1414, 1426, 285, 1331, 1583,
	1494, 283, 1462, 1529, 1401, 1347, 1413, 1742,
	517, 1662, 1757, 809, 1693, 1916, 290, 43,
	21, 1552, 5, 1446, 310, 1800, 1688, 1755,
	670, 1754, 161, 1868, 1565, 305, 1916, 1693,
	161, 1446, 21, 1992, 1992, 533, 
	}, {
	1408,1392,1792,1824,1860,1808,1360,1376,
	518,1319,1360,1376,9,1360,1908,1072,
	1083,1075,1121,1129,1073,1069,1051,1840,
	1955,1840,516,1425,552,1430,33,1792,
	1398,1631,1920,1683,34,2003,1540,4,
	1399,1858,1872,1538,4,1329,1335,4,
	1349,1347,4,1443,676,1431,57,1559,
	4,1553,59,1443,677,1839,1632,2018,
	65,2023,1719,72,1730,71,1840,1666,
	1751,587,1840,1754,78,1840,1718,594,
	1924,78,2017,1713,89,1540,130,1844,
	1841,1652,597,130,1730,95,1849,1650,
	114,1443,675,1355,1345,130,1409,1559,
	105,1443,750,1839,1632,1844,2023,1719,
	92,1538,1537,116,1451,1796,791,1908,
	1781,637,1722,1540,120,1940,1786,119,
	1445,820,1754,512,1747,145,1860,1751,
	142,1686,141,1799,1798,1686,1558,132,
	132,1908,1751,662,1686,1686,1558,152,
	1441,614,1392,1334,1408,1750,161,1559,
	159,1568,4,1351,1355,1686,681,1908,
	165,1801,1689,1824,1445,180,1447,179,
	1568,1819,185,1565,1820,1924,2011,1693,
	738,1888,1888,1888,2012,1696,1936,1936,
	1936,1872,1872,1872,2012,1584,1724,1920,
	1920,1920,1445,183,1561,1447,210,1779,
	697,220,1451,727,1732,185,220,1754,
	1844,1764,1844,185,1904,1904,1904,1904,
	1904,130,1447,233,1616,1600,1808,1411,
	100,1451,236,1632,1840,130,1445,251,
	1750,760,1751,759,1908,1686,240,1860,
	1794,1864,1864,1824,2003,1636,1924,1924,
	1700,1431,823,2009,1787,253,1993,2036,
	1723,1920,1920,1920,1920,1588,1844,1445,
	814,1600,1479,1447,765,1572,1796,1806,
	1764,797,1700,1562,280,1571,803,1860,
	1631,1892,280,1807,1443,808,130,10,
	1572,1796,1904,1904,1794,61,1572,1796,
	1803,1652,1844,117,1908,1335,251,1693,
	253,1565,1860,1563,2036,1844,1411,790,
}, };

#define Reg0	regs[0]
#define RegA	regs[1]
#define RegB	regs[2]
#define RegC	regs[3]
#define RegK	regs[4]

#define KS_KN	1
#define KS_KO	2
#define KS_KP	3

int8_t _sinclair = 0;
int8_t regs[5][12];
uint16_t _flgs[3];		// flag A, B and Mask
int8_t _keyStrobe = 0;
int8_t _keyPressed = 0;
uint16_t _instruction = 0;
int8_t _dActive = 1;
char _opCode[6];

const char _opMap[32+16+5+5][6] = {
	"NOP  ", "WAITD", "WAITN", "SFB  ",
	"SFA  ", "SYNC ", "SCAN ", "ZFB  ",
	"ZFA  ", "TFB  ", "TFA  ", "FFB  ",
	"FFA  ", "CF   ", "NOP  ", "EXF  ",

	"AABA ", "AAKA ", "AAKC ", "ABOA ",
	"ABOC ", "ACKA ", "AKCB ", "SABA ",
	"SABC ", "SAKA ", "SCBC ", "SCKC ",
	"CAB  ", "CAK  ", "CCB  ", "CCK  ",

	"AKOA ", "AKOB ", "AKOC ", "EXAB ",
	"SLLA ", "SLLB ", "SLLC ", "SRLA ",
	"SRLB ", "SRLC ", "AKCN ", "AAKAH",
	"SAKAH", "ACKC ", "NOP  ", "NOP  ",

	"BIR  ", "BIS  ", "BKN  ", "BKO  ", "BKP  ",	// branch ops
	"ACBB ", "SCBA ", "SCKB ", "AABC ", "ACBC ", 	// 0805 cpu alternative ops

};

int8_t _cc = 0;
uint16_t *_address = 0;
uint16_t *_base_addr = 0;
uint16_t _branch_addr = 0;
int8_t *_mask = 0;

const int8_t _masks[2][16][12] = { {
    "          7", // M0
    "         4 ", // M1
    "        1  ", // M2
    "       0   ", // M3
    "      0    ", // M4
    "     0     ", // M5
    "    0      ", // M6
    " 0         ", // M7
    "1          ", // M8
    "        000", // M9
    "01         ", // M10 a
    "000000001  ", // M11 b
    "000000000  ", // M12 c
    "         01", // M13 d
    "         00", // M14 e
    "00000000000", // M15 f
	}, {						// sinclair mask below
    "00000000000", // M0
    "5          ", // M1
    "  00       ", // M2
    "    1      ", // M3
    "    0000000", // M4
    "          1", // M5
    "  01       ", // M6
    " 5         ", // M7
    "000000     ", // M8
    "0001       ", // M9
    "    0000001", // M10
    "     1     ", // M11
    "    00005  ", // M12
    "    00001  ", // M13
    "    4      ", // M14
    "    0      ", // M15
}, };


//______________________________________________________________________
void load_rom(uint8_t flip) {
	if (flip) _sinclair ^= 1;
	_base_addr = (uint16_t *) _rom[_sinclair];
	_address = _base_addr;
	uint8_t i,j;
	_flgs[0] = _flgs[1] = _flgs[2] = 0;
	for (i=0;i<5;i++)
		for (j=0;j<12;j++) regs[i][j] = 0;
}

//______________________________________________________________________
uint8_t exec_add(int8_t s1, int8_t s2, int8_t carry, int8_t base) {
	uint8_t res = s1 + s2 + carry;
	if (res >= base) {
		res -= base;
		res |= 0x10;
	}//if
	return res;
}

//______________________________________________________________________
uint8_t exec_sub(int8_t s1, int8_t s2, int8_t borrow, int8_t base) {
	int8_t res = s1 - s2 - borrow;
	if (res < 0) {
		res += base;
		res |= 0x10;
	}//if
	return res;
}

//______________________________________________________________________
uint8_t execf(char const *op) {

	// we handles EXF, CF, TFx, FFx, SFx, ZFx
	uint16_t *flg = _flgs;

	if (*(op+2)=='B') flg++;

	switch (*op) {
		case 'F': *flg ^=  _flgs[2]; break;
		case 'Z': *flg &= ~_flgs[2]; break;
		case 'S': *flg |=  _flgs[2]; break;
		case 'T': if (*flg & _flgs[2]) _cc = 1; break;
		case 'C': if ((_flgs[0]&_flgs[2]) != (_flgs[1]&_flgs[2])) _cc = 1; break;
		case 'E': // exchange
			{
				uint16_t t = _flgs[0];
				_flgs[0] &= ~_flgs[2];
				_flgs[0] |= (_flgs[1] & _flgs[2]);
				_flgs[1] &= ~_flgs[2];
				_flgs[1] |= (t & _flgs[2]);
			}
		default:
			break;
	}//switch

	return 0;

}

//______________________________________________________________________
uint8_t execb(char const *op) {
	uint8_t branch=0;
	_branch_addr = _instruction & 0x1ff;
	switch (*(op+2)) {
		case 'R': if (!_cc) branch = 1; break;
		case 'S': if (_cc) branch = 1; break;
		case 'N': if (_keyStrobe == KS_KN) branch = 1; break;
		case 'O': if (_keyStrobe == KS_KO) branch = 1; break;
		case 'P': if (_keyStrobe == KS_KP) branch = 1; break;
		default: break;
	}//switch
	_cc = 0;
	return branch;
}

//______________________________________________________________________
uint8_t exec(char const *op) {

	if (*op == 'N') return 0;
	if (*op == 'B') return execb(op);
	if (*(op+1) == 'F' || *(op+2) == 'F') return execf(op);

    int8_t carry_borrow = 0;
	int8_t base = 10;
	char const *lp = op + 1;
	int8_t *sp[4]={0,0,0,0}, **pp=&sp[0];

	if (*(op+4) == 'H') base += 6;		// hex operation

	while (*lp) {			// load register pointers accordingly
		switch (*lp) {
			case 'A': *pp = RegA; break;
			case 'B': *pp = RegB; break;
			case 'C': *pp = RegC; break;
			case 'K': *pp = RegK; break;
			case 'X':	// happens in exchange;
			case 'L': 	// happens for shifts, like SRLx, SLLx
			case 'O': *pp = Reg0; break;
			default: break;
		}//switch
		lp++;
		pp++;
	}//while

	uint8_t (*cmdp)(int8_t, int8_t, int8_t, int8_t) = exec_sub;
	switch (*op) {
		case 'E':		// EX
			sp[3] = sp[1];
			base = 16;
		case 'A':
			cmdp = exec_add;
			break;
		default:		// S, C, SLL, SRL
			break;
	}//switch


	uint8_t reverse=0;
	if (*op == 'S' && *(op+2) == 'L') {		// shift operation
		if (*(op+1) == 'R') reverse = 1;
		sp[0] = 0;
	}//if

	int8_t hold=0, t_hold=0;
	int8_t i;
	for (i=10;i>=0;i--) {
		uint8_t id = reverse ? 10 - i : i;
		if (RegK[id] == ' ') continue;
		t_hold = sp[2][id];
		if (sp[3]) {
			sp[2][id] = sp[3][id];
			sp[3][id] = t_hold;
		}//if
		else {
			uint8_t res = (*cmdp)(sp[0] ? sp[0][id] : hold, sp[1][id], carry_borrow, base);
			if (sp[2]) 	// compare and some commands has no destination
				sp[2][id] = res & 0x0f;
			hold = t_hold;		// for shifting
			carry_borrow = res >> 4;
		}//else
	}//for
	if (carry_borrow) _cc = 1;
	if (*(op+4) == 'H') _cc = 0;

	return 0;

}

  const int8_t opsWithK[32] = {
  	0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1,
  	1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0,
  };
//______________________________________________________________________

void step() {
	_instruction = *_address;
	_branch_addr = 0;
	uint16_t *nextAddress = _address + 1;

	// Get the mask vector associated with the current instruction's mask
	// Entries are ' ' if not masked, '*' if masked, n if mask and constant.
	// Note that S10 is in mask[0]
	/*
	if (classBits == 3 || (classBits == 2 && opcode > 18 &&
		opcode != 21 && opcode != 22)) {
		const int8_t *mask = _masks[_sinclair][*_address&0xf];
		_flgs[2] = 0x0000;
		int8_t i;
		for (i=0;i<=10;i++) {
			_flgs[2] <<= 1;
			if (mask[i] == ' ') {
				RegK[i] = mask[i];
			} else {
				_flgs[2] |= 1;
				RegK[i] = mask[i] - '0';
			}//else
		}//for
	}//if
	*/

	/*
	[ 0 0 ] <---------- jump address if condition reset ------->
	[ 0 1 ] <---------- jump address if condition set --------->
	[ 1 0 0 0 ] <------ jump if key-down on KO (0 to 127) ----->
	[ 1 0 0 1 ] <------ jump if key-down on KP (128 to 255) --->
	[ 1 0 1 x x x x m m m m ] <-- flag instr + mask ----------->
	[ 1 1 x x x x x m m m m ] <-- regs instr + mask ----------->
	*/

	uint8_t opClass = _instruction >> 7;

	//_______________ determine mask
	const int8_t *mask = _masks[_sinclair][*_address&0xf];
	_flgs[2] = 0x0000;
	int8_t i;
	for (i=0;i<=10;i++) {
		_flgs[2] <<= 1;
		if (mask[i] == ' ') {
			RegK[i] = mask[i];
		} else {
			_flgs[2] |= 1;
			/*
			int8_t opc = (_instruction >> 4) & 0x1f;
			if ((_instruction >> 9) == 3 && opsWithK[opc]) {
				// Register instruction
				RegK[i] = mask[i] - '0';
			} else {
				RegK[i] = '*';
			}//else
			*/
			RegK[i] = mask[i] - '0';
		}//else
	}//for

	uint8_t opIdx = 0;
#define BRANCH_OP   	0x30
#define SINCLAIR_ALT	0x35

	//"BIR  ", "BIS  ", "BKN  ", "BKO  ", "BKP  ",	// branch ops
	if (opClass & 0x08) {
		if ((opClass&0x0e) == 0x08) {
			opIdx = BRANCH_OP + 3;
			if (opClass&0x01) opIdx++;
			/*
			// Jump if key down on KP/KO (BKP/BKO)
			if (((opClass&0x01) && (_keyStrobe == KS_KP)) || (_keyStrobe == KS_KO))
				nextAddress = _base_addr + (_instruction & 0x1ff);
			_cc = 0;
			*/
		}//if
		else {
			opIdx = _instruction >> 4;
			opIdx &= 0x3f;
			opIdx -= 0x10;

			if (_sinclair) {
				switch (opIdx) {
					case  3+16: opIdx = SINCLAIR_ALT + 0; break;
					case 27+16: opIdx = SINCLAIR_ALT + 1; break;
					case 28+16: opIdx = SINCLAIR_ALT + 2; break;
					case 30+16: opIdx = SINCLAIR_ALT + 3; break;
					case 31+16: opIdx = SINCLAIR_ALT + 4; break;
					default: break;
				}//switch
			}//if


		}//else
	}//if
	else {
		opClass >>= 2;
		opIdx = BRANCH_OP;
		if (opClass) opIdx++;
		// jump if reset: (BIR) BIU, BIZ, BIGE, BINC, BIE, BET
		// jump if set: (BIS) BID, BIO, BILT, BIC, BINE
		/*
		if (_cc == opClass) 
			nextAddress = _base_addr + (_instruction & 0x1ff);
		_cc = 0;
		*/
	}//else

	for (i=0;i<6;i++) _opCode[i] = _opMap[opIdx][i];


	switch (opIdx) {
		case 26+16: // AKCN: A+K -> A until key down on N or D11 [sic]
			// Patent says sets condition if key down, but real behavior
			// is to set condition if addition overflows (i.e. no key down)
			exec("AAKA ");
			if (_keyStrobe == KS_KN) {
				// Advance to next instruction
			} else if (_dActive != 10) {
				// Hold at current instruction and continue scan
				nextAddress = _address;
			} else {
				// For state d10, fall through
			}
			break;
		case 17-16: // WAITDK: wait for display key
			//this.model.display = 0;
			//if (this.model.keyPressed == 'DK') 
			if (_keyPressed) {
				// Jump
				nextAddress = _base_addr + (_instruction & 0x1ff);
			} else {
				// Hold address until DK pressed
				nextAddress = _address;
			}
			break;
		case 18-16: // WAITNO: wait for key or address register overflow
			if (_keyStrobe) {
				// Jump
				nextAddress = _base_addr + (_instruction & 0x1ff);
			} else {
				// Hold address until key pressed or address overflow (TODO)
				nextAddress = _address;
			}
			break;
		case 21-16: // SYNC (SYNCH): hold address until end of D10
			if (_dActive != 10) {
				nextAddress = _address;
			}
			_cc = 0;
			break;
		case 22-16: // SCAN (SCANNO): wait for key
			//this.model.display = 1; // Reset display power off latch
			if (_keyStrobe) {
				_cc = 1;
			} else {
				_cc = 0;
				if (_dActive != 10) 
					nextAddress = _address;
			}//else
			break;
		default:
			if (exec(_opCode)) {
				//nextAddress = _base_addr + (_instruction & 0x1ff);
				nextAddress = _base_addr + _branch_addr;
				//uart_putc('(');
				//uart_puthex16(nextAddress - _base_addr);
				//uart_putc(')');
			}//if
	}//switch


	_address = nextAddress;
	// Update D state (rotate scan column)
	_dActive++;
	if (_dActive > 10) _dActive = 1;
}

