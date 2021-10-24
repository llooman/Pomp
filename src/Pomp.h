#ifndef _Pomp_H_
#define _Pomp_H_
#define JL_VERSION 20000

// #define DEBUG
// RAM:   [=====     ]  46.2% (used 946 bytes from 2048 bytes)
// Flash: [=======   ]  73.8% (used 22660 bytes from 30720 bytes)
// #define OTHER_TEMPS
// RAM:   [======    ]  63.5% (used 1300 bytes from 2048 bytes)
// Flash: [========  ]  75.2% (used 23090 bytes from 30720 bytes)

#include "Arduino.h"
#include <inttypes.h>
#include <DS18B20.h>
#include "EEUtil.h"
#include <MyTimers.h>
#include <avr/wdt.h>

#include <NetwTWI.h>
#include <Relais2.h>


#define NODE_ID  3
// #define SERIAL_PORT 0
#define DS18B20_PIN 11
#define POMP_PIN 6
#define NORMALY_ON_PIN 7		// klep warm water retour
#define COLD_WATER_PIN 9

#define POMP_ID 30
#define KLEP_ID 40

#define TEMP_AANVOER_ID 20		// aanvoer CV ds20  ds19  
#define TEMP_AFVOER_ID 15		// retour CV  rood
#define TEMP_KRUIP_RUIMTE_ID 12 // Koeling   wit

#define TEMP_POMP_UIT_ID 14		// Muur in rood 
#define TEMP_KEUKEN_ID 16 		// Keuken uit
#define TEMP_GRENS_MUUR_ID 11	// Grensmuur uit
#define TEMP_MUUR_2_ID 21		// Muur 2 uit
#define TEMP_MUUR_IN_ID 22		// Muur 3 uit 

//#define SERIAL_PARENT
//#include <NetwSerial.h>
//NetwSerial parentNode;


#ifdef DEBUG
	#define SERIAL_CONSOLE 77
	#include <NetwSerial.h>
	NetwSerial console;
#endif

#define TWI_PARENT

NetwTWI parentNode;
// #define TWI_RELAIS_PIN 8

//#define SPI_PARENT
//#include <NetwSPI.h>
//NetwSPI parentNode(NODE_ID);



/*
//TODO log het vermogen
 
 *
2021-01-16 2.0.0 - refator twi 
2020-08-20 1.2.0 - add Koelen 
                 - add WaterSupply relais
				 - new hardware 
				 - remove i2c relais



2019-05-10 re-add netw Relaies on PIN 8

2017-05-05 TWI_FREQ 200000L

2017-02-14 Netw2.0 update

2017-01-12 add bootCount
           retry
		   HomeNetH.h 2017-01-10  refactor bootTimer
           new EEProm class replacing EEParms and Parms
		   System::ramFree()
		   set netw.netwTimer   moved to NETW class
		   HomeNetH.h 2017-01-10  netw.sendData retry on send error
		   ArdUtils.h 2017-01-12 fix signatureChanged

2017-01-08 HomeNetH.h 2017-01-08 fix volatile
           add toggle when Pomp Running (inactive)

2017-01-07 add netw.execParms(cmd, id, value) in execParms
           add if(req->data.cmd != 21 || parms.parmsUpdateFlag==0 ) in loop.handle netw request
           move parms.loop(); before loop.handle netw request
           HomeNetH.h 2017-01-06

2017-01-07 TODO eeparms.exeParms  case 3: netw.netwTimer = millis() + (value * 1000); //  netwSleep


// 5.0 2016-04-03 parms class
// 4.0 events + aync DS18B20
// 3.01 2015-09-20 New EEPROM + I2C timmings
// 3.0 2015-02-08 switch case + refactoring + beter testing
// 2.9 2015-01-24 volatile, disable cvTemp, CMD_getDeviceStatus.lastErrorTimeStamp for errorId
// 2.8 2015-01-21 add last error
// 2.7 2014-12-04 sync with ketel temp
// 2.6 2014-06-14 retries in homenet and here
// 2.5 2014-06-18  add checksum
// 2.4 2014-05-20  add hang
// 2.3 2014-05-10  ReadTemp
// 2.2 2014-04-20  add eeprom parameters
// 2.1 2014-01-31  add temp sensors on pin 5 and 6, remove queue for mem space
// PompSchakelaar v 2.0 2013-10-04
*/
const byte ds20[] PROGMEM = {0x28, 0xff, 0x8b, 0x97, 0x66, 0x14, 0x02, 0x8b};   // waterdicht
const byte ds13[] PROGMEM = {0x28, 0xff, 0x0e, 0x77, 0x82, 0x15, 0x01, 0xa4};   // rood
const byte ds02[] PROGMEM = {0x28, 0xff, 0xc8, 0x75, 0x82, 0x15, 0x01, 0x1d};   // wit

const byte ds110[] PROGMEM = {0x28, 0xff, 0x68, 0x18, 0xa0, 0x16, 0x05, 0xcc}; //28 FF 68 18 A0 16 5 CC
const byte ds111[] PROGMEM = {0x28, 0xff, 0x7f, 0xd9, 0x83, 0x16, 0x03, 0xfa}; //28 FF 7F D9 83 16 3 FA
const byte ds112[] PROGMEM = {0x28, 0xff, 0xec, 0xf5, 0x83, 0x16, 0x03, 0x61}; //28 FF EC F5 83 16 3 61

const byte ds113[] PROGMEM = {0x28, 0xff, 0x94, 0x31, 0x85, 0x16, 0x05, 0x09}; //addr: 28 FF 94 31 85 16 5 9
const byte ds114[] PROGMEM = {0x28, 0xff, 0xff, 0x36, 0xa0, 0x16, 0x05, 0x4f}; //addr: 28 FF FF 36 A0 16 5 4F


#define TRACE_SEC 3


#define AUTO_ACTIVATE_PERIODE_IN_SEC  86400   // = 1 dag

#define MINIMAL_ACTIVE_IN_SEC  60
#define MINIMAL_INACTIVE_IN_SEC 60

//MySensor gw;            r// wireless

#ifdef DEBUG
DS18B20 tempAanvoer(DS18B20_PIN );            // _ds101  ds12 ds03
#else
DS18B20 tempAanvoer     (DS18B20_PIN, ds20 );            // _ds101  ds12 ds03
#endif
DS18B20 tempMuurIn      (DS18B20_PIN, ds114);   
DS18B20 tempGrensMuur   (DS18B20_PIN, ds02 );   //ds09 ds12

#ifndef DEBUG
DS18B20 tempKruipRuimte (DS18B20_PIN, ds13 );   //ds08 ds106
DS18B20 tempPompUit      (DS18B20_PIN, ds110);   //ds09 ds12
DS18B20 tempAfvoer      (DS18B20_PIN, ds111);   //ds08 ds106
DS18B20 tempKeuken      (DS18B20_PIN, ds112 );   //ds08 ds106
DS18B20 tempRetour2      (DS18B20_PIN, ds113);   
#endif


#define INVERTED true
#define START_RUNNING true

 
//  "default changed to off "  
Relais2 pomp(POMP_PIN, INVERTED, START_RUNNING );  	      // the pomp relais in on pin 2, inverted logic
Relais2 klep(NORMALY_ON_PIN);    							// the klep default open  , INVERTED, START_RUNNING
 

#define TIMERS_COUNT 3
MyTimers myTimers(TIMERS_COUNT);
#define TIMER_TRACE 0
#define TIMER_UPLOAD_LED 1
#define TIMER_UPLOAD_ON_BOOT 2

int 	uploadOnBootCount=0; 

// kpi's
//long millisPompPrev = millis();
//long millisPompAan = 0;
//long millisPompUit = 0;

unsigned long pompChanged_t = millis();

bool toggled = false;

int testTemp = 33;
bool koelen = false;   // detect changes to act direct

	 
int evaluateKlep(bool on, bool isRunning);
int evaluatePomp(bool on, bool isRunning);
void localSetVal(int id, long val);
int  upload(int id, long val, unsigned long timeStamp);
int  upload(int id, long val) ;
int  upload(int id);
int  uploadError(int id, long val);
int  handleParentReq( RxItem *rxItem) ;
int  localRequest(RxItem *rxItem);
void trace( );

#endif