#include "../blu-qstarz/blu-qstarz.h"

SoftwareSerial bluetooth(6, 5);
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

void readResponse() {
	char buffer[25];

	char *p = buffer;
	*p = 0;

	unsigned long start_ticks = millis(), now_ticks;

	while (((size_t) (p - buffer) < sizeof(buffer) - 1) && (p[-1] != '\n' || p[-2] != '\r')) {
		now_ticks = millis();
		if (now_ticks - start_ticks > 1000) {
			break;
		}
		if (bluetooth.available()) {
			*p++ = bluetooth.read();
			start_ticks = millis();
		}
	}

	if (p[-1] == '\n' && p[-2] == '\r')
		p -= 2;

	*p = 0;

	Serial.println(buffer);
}


void readUntil(unsigned long delay) {
	unsigned long start_ticks = millis();
	unsigned long now_ticks = millis();
	while (now_ticks - start_ticks < delay) {
		if (bluetooth.available()) {
			Serial.print(char(bluetooth.read()));
		}
		now_ticks = millis();
	}
}

void doCmd(const __FlashStringHelper *cmd) {

	Serial.println(cmd);
	bluetooth.println(cmd);
	readResponse();
}

void bluqstarz_setup() {
	Serial.begin(115200);
	bluetooth.begin(38400);

	Serial.println(F("** blu-qstarz **"));

	readUntil(100);
	pinMode(7, OUTPUT);
	digitalWrite(7, HIGH);
	Serial.println(F("pin 7 high"));

	doCmd(F("AT"));

	doCmd(F("AT+UART=9600,0,0"));

	doCmd(F("AT+ORGL"));

	doCmd(F("AT+RESET"));

	readUntil(1000);

//	doCmd(F("AT+RMAAD"));
	doCmd(F("AT+ADCN?"));

	doCmd(F("AT+ROLE=1"));

	doCmd(F("AT+PSWD=0000"));

	doCmd(F("AT+INIT"));

	doCmd(F("AT+INQM=1,9,4")); // limit to 4*1.28s = ~5 seconds

	doCmd(F("AT+INQ"));
	readUntil(6000);

	doCmd(F("AT+STATE"));

	doCmd(F("AT+RNAME?1C,88,14386E"));
	readUntil(5000);
	doCmd(F("AT+STATE"));

	doCmd(F("AT+LINK=1C,88,14386E"));
	readUntil(1000);
	Serial.println(F("wait for 5 sec"));

	delay(5000);
	digitalWrite(7, LOW);
	Serial.println(F("pin 7 low"));

//	doCmd(F("AT+RESET"));

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

unsigned long loopCnt = 0;

void bluqstarz_loop() {
	if (bluetooth.available()) {
		char c = char(bluetooth.read());
		gps.encode(c);
//		Serial.print(c);
	}
	loopCnt++;

	if (loopCnt > 10000) {
		displayInfo();
		loopCnt=0;
	}
}
