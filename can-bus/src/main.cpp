#include "Arduino.h"

const int myCANid = 10; // CAN ID of this unit
const int msCANid = 0; // CAN ID of the Megasquirt (should almost always be 0)

#include <mcp_can.h>

#define SPI_CS_PIN 9

MCP_CAN CANbus(SPI_CS_PIN);

long time_lap, start_time = millis();

typedef struct {
	// 1512
	union {
		uint8_t bytes1512[8];
		struct {
			int16_t MAP; // * 0.1
			uint16_t RPM;
			int16_t CLT; // * 0.1
			int16_t TPS; // * 0.1
		};
	};
	// 1513
	union {
		uint8_t bytes1513[8];
		struct {
			uint16_t PW1; // * 0.001
			uint16_t PW2; // * 0.001
			int16_t MAT; // * 0.1
			int16_t ADV_DEG; // * 0.1
		};
	};

	// 1514
	union {
		uint8_t bytes1514[8];
		struct {
			uint8_t AFRTGT1; // * 0.1
			uint8_t AFR1; // * 0.1
			int16_t EGOCOR1; // * 0.1
			int16_t EGT1; // * 0.1
			int16_t PWSEQ1; // * 0.1
		};
	};

	// 1515
	union {
		uint8_t bytes1515[8];
		struct {
			int16_t BATT; // * 0.1
			int16_t SENSORS1; // * 0.1
			int16_t SENSORS2; // * 0.1
			int16_t KNK_RTD; // * 0.1
		};
	};

	// 1516
	union {
		uint8_t bytes1516[8];
		struct {
			uint16_t VSS; // * 0.1
			int16_t TC_RTD; // * 0.1
			int16_t LAUNCH_TIMING; // * 0.1
			uint16_t unused;
		};
	};
} MSVars;

MSVars ms_variables;
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
	if (time_lap > 200) {
		start_time = millis();

		Serial.printf(F("%d "), ms_variables.RPM);
		dtostrf((float) ms_variables.ADV_DEG / 10.0, 6, 2, outstr);
		Serial.printf(F("%s "), outstr);
		dtostrf((float) ms_variables.MAP / 10.0, 6, 2, outstr);
		Serial.printf(F("%s \r\n"), outstr);
	}

	unsigned char len = 0;
	unsigned char buf[8];
	if (CAN_MSGAVAIL == CANbus.checkReceive()) {
		CANbus.readMsgBuf(&len, buf);
		switch (CANbus.getCanId()) { // ID's 1520+ are Megasquirt CAN broadcast frames

		// typical compiler optimizations will convert memcpy to multiple LDM/STM instructions
		case 1512:
			  //ms_variables.RPM = (uint16_t) (word(buf[2], buf[3]));
			  memcpy(ms_variables.bytes1512, buf, 8);
			break;
		case 1513:
			  memcpy(ms_variables.bytes1513, buf, 8);
			break;
		case 1514:
			  memcpy(ms_variables.bytes1514, buf, 8);
			break;
		case 1515:
			  memcpy(ms_variables.bytes1515, buf, 8);
			break;
		case 1516:
			  memcpy(ms_variables.bytes1516, buf, 8);
			break;
		default: // not a broadcast packet

			Serial.write("ID: ");
			Serial.println(CANbus.getCanId());

		}
	}
}
