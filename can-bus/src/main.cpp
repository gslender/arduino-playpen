#include "Arduino.h"
#include "megasquirt/MegaSquirt.h"
//
//const int myCANid = 10; // CAN ID of this unit
//const int msCANid = 0; // CAN ID of the Megasquirt (should almost always be 0)

#include <SPI.h>
#include <mcp_can.h>

#define SPI_CS_PIN 9
#define logfile Serial

MCP_CAN CANbus(SPI_CS_PIN);

unsigned long can_msg_cnt;
long time_lap, start_time = millis();
MegaSquirt ms;
//MSVars ms_variables;
static char outstr[15];

// -------------------------------------------------------------
void setup(void) {

    Serial.begin(115200);

START_INIT:
    if(CAN_OK == CANbus.begin(CAN_500KBPS))                   // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init ok!");
    }
    else
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
        goto START_INIT;
    }
}

// -------------------------------------------------------------
void loop(void) {

	time_lap = millis() - start_time;
	if (time_lap > 100) {
		start_time = millis();


		logfile.print(millis());
		logfile.print('\t');
		logfile.print(ms.getData().RPM);
		logfile.print('\t');
		dtostrf((float) ms.getData().MAP / 10.0f, 6, 2, outstr);
		logfile.print(outstr);
		logfile.print('\t');
		dtostrf((float) ms.getData().TPS / 10.0f, 6, 2, outstr);
		logfile.print(outstr);
		logfile.print('\t');
		dtostrf((float) ms.getData().AFR1 / 10.0f, 6, 2, outstr);
		logfile.print(outstr);
		logfile.print('\t');
		dtostrf((float) (ms.getData().MAT-320) * 0.05555f, 6, 2, outstr);
		logfile.print(outstr);
		logfile.print('\t');
		dtostrf((float) (ms.getData().CLT-320) * 0.05555f, 6, 2, outstr);
		logfile.print(outstr);
		logfile.print('\t');
		logfile.print("0");
		logfile.print('\t');
		dtostrf((float) ms.getData().BATT / 10.0f, 6, 2, outstr);
		logfile.print(outstr);
		logfile.print('\t');
		dtostrf((float) ms.getData().EGOCOR1 / 10.0f, 6, 2, outstr);
		logfile.print(outstr);
		logfile.print('\t');
		dtostrf((float) ms.getData().PW1 / 1000.0f, 6, 3, outstr);
		logfile.print(outstr);
		logfile.print('\t');
		dtostrf((float) ms.getData().ADV_DEG / 10.0f, 6, 2, outstr);
		logfile.print(outstr);

		logfile.println();

	}

	unsigned char len = 0;
	unsigned char buf[8];
	if (CAN_MSGAVAIL == CANbus.checkReceive()) {
		can_msg_cnt++;
		CANbus.readMsgBuf(&len, buf);
		ms.process(CANbus.getCanId(),buf);
	}
}
