#include "Arduino.h"

int dataPin = A2;  // Data pin
int latchPin = A3;  // Latch pin
int clockPin = A4;  // Clock pin

int ledBits = 0;

void writeLedBits() {
	//ground latchPin and hold low for as long as you are transmitting
	digitalWrite(latchPin, LOW);
	shiftOut(dataPin, clockPin, LSBFIRST, ledBits);
	//return the latch pin high to signal chip that it
	//no longer needs to listen for information
	digitalWrite(latchPin, HIGH);
}

void enableLed(int led, bool enable) {
	if (enable)
		ledBits |= (1UL << (led));
	else
		ledBits &= ~(1UL << (led));
	writeLedBits();
}

void setup() {
	//set pins to output because they are addressed in the main loop
	pinMode(latchPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	pinMode(dataPin, OUTPUT);
	writeLedBits();
}

#define pause_ms 70

void loop() {

	ledBits = 0;
	for (int j = 0; j < 8; j++) {
		enableLed(j, true);
		delay(pause_ms);
	}
	delay(pause_ms);
	for (int j = 0; j < 4; j++) {
		ledBits ^= 0xffF;
		writeLedBits();
		delay(pause_ms);
		ledBits ^= 0xffF;
		writeLedBits();
		delay(pause_ms);
	}

	for (int j = 8; j > 0; j--) {
		enableLed(j, false);
		delay(pause_ms);
	}
}
