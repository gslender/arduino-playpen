#include "Arduino.h"
/*
 * Simple sketch that cycles the display through available hues,
 * keeping a solid 100% saturation value,
 * simulating slowly circling the HSV colour wheel.
 */
#include <SPI.h>
#include <SD.h>
#include <FTOLED.h>

#include <fonts/Droid_Sans_36.h>
#include <fonts/Droid_Sans_24.h>
#include <fonts/Droid_Sans_16.h>
#include <fonts/Arial_Black_16.h>
#include <fonts/Arial14.h>
#include <fonts/Droid_Sans_12.h>
#include <fonts/SystemFont5x7.h>

const byte pin_cs = 8;
const byte pin_dc = 9;
const byte pin_reset = 10;

const OLED_Colour VERYDARKSLATEGRAY        = {  30/8,  30/4,  30/8 };

OLED oled(pin_cs, pin_dc, pin_reset);
//OLED_TextBox box(oled);

void drawRpm(int rpm) {

	oled.selectFont(Droid_Sans_36);
	char rpmStr[6];
	sprintf(rpmStr, "%d", rpm);
	int w = oled.stringWidth(rpmStr);
	byte x1 = 63-w/2;
	byte y1=42;
	oled.drawFilledBox(10,y1,118,y1+36,BLACK);
	oled.drawString(x1, y1, rpmStr, ORANGERED, BLACK);

}
int rpm = 5500;
void setup() {
	oled.begin();

	oled.drawCircle(63, 63, 63, WHITE);


//	oled.setPixel(0,0,YELLOW);
//	oled.setPixel(127,127,BLUE);
	drawRpm(rpm);
}

void loop() {

	delay(300);
	rpm -= 15;
	drawRpm(rpm);

}
