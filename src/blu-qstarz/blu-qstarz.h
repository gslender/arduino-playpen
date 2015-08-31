
#ifndef _BLUQSTARZ_H_
#define _BLUQSTARZ_H_

#include "Arduino.h"
#include "TinyGPS++.h" // GPS library
#include "SerialCommand.h"
#include "Bluetooth_HC05.h"
#include <SoftwareSerial.h>
#define SERIALCOMMANDBUFFER 16
#define SERIALCOMMANDDEBUG


void bluqstarz_setup();
void bluqstarz_loop();
void listBluetoothDevicesFound();
void bluetoothDeviceFound(const BluetoothAddress &address);
void blu_command();
void gps_command();
void showHelp();
void unrecognized();

void displayInfo();

/*

void readResponse();
void readUntil(unsigned long delay);
void doCmd(const __FlashStringHelper *cmd);
void doCmdFmt(const __FlashStringHelper *cmd,...);
void enableATCmds();
void disableATCmds();
*/
#endif
