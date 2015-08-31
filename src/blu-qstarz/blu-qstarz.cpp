#include "../blu-qstarz/blu-qstarz.h"

SoftwareSerial bt_serial(6, 5);
TinyGPSPlus gps;

SerialCommand sercmd;
Bluetooth_HC05 blumod(bt_serial);

#define STR_FLASH(name, src) \
  static const char name##_pgm[] PROGMEM = src; \
  char name[sizeof(name##_pgm)]; \
  strcpy_P(name, name##_pgm);

void bluqstarz_setup() {
	Serial.begin(115200);
//
	blumod.begin(38400, 7, HC05_MODE_COMMAND);

	sercmd.addCommand("?", showHelp);
	sercmd.addCommand("gps", gps_command);
	sercmd.addCommand("blu", blu_command);
	sercmd.addDefaultHandler(unrecognized);
	Serial.println(F("** blu-qstarz **"));
/*
	bt_serial.begin(38400);

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
	 // */
}

void readResponse() {
	char buffer[25];

	char *p = buffer;
	*p = 0;

	unsigned long start_ticks = millis(), now_ticks;

	while (((size_t) (p - buffer) < sizeof(buffer) - 1)
			&& (p[-1] != '\n' || p[-2] != '\r')) {
		now_ticks = millis();
		if (now_ticks - start_ticks > 1000) {
			break;
		}
		if (bt_serial.available()) {
			*p++ = bt_serial.read();
			start_ticks = millis();
		}
	}

	if (p[-1] == '\n' && p[-2] == '\r')
		p -= 2;

	*p = 0;

	Serial.println(buffer);
}

void readUntil(unsigned long delay) {
	bt_serial.flush();
	unsigned long start_ticks = millis();
	unsigned long now_ticks = millis();
	while (now_ticks - start_ticks < delay) {
		if (bt_serial.available()) {
			Serial.print(char(bt_serial.read()));
		}
		now_ticks = millis();
	}
}

void doCmd(const __FlashStringHelper *cmd) {
	Serial.println(cmd);
	bt_serial.println(cmd);
	readResponse();
}

void gps_command() {
	char *arg;
	arg = sercmd.next();

	if (arg != NULL) {
		if (strcmp(arg, "loc") == 0) {

			arg = sercmd.next();
			if (arg != NULL) {
				int num = atol(arg);
				for (int c = 0; c < num; c++) {
					Serial.printf("%d ", c + 1);
					displayInfo();
					delay(1000);
				}
			} else {
				displayInfo();
			}

		}
	} else {
		Serial.print(gps.charsProcessed());
		Serial.print(" chars processed - ");
		Serial.print(gps.sentencesWithFix());
		Serial.println(" sentences with a fix.");
	}
}
void bluetoothDeviceFound(const BluetoothAddress &address) {
	char address_str[HC05_ADDRESS_BUFSIZE];
	blumod.printBluetoothAddress(address_str, address, ',');
	Serial.print("bluetoothDeviceFound=");
	Serial.println(address_str);
}

void blu_command() {
	char *arg;
	arg = sercmd.next();

	if (arg != NULL) {


		if (strcmp(arg, "test") == 0) {

			doCmd(F("AT"));

			doCmd(F("AT+UART=9600,0,0"));

			doCmd(F("AT+ORGL"));

			doCmd(F("AT+RESET"));

			delay(500);

			doCmd(F("AT+RMAAD"));
			doCmd(F("AT+ADCN?"));

			doCmd(F("AT+ROLE=1"));

			doCmd(F("AT+PSWD=0000"));

			doCmd(F("AT+INIT"));

			doCmd(F("AT+INQM=1,9,4")); // limit to 4*1.28s = ~5 seconds

			doCmd(F("AT+INQ"));
		}

		if (strcmp(arg, "name") == 0) {
			char buffer[20];
			blumod.getName(buffer);
			Serial.print("name=");
			Serial.println(buffer);
		}

		if (strcmp(arg, "reset") == 0) {
			blumod.restoreDefaults();
			delay(500);
			blumod.setSerialMode(9600, 1, HC05_NO_PARITY);
			blumod.setRole(HC05_ROLE_MASTER);
			blumod.setPassword("0000");
			blumod.softReset();
			delay(500);
			blumod.probe();
			Serial.println("bluetooth reset");
		}

		if (strcmp(arg, "find") == 0) {
			int num = 4;
			arg = sercmd.next();
			if (arg != NULL) {
				num = atol(arg);
			}

			blumod.setRole(HC05_ROLE_MASTER);
			blumod.setPassword("0000");
			blumod.initSerialPortProfile();
			blumod.setInquiryMode(HC05_INQUIRY_RSSI, 9, num);
			blumod.inquire(bluetoothDeviceFound);
//			if (blumod.getLastError() != HC05_OK) Serial.print("6fail");

		}
	} else {
		HC05_State state;
		blumod.probe();
		blumod.getState(state);
		Serial.print("state=");
		Serial.println(state);
	}
}

void showHelp() {
	Serial.println("** HELP ** ");
	Serial.println("gps - display char processed and sentences with gps fix");
	Serial.println("gps loc - display gps location");
	Serial.println(
			"gps loc N - display gps location up to N times, 1 second apart");
}

void unrecognized() {
	Serial.println("? unrecognized cmd - type '?' for help");
}

void bluqstarz_loop() {

	sercmd.readSerial();
}

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
/*
 void readResponse() {
 char buffer[25];

 char *p = buffer;
 *p = 0;

 unsigned long start_ticks = millis(), now_ticks;

 while (((size_t) (p - buffer) < sizeof(buffer) - 1)
 && (p[-1] != '\n' || p[-2] != '\r')) {
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
 bluetooth.flush();
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

 void doCmdFmt(const __FlashStringHelper *cmd,...){
 va_list ap;
 Serial.printf(cmd,ap);
 Serial.println();
 bluetooth.printf(cmd,ap);
 bluetooth.println();
 readResponse();
 }


 void enableATCmds() {
 digitalWrite(7, HIGH);
 readUntil(100);
 }

 void disableATCmds() {
 digitalWrite(7, LOW);
 readUntil(100);
 }

 void doCmd(const char *cmd) {
 Serial.println(cmd);
 bluetooth.println(cmd);
 readResponse();
 }

 void doCmd(const __FlashStringHelper *cmd) {
 Serial.println(cmd);
 bluetooth.println(cmd);
 readResponse();
 }

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

 */

/*
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
 */
