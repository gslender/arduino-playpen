#include "Arduino.h"
#include <SPI.h>
#include <SdFat.h>

#define SD_CS_PIN    	4

File logfile;
SdFat SD;


void setup() {
    Serial.begin(115200);

	Serial.println("test");

	Serial.println(SD.begin(SD_CS_PIN, SPI_FULL_SPEED));

	logfile = SD.open("test.txt", FILE_WRITE);
	if (logfile) {
		Serial.println("write header");
		logfile.println(F("Time\tRPM\tMAP\tTPS\tAFR\tMAT\tCLT\tBatt V\tEGO cor1\tPW\tSpark Adv\tGPS Lat\tGPS Lon\tGPS Speed"));
	}
}

void loop()
{
}
