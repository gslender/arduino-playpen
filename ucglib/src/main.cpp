/*
 test
 */

#include <Arduino.h>
#include <SPI.h>
#include <Ucglib.h>

Ucglib_SSD1351_18x128x128_FT_HWSPI ucg(9, 8, 10);

const byte pin_cs = 8;
const byte pin_dc = 9;
const byte pin_reset = 10;

struct Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

char displayed_rpm_digits[5];

int rpm = 1900;
int rpmMax = 8000;
int rpmMin = 800;
int rpmStep = 1;

int clt = 40;
int cltMax = 120;
int cltMin = 20;
int cltStep = 1;

inline void printCharAt(byte x, byte y, Color c, char ch) {
	ucg.setColor(c.r, c.g, c.b);
	ucg.setPrintPos(x, y);
	ucg.print(ch);
}

void drawRpm(int rpm) {
	const byte x = 32;
	const byte y = 35;
	const byte w = 16;

	const Color fgCol = { 200, 40, 0 };
	const Color bgCol = { 0, 0, 0 };

//	ucg.setFont(ucg_font_ncenR24_tr);
//	ucg.setFont(ucg_font_fub30_tr);
	ucg.setFont(ucg_font_helvB18_hr);
	ucg.setColor(1, bgCol.r, bgCol.g, bgCol.b);

	char newRpmStr[5];
	snprintf(newRpmStr, 5, "%4d", rpm);

	for (int n = 0; n < 5; n++) {
		if (newRpmStr[n] != displayed_rpm_digits[n]) {
			byte wx = x + (w * n);
			printCharAt(wx, y, bgCol, displayed_rpm_digits[n]);
			displayed_rpm_digits[n] = newRpmStr[n];
			printCharAt(wx, y, fgCol, displayed_rpm_digits[n]);
		}
	}
}

void drawClt(int clt, uint8_t width) {
	ucg.setFont(ucg_font_helvB10_hf);
	ucg.setFontMode(UCG_FONT_MODE_SOLID);

	const byte x = 10;
	const byte y = 45;
	const byte w = width;
	const byte h = ucg.getFontAscent()+abs(ucg.getFontDescent());

	const Color bg_cold = {0,130,0};
	const Color bg_warm = {0,0,160};
	const Color bg_hot = {160,0,0};
	const Color fg = {0,0,0};

//	ucg.setColor(0, bg_cold.r/2, bg_cold.g/2, bg_cold.b/2);
	ucg.setColor(0, bg_cold.r/2, bg_cold.g/2, 255);
	ucg.drawFrame(x,y,w,h);
	ucg.setPrintPos(x+1, 1+y+h);
	ucg.setColor(0, 0, 0, 0);
	ucg.setColor(1, bg_cold.r, bg_cold.g, bg_cold.b);
	ucg.printf("CgT:%d%c",clt,176); // ASCII 248 = degree symbol
}

void setup(void) {

	ucg.begin(UCG_FONT_MODE_TRANSPARENT);

	ucg.clearScreen();
	ucg.setColor(25, 25, 25);
	ucg.drawCircle(64, 64, 63, UCG_DRAW_ALL);

// zero,zero is top left
//	ucg.setColor(0, 125, 125);
//	ucg.drawPixel(126, 126);
//	ucg.setColor(0, 0, 125);
//	ucg.drawPixel(2, 2);

	drawRpm(0);
	drawClt(0,100);

	rpmMax = random(1000, 8000);
}

void loop() {
	drawRpm(rpm);
//	drawClt(clt);
	delay(40);

	rpm+=rpmStep;
	if (rpmStep > 0) {
		rpmStep = random(1, 10);
		if (rpm > rpmMax) {
			rpmStep = -random(1, 10);
		}
	} else {
		rpmStep = -random(1, 10);
		if (rpm < rpmMin) {
			rpmStep = random(1, 10);
			rpmMax = random(1000, 8000);
		}
	}


	clt+=cltStep;
	if (cltStep > 0) {
		cltStep = random(1, 50);
		if (clt > cltMax) {
			cltStep = -random(1, 50);
		}
	} else {
		cltStep = -random(1, 50);
		if (clt < cltMin) {
			cltStep = random(1, 50);
		}
	}
}
