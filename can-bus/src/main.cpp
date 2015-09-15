#include "Arduino.h"

const int myCANid = 10; // CAN ID of this unit
const int msCANid = 0; // CAN ID of the Megasquirt (should almost always be 0)

#include <mcp_can.h>

#define SPI_CS_PIN 9

MCP_CAN CANbus(SPI_CS_PIN);

int rpm,ect,afr;
unsigned int txTimer,rxTimer;

//MS data vars
byte indicator[7]; // where to store indicator data
unsigned int RPM, CLT, MAP, MAT, SPKADV, BATTV, TPS, Knock, Baro, EGOc, IAC, dwell, bstduty, idle_tar;
int AFR, AFRtgt;
unsigned int MAP_HI, Knock_HI, RPM_HI, CLT_HI, MAT_HI;
int AFR_HI, AFR_LO;

struct MSDataObject {
  char name[10]; // length tbd
  byte block; // max val 32?
  unsigned int offset; // max val?
  byte reqbytes; // max val 8
  byte mult; // does this need to be * 0.1 ?
};

const MSDataObject MSData[] PROGMEM = {
// string,  block, offset, reqbytes, mult, div
  {"RPM",     7,   6,  2,    0   }, // 0
  {"AFR",     7,  252, 1,    0   }, // 1
  {"CLT",     7,  22,  2,    1   }, // 2
  {"MAP",     7,  18,  2,    1   }, // 3
  {"MAT",     7,  20,  2,    1   }, // 4
  {"SPKADV",  7,   8,  2,    1   }, // 5
  {"BATTV",   7,  26,  2,    1   }, // 6
  {"TPS",     7,  24,  2,    1   }, // 7
  {"Knock",   7,  32,  2,    1   }, // 8
  {"Baro",    7,  16,  2,    1   }, // 9
  {"EGOc",    7,  34,  2,    1   }, // 10
  {"IAC",     7,  54,  2,    0   }, // 11 -- this was GFC's to 49 / 125
  {"dwell",   7,  62,  2,    1   }, // 12
  {"bstduty", 7,  39,  1,    0   }, // 13 boost duty cycle
  {"idletar", 7, 380,  2,    0   }, // 14
  {"AFRtgt",  7,  12,  1,    1   }, // 15
};

struct MSDataBinaryObject {
  char name[14];
  byte sbyte;
  byte bitp;
};

const MSDataBinaryObject MSDataBin[] PROGMEM = {
// "1234567890"
//"name", indicator byte, bit position
//0 engine - block 7 offset 11
  {"Ready",            0,  0 },
  {"Crank",            0,  1 },
  {"ASE",              0,  2 },
  {"WUE",              0,  3 },
  {"TPS acc",          0,  4 },
  {"TPS dec",          0,  5 },
  {"MAP acc",          0,  6 },
  {"MAP dec",          0,  7 },
//1 status1 - block7 offset 78
  {"Need Burn",        1,  0,},
  {"Data Lost",        1,  1 },
  {"Config Error",     1,  2 },
  {"Synced",           1,  3 },
  {"VE1/2",            1,  5 },
  {"SPK1/2",           1,  6 },
  {"Full-sync",        1,  7 },
//2 status2 - block 7 offset 79
  {"N2O 1",            2,  0 },
  {"N2O 2",            2,  1 },
  {"Rev lim",          2,  2 },
  {"Launch",           2,  2 },
  {"Flat shift",       2,  4 },
  {"Spark cut",        2,  5 },
  {"Over boost",       2,  6 },
  {"CL Idle",          2,  7 },
//3 status3 - block 7 offset 80
  {"Fuel cut",         3,  0 },
//{"T-log",            3,  1 },
//{"3 step",           3,  2 },
//{"Test mode",        3,  3 },
//{"3 step",           3,  4 },
  {"Soft limit",       3,  5 },
//{"Bike shift",       3,  6 },
  {"Launch",           3,  7 },
//4 check engine lamps - block 7 offset 425
  {"cel_map",          4,  0 },
  {"cel_mat",          4,  1 },
  {"cel_clt",          4,  2 },
  {"cel_tps",          4,  3 },
  {"cel_batt",         4,  4 },
  {"cel_afr0",         4,  5 },
  {"cel_sync",         4,  6 },
  {"cel_egt",          4,  7 },
//5 port status - block 7 offset 70
  {"port0",            5,  0 },
  {"port1",            5,  1 },
  {"port2",            5,  2 },
  {"port3",            5,  3 },
  {"port4",            5,  4 },
  {"port5",            5,  5 },
  {"port6",            5,  6 },
//6 status6 - block 7 offset 233
  {"EGT warn",         6,  0 },
  {"EGT shutdown",     6,  1 },
  {"AFR warn",         6,  2 },
  {"AFR shutdown",     6,  3 },
  {"Idle VE",          6,  4 },
  {"Idle VE",          6,  5 },
  {"Fan",              6,  6 },
//7 status7 - block 7 offset 351
  {"Knock",            7,  4 },
  {"AC",               7,  5 },
};


// pack/unpack the Megasquirt extended message format header
typedef struct msg_packed_int {
  unsigned char b0;
  unsigned char b1;
  unsigned char b2;
  unsigned char b3;
} msg_packed_int;

typedef struct msg_bit_info {
  unsigned int spare:2;
  unsigned int block_h:1;
  unsigned int block:4;
  unsigned int to_id:4;
  unsigned int from_id:4;
  unsigned int msg_type:3;
  unsigned int offset:11;
} msg_bit_info;

typedef union {
  unsigned int i;
  msg_packed_int b;
  msg_bit_info values;
} msg_packed;

msg_packed rxmsg_id,txmsg_id;

// unpack the vars from the payload of a MSG_REQ packet
typedef struct msg_req_data_packed_int {
  unsigned char b2;
  unsigned char b1;
  unsigned char b0;
} msg_req_data_packed_int;

typedef struct msq_req_data_bit_info {
  unsigned int varbyt:4;
  unsigned int spare:1;
  unsigned int varoffset:11;
  unsigned int varblk:4;
} msg_req_data_bit_info;

typedef union {
  msg_req_data_packed_int bytes;
  msg_req_data_bit_info values;
} msg_req_data_raw;

msg_req_data_raw msg_req_data;


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

	unsigned char len = 0;
	unsigned char buf[8];
	if (CAN_MSGAVAIL == CANbus.checkReceive())           // check if data coming
			{
		CANbus.readMsgBuf(&len, buf); // read data,  len: data length, buf: data buf

		switch (CANbus.getCanId()) { // ID's 1520+ are Megasquirt CAN broadcast frames
		case 1520: // 0
			RPM = (int) (word(buf[6], buf[7]));
			break;
		case 1521: // 1
			SPKADV = (int) (word(buf[0], buf[1]));
			indicator[0] = buf[3]; // engine
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
		case 1530: // 10
			indicator[1] = buf[0]; // status 1
			indicator[2] = buf[1]; // status 2
			indicator[3] = buf[2]; // status 3
			indicator[6] = buf[6]; // status 6
			indicator[7] = buf[7]; // status 7
			break;
		case 1537: // 17
			bstduty = (int) (word(buf[4], buf[5]));
			break;
		case 1548: // 28
			idle_tar = (int) (word(buf[0], buf[1]));
			break;
		case 1551: // 31
			AFR = (int) (word(0x00, buf[0]));
			//afr = buf[0];
			break;
		case 1574: // 54
			indicator[4] = buf[2]; // cel
			break;
		default: // not a broadcast packet

			Serial.write("ID: ");
			Serial.print(CANbus.getCanId());

		}
	}
}
