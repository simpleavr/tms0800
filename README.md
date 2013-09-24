
tms0800
========
tms080x calculation chip emulation, w/ TI-1500 and Sinclair Scientific ROMs

Sep 2013
code provided as is, no warranty

this is a hobby project

you should use code for commercial purpose w/o permission from creators
nice if you give credit, mention source(s) if you adopt much of the code

based on work of Ken Shirriff, and others, please visit
http://righto.com/ti and http://righto.com/sinclair for details
chris chung (www.simpleavr.com)

September 2013

. h/w (UI booster) based on another project (ezs430trainer)

http://www.simpleavr.com/msp430-projects/ez430trainer

. test compiled and run on win7 w/ cygwin, mint 14 linux.
. NOT tested w/ CCS, IAR, should also work w/ tweaks.
. compile w/ msp430-gcc like so

msp430-gcc -Os -Wall -ffunction-sections -fdata-sections -fno-inline-small-functions -Wl,-Map=tms0800.map,--cref -Wl,--relax -Wl,--gc-sections -I<your mpsgcc>/msp430/include -mmcu=msp430g2553 -o tms0800.elf tms0800.c

September 2013 initial check-in


