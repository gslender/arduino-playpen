#include "Arduino.h"

// BMP-loading example specifically for the TFTLCD Arduino shield.
//#include "bmp-tft-sd/bmp-tft-sd.h"

// Testing the LinkSprite bluetooth with QStarz GPS
#include "blu-qstarz/blu-qstarz.h"


void setup()
{
//	bmptftsd_setup();
	bluqstarz_setup();
}
void loop()
{
//	bmptftsd_loop();
	bluqstarz_loop();
}
