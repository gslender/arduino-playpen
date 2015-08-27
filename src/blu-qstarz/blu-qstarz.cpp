#include "../blu-qstarz/blu-qstarz.h"

SoftwareSerial bluetooth(4, 3);
TinyGPSPlus gps;

void displayInfo() {
	Serial.print(F("Location: "));
	if (gps.location.isValid()) {
		Serial.print(gps.location.lat(), 6);
		Serial.print(F(","));
		Serial.print(gps.location.lng(), 6);
	} else {
		Serial.print(F("INVALID"));
	}

	Serial.print(F("  Date/Time: "));
	if (gps.date.isValid()) {
		Serial.print(gps.date.month());
		Serial.print(F("/"));
		Serial.print(gps.date.day());
		Serial.print(F("/"));
		Serial.print(gps.date.year());
	} else {
		Serial.print(F("INVALID"));
	}

	Serial.print(F(" "));
	if (gps.time.isValid()) {
		if (gps.time.hour() < 10)
			Serial.print(F("0"));
		Serial.print(gps.time.hour());
		Serial.print(F(":"));
		if (gps.time.minute() < 10)
			Serial.print(F("0"));
		Serial.print(gps.time.minute());
		Serial.print(F(":"));
		if (gps.time.second() < 10)
			Serial.print(F("0"));
		Serial.print(gps.time.second());
		Serial.print(F("."));
		if (gps.time.centisecond() < 10)
			Serial.print(F("0"));
		Serial.print(gps.time.centisecond());
	} else {
		Serial.print(F("INVALID"));
	}

	Serial.println();
}

void gpsLoop() {
	// This sketch displays information every time a new sentence is correctly encoded.
	while (bluetooth.available() > 0)
		if (gps.encode(bluetooth.read()))
			displayInfo();

	if (millis() > 10000 && gps.charsProcessed() < 10) {
		Serial.println(F("No GPS device detected: check wiring."));
		while (true)
			;
	}
}

void bluqstarz_setup()
{
	Serial.begin(38400);
	Serial.print(F("** blu-qstarz **"));

	bluetooth.begin(38400);
	// turn on RMC (recommended minimum) and GGA (fix data) including altitude
	//gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

	// Set the update rate
	//gps.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);   // 5 Hz update rate
	//gps.sendCommand(PMTK_API_SET_FIX_CTL_5HZ);   // 5 Hz update rate

}

/*

 AT+ROLE=1
 AT+INIT
 AT+INQ

 */

void bluqstarz_loop()
{
    if(bluetooth.available())
    {
      Serial.print(char(bluetooth.read()));
    }
    if(Serial.available())
    {
    	bluetooth.print(char(Serial.read()));
    }
}
