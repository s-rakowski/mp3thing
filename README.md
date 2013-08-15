stellaris-mp3thing
==================

A project for Stellaris LM4F120 Launchpad.

Hardware:
- A Stellaris LM4F120 Launchpad - Tiva C Launchpad should also work, but I don't have one.
- 3.2" LCD TFT screen with SSD1289 controller and a touch panel with XPT2046
- VS1003b board for sound
- SD/SDHC card for storage
- A board that connects it all. In my case, it is done on perfboard with lots of cables.


Software is made in Energia IDE. I modified it to compile with -O2 and not -Os.
FatFs library is used for FAT support.

My code is a big mess. You have been warned.