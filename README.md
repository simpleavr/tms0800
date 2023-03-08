tms0800
========
tms080x calculation chip emulation, w/ TI-1500 and Sinclair Scientific ROMs

Nov 2019

- create new pcb as bubble led no longer available
- new pcb adopts 2.5" 3 digit 7 segment LED display (x3 units)
- minimal source change to employ new hardware design
- compile source
- tms0800_pcb19.c for new 2019 version
- tms0800_pcb.c for old 2014 version
- tms0800.c for ti launchpad + paradiso sheild version

May 2015

- now does LPM4 auto-off, can eliminate spdt slide switch on pcb
- auto-off time is 60 seconds of in-activity. 'C' key to swith on
- move setup function from press-n-hold to 'C' + 5, 6, 7, 8, 9 keys
- setup is pressing 'C' and in very quick succession one of 6, 7, 8, 9 keys
- C+5 = button test ('C'+'C' to end test)
- C+6 = secret message edit (+ key to advance)
- C+7 = toggle datamath / sinclair roms
- C+8 = toggle slow / fast cpu
- C+9 = show secret message

June 2014

- additional single pcb design
- compile tms0800_pcb.c in place of tms0800.c
- corresponding led_pcb.h defines new IO used
- employs 2 x 4 digit bubble led
- mutilplexing w/ 8 col x 8 row, no more charlieplexing
- remove uart dis-assembler

July 2014
- change code to adopt to V2 PCB layout

Sep 2013
code provided as is, no warranty

this is a hobby project

you should not use code for commercial purpose w/o permission from creators
nice if you give credit, mention source(s) if you adopt much of the code

based on work of Ken Shirriff, and others, please visit
http://righto.com/ti and http://righto.com/sinclair for details
chris chung (www.simpleavr.com)

September 2013

- h/w (UI booster) based on another project (ezs430trainer)

http://www.simpleavr.com/msp430-projects/ez430trainer

- test compiled and run on win7 w/ cygwin, mint 14 linux.
- NOT tested w/ CCS, IAR, should also work w/ tweaks.
- compile w/ msp430-gcc like so

msp430-gcc -Os -Wall -ffunction-sections -fdata-sections -fno-inline-small-functions -Wl,-Map=tms0800.map,--cref -Wl,--relax -Wl,--gc-sections -I<your mpsgcc>/msp430/include -mmcu=msp430g2553 -o tms0800.elf tms0800.c

September 2013 initial check-in


