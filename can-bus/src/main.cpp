#include "Arduino.h"

const int myCANid = 10; // CAN ID of this unit
const int msCANid = 0; // CAN ID of the Megasquirt (should almost always be 0)

#include <mcp_can.h>

#define SPI_CS_PIN 9

MCP_CAN CANbus(SPI_CS_PIN);

long time_lap, start_time = millis();

//MS data vars
byte ENGINE;
unsigned int RPM, MAP, SPKADV, BATTV, TPS, Knock, Baro, EGOc, IAC, dwell, idle_tar;
int AFR, AFRtgt, CLT, MAT;

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

		Serial.printf(F("%d %d %d \r\n"), RPM, SPKADV, MAP);
	}

	unsigned char len = 0;
	unsigned char buf[8];
	if (CAN_MSGAVAIL == CANbus.checkReceive())           // check if data coming
			{
		CANbus.readMsgBuf(&len, buf); // read data,  len: data length, buf: data buf

		switch (CANbus.getCanId()) { // ID's 1520+ are Megasquirt CAN broadcast frames

		// typical compiler optimizations will convert memcpy to multiple LDM/STM instructions
		case 1512:
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
/*
		case 1520: // 0
			RPM = (int) (word(buf[6], buf[7]));
			break;
		case 1521: // 1
			SPKADV = (int) (word(buf[0], buf[1]));
			ENGINE = buf[3];
			AFRtgt = (int) (word(0x00, buf[4]));
			break;
		case 1522: // 2
			Baro = (int) (word(buf[0], buf[1]));
			MAP = (int) (word(buf[2], buf[3]));
			MAT = (int) (word(buf[4], buf[5]));
			CLT = (int) (word(buf[6], buf[7]));
			break;
		case 1523: // 3
			TPS = (int) (word(buf[0], buf[1]));
			BATTV = (int) (word(buf[2], buf[3]));
			break;
		case 1524: // 4
			Knock = (int) (word(buf[0], buf[1]));
			EGOc = (int) (word(buf[2], buf[3]));
			break;
		case 1526: // 6
			IAC = (int) (word(buf[6], buf[7])); //IAC = (IAC * 49) / 125;
			break;
		case 1529: // 9
			dwell = (int) (word(buf[4], buf[5]));
			break;
		case 1548: // 28
			idle_tar = (int) (word(buf[0], buf[1]));
			break;
		case 1551: // 31
			AFR = (int) (word(0x00, buf[0]));
			break;
			*/
		default: // not a broadcast packet

			Serial.write("ID: ");
			Serial.print(CANbus.getCanId());

		}
	}
}
