#include "Arduino.h"
#include "TinyGPS++.h" // GPS library
#include "SerialCommand.h"
#include "Bluetooth_HC05.h"
#include <SoftwareSerial.h>
#define SERIALCOMMANDBUFFER 16
#define SERIALCOMMANDDEBUG

void listBluetoothDevicesFound();
void bluetoothDeviceFound(const BluetoothAddress &address);
void blu_command();
void gps_command();
void showHelp();
void unrecognized();

void displayInfo();

SoftwareSerial bt_serial(3, 2);
TinyGPSPlus gps;
SerialCommand sercmd;
Bluetooth_HC05 blumod(bt_serial);
#define MAX_DISCOVERED_DEVICES	3
BluetoothAddress *discoveredDevices_p = (BluetoothAddress *)malloc(MAX_DISCOVERED_DEVICES * sizeof(BluetoothAddress));
uint8_t discoveredDevicesPos = 0;
bool bt_gps_active = false;
int cnt= 0;

#define STR_FLASH(name, src) \
  static const char name##_pgm[] PROGMEM = src; \
  char name[sizeof(name##_pgm)]; \
  strcpy_P(name, name##_pgm);

void setup() {
	Serial.begin(115200);

//	blumod.beginDetect(7);
	blumod.begin(38400,6, HC05_RESET_GND, 7, HC05_MODE_COMMAND);

	sercmd.addCommand("?", showHelp);
	sercmd.addCommand("gps", gps_command);
	sercmd.addCommand("blu", blu_command);
	sercmd.addDefaultHandler(unrecognized);
	Serial.println(F("** blu-qstarz **"));

//	cli(); //stop interrupts
//	OCR0A = 0xAF;
//	TIMSK0 |= _BV(OCIE0A);
//	sei(); //allow interrupts
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
		if (strcmp(arg, "read") == 0) {
			unsigned long start_time = millis();
			unsigned long time_lap = 0;
			Serial.print(F("start_time:"));
			Serial.print(start_time);
			Serial.println(F(" --- "));

			if (bt_gps_active) {
				while (time_lap < 5000) {
					if (bt_serial.available() > 0) {
						char c = bt_serial.read();
						gps.encode(c);
						Serial.print(c);
					}
					time_lap = millis() - start_time;
				}
				Serial.println(F("--- done!"));
			}
		}
	} else {
		Serial.print(gps.charsProcessed());
		Serial.print(F(" chars processed - "));
		Serial.print(gps.sentencesWithFix());
		Serial.println(F(" sentences with a fix."));
	}
}

void listBluetoothDevicesFound() {
	if (discoveredDevicesPos == 0)  {
		Serial.println(F("No devices!"));
		return;
	}

	for (int c = 0; c < discoveredDevicesPos; c++) {
		char address_str[HC05_ADDRESS_BUFSIZE];
		blumod.getRemoteDeviceName(discoveredDevices_p[c],address_str,HC05_ADDRESS_BUFSIZE,30000);
		Serial.printf(F("Device %d="),c);
		Serial.print(address_str);
		Serial.print(F(" ["));
		blumod.printBluetoothAddress(address_str,discoveredDevices_p[c],',');
		Serial.print(address_str);
		Serial.println(F("]"));
	}
}

void bluetoothDeviceFound(const BluetoothAddress &address) {
	Serial.print('.');

	if (discoveredDevicesPos > MAX_DISCOVERED_DEVICES) return;

	for (int c = 0; c < discoveredDevicesPos; c++) {
		if (memcmp(discoveredDevices_p[c].bytes, address.bytes,sizeof(address.bytes)) == 0)
			return;
	}
//	Serial.printf(F("discoveredDevicesPos=%d\r\n"),discoveredDevicesPos);
	memcpy(discoveredDevices_p[discoveredDevicesPos].bytes,address.bytes,sizeof(address.bytes));

	discoveredDevicesPos++;
}

void blu_command() {
	char *arg;
	arg = sercmd.next();

	if (arg != NULL) {

		//1c 88 14 38 6e

		if (strcmp(arg, "connect") == 0) {

			blumod.setRole(HC05_ROLE_MASTER);
			blumod.setPassword("0000");
			blumod.initSerialPortProfile();
			blumod.setInquiryMode(HC05_INQUIRY_RSSI, 9, 3);
			blumod.inquire(NULL,60000);
			BluetoothAddress qstarz818 = { 0x1c, 0x00, 0x88, 0x6e, 0x38, 0x14 };
			bt_gps_active = blumod.connect(qstarz818,10000);
//			blumod.changeMode(HC05_MODE_DATA);

//			pinMode(5, OUTPUT);
//			digitalWrite(5, LOW);
			/*
			 	blumod.begin(38400, 7, HC05_MODE_COMMAND);
				delay(1000);
				blumod.restoreDefaults();
				blumod.setRole(HC05_ROLE_MASTER);
				blumod.setPassword("0000");
				blumod.initSerialPortProfile();
				delay(1000);
				blumod.inquire(NULL);
				BluetoothAddress qstarz818 = { 0x1c, 0x00, 0x88, 0x6e, 0x38, 0x14 };
				bt_gps_active = blumod.connect(qstarz818,10000);
				blumod.changeMode(HC05_MODE_DATA);
			 */
		}

		if (strcmp(arg, "name") == 0) {
			char buffer[20];
			blumod.getName(buffer);
			Serial.print(F("name="));
			Serial.println(buffer);
		}

		if (strcmp(arg, "reset") == 0) {
			blumod.changeMode(HC05_MODE_COMMAND);
			delay(1000);
			blumod.restoreDefaults();
			blumod.setRole(HC05_ROLE_MASTER);
			blumod.setPassword("0000");
			Serial.println(F("bluetooth reset"));
		}

		if (strcmp(arg, "find") == 0) {
			int num = 5;
			arg = sercmd.next();
			if (arg != NULL) {
				num = atol(arg);
			}

			blumod.setRole(HC05_ROLE_MASTER);
			blumod.setPassword("0000");
			blumod.initSerialPortProfile();
			blumod.setInquiryMode(HC05_INQUIRY_RSSI, 9, num);
			Serial.print(F("Searching"));
			blumod.inquire(bluetoothDeviceFound,60000);
			Serial.println();
			listBluetoothDevicesFound();
		}


		if (strcmp(arg, "cmd") == 0) {
//			blumod.setCommandMode()

		}
	} else {
		HC05_State state;
		blumod.probe();
		blumod.getState(state);
		Serial.print(F("state="));
		Serial.println(state);
	}
}

void showHelp() {
	Serial.println(F("** HELP ** "));
	Serial.println(F("gps - display char processed and sentences with gps fix"));
	Serial.println(F("gps loc - display gps location"));
	Serial.println(F("gps loc N - display gps location up to N times, 1 second apart"));
}

void unrecognized() {
	Serial.println(F("? unrecognized cmd - type '?' for help"));
}

void loop() {

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
