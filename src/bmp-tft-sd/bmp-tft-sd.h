
#ifndef _BMPTFTSD_H_
#define _BMPTFTSD_H_


// BMP-loading example specifically for the TFTLCD Arduino shield.
// If using the breakout board, use the tftbmp.pde sketch instead!
// If using an Arduino Mega and your sheild does not use the ICSP header for
// SPI, make sure the SD library is configured for 'soft' SPI in the file Sd2Card.h.
// If in doubt, update the library to use 'soft' SPI.

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <SD.h>

// In the SD card, place 24 bit color BMP files (be sure they are 24-bit!)
// There are examples in the sketch folder

#define SD_CS 5 // Card select for shield use


// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

#define BUFFPIXEL 20

void bmptftsd_setup();
void bmptftsd_loop();

#endif
