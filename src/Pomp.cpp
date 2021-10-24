#include "Pomp.h"


class EEParms: public EEUtil
{
public:
	volatile int refTemp 			= 30;     	// referentie temp in 33 graden
	volatile bool koelen 			= false;

	volatile int pompDuty			= 80;      	// % aan
	volatile int pompDebounce		= 15;      	// sec
	volatile int deltaTempPomp		= 2;       	// graden

	volatile int klepDuty			= 60;		// % aan
	volatile int klepDebounce		= 60;		// sec
	volatile int deltaTempKlep		= 4;       	// graden

	long chk1  = 0x01020304;
 
	EEParms(){   }
	virtual ~EEParms(){}

	void setup( )
    {
		if( readLong(offsetof(EEParms, chk1)) == chk1  )
    	{
    		readAll();
    		setRefTemp(readInt(offsetof(EEParms, refTemp)));
    		setKoelen(readBool(offsetof(EEParms, koelen)));

    		setPompDuty(readInt(offsetof(EEParms, pompDuty)));
    		setPompDebounce(readInt(offsetof(EEParms, pompDebounce)));
    		setPompDelta(readInt(offsetof(EEParms, deltaTempPomp)));

    		setKlepDuty(readInt(offsetof(EEParms, klepDuty)));
    		setKlepDebounce(readInt(offsetof(EEParms, klepDebounce)));
    		setKlepDelta(readInt(offsetof(EEParms, deltaTempKlep)));


    		changed = false;
			#ifdef DEBUG
				Serial.println(F("read EEProm"));
			#endif
    	}
		else
		{
			bootCount=0;
		}
    }

	void loop()
	{
		if(changed)
		{
			#ifdef DEBUG
				Serial.println(F("Parms.loop.changed"));
			#endif
			write(offsetof(EEParms, refTemp), refTemp);
			write(offsetof(EEParms, koelen), koelen);

			write(offsetof(EEParms, klepDuty), klepDuty);
			write(offsetof(EEParms, klepDebounce), klepDebounce);
			write(offsetof(EEParms, deltaTempKlep), deltaTempKlep);
			
			write(offsetof(EEParms, pompDuty), pompDuty);
			write(offsetof(EEParms, pompDebounce), pompDebounce);
			write(offsetof(EEParms, deltaTempPomp), deltaTempPomp);

			write(offsetof(EEParms, chk1), chk1);
			EEUtil::writeAll();
			changed = false;
		}
	}

    void setRefTemp( long newVal)  // in .1 degrees
    {
    	refTemp = (int) newVal;
		if(refTemp < 15) refTemp = 15;
		if(refTemp > 45) refTemp = 45;
		changed = true;
    }

    void setKoelen( long newVal)
    {
    	koelen = newVal > 0;
		changed = true;
    }

    void setKlepDuty( long newVal)
    {
    	klepDuty = (int)newVal;
		if(klepDuty < 0) klepDuty = 0;		// 10 %
		if(klepDuty > 100) klepDuty = 100;	// 100 %
		changed = true;
    }

    void setKlepDebounce( long newVal)
    {
    	klepDebounce = (int)newVal;
		if(klepDebounce < 5) klepDebounce = 5;		// 30 sec
		if(klepDebounce > 300) klepDebounce = 300;		// 5 min
		changed = true;
    }

    void setKlepDelta( long newVal)
    {
    	deltaTempKlep = (int)newVal;
		if(deltaTempKlep < 1) deltaTempKlep = 1;			// 0.1 graden
		if(deltaTempKlep > 100) deltaTempKlep = 100;		// 10.0 graden
		changed = true;
    }


    void setPompDuty( long newVal)
    {
    	pompDuty = (int)newVal;
		if(pompDuty < 0) pompDuty = 0;		// 10 %
		if(pompDuty > 100) pompDuty = 100;	// 100 %
		changed = true;
    }

    void setPompDebounce( long newVal)
    {
    	pompDebounce = (int)newVal;
		if(pompDebounce < 5) pompDebounce = 5;		// 30 sec
		if(pompDebounce > 300) pompDebounce = 300;		// 5 min
		changed = true;
    }

    void setPompDelta( long newVal)
    {
    	deltaTempPomp = (int)newVal;
		if(deltaTempPomp < 1) deltaTempPomp =1;			// 1 graad
		if(deltaTempPomp > 5) deltaTempPomp = 5;		// 5 graden
		changed = true;
    }

};
EEParms eeParms	;

#ifdef SPI_CONSOLE
	ISR (SPI_STC_vect){	console.handleInterupt();}
#endif
#ifdef	TWI_PARENT
	ISR(TWI_vect){ parentNode.tw_int(); }
#endif
#ifdef SPI_PARENT
	ISR (SPI_STC_vect){parentNode.handleInterupt();}
#endif

void localSetVal(int id, long val)
{
	// Serial.print("set ");Serial.print(id);Serial.print(" to ");Serial.println(val);

	switch(id )
	{

	case POMP_ID+2:
		pomp.setManual(val);
		break;

	case 11:
		testTemp = val;
		break;

	case 52: 
		eeParms.setRefTemp(val); 
		pomp.force = true;
		klep.force = true;
		break;
	case 53:
	 	eeParms.setKoelen(val); 
		pomp.force = true;
		// klep.force = true;
	 	break;

	case 54: 
		eeParms.setPompDuty(val);
		pomp.force = true;
		break;
	case 55:
		eeParms.setPompDebounce(val);
		pomp.setDebounce_s(eeParms.pompDebounce) ;
		break;
	case 56: eeParms.setPompDelta(val); break;

	case 57: 
		eeParms.setKlepDuty(val);
		klep.force = true;
	 	break;
	case 58:
		eeParms.setKlepDebounce(val);
		klep.setDebounce_s(eeParms.klepDebounce) ;
		break;
	case 59: 
		eeParms.setKlepDelta(val); 
		klep.force = true;
		break;



	case 50: // LED: LOW = on, HIGH = off
		upload(id, !digitalRead(LED_BUILTIN) );		
		break;

	default:
		eeParms.setVal( id,  val);
		break;
	}
}

void nextUpload(int id){
	switch( id ){
 
		case 50: myTimers.nextTimer(TIMER_UPLOAD_LED);		break;
	}
}

int upload(int id)
{
	int ret = 0;
	nextUpload(id);

	switch( id )
	{

	case 8:
		upload(id, JL_VERSION );   
		break;

	case POMP_ID:
		pomp.uploadIsRunning();
		break;
	case POMP_ID+1:
		pomp.uploadState() ;  
		break;	
	case POMP_ID+2:
		pomp.uploadManual();
		break;

	case KLEP_ID:
		klep.uploadIsRunning();
		break;
	case KLEP_ID+1:
		klep.uploadState() ;  
		break;	
	case KLEP_ID+2:
		klep.uploadManual();
		break;

	case TEMP_AANVOER_ID: tempAanvoer.upload(); 		break;

	#ifndef DEBUG

		case TEMP_KRUIP_RUIMTE_ID: tempKruipRuimte.upload();  	break;
		case TEMP_POMP_UIT_ID: tempPompUit.upload();   			break;
		case TEMP_AFVOER_ID: tempAfvoer.upload();   			break;
		case TEMP_KEUKEN_ID: tempKeuken.upload();   			break;
		case TEMP_MUUR_2_ID: tempRetour2.upload();   			break;

	#endif

	case TEMP_GRENS_MUUR_ID: tempGrensMuur.upload();   		break;
	case TEMP_MUUR_IN_ID: tempMuurIn.upload();   			break;

	case 52: upload(id, eeParms.refTemp);  	break;
	case 53: upload(id, eeParms.koelen);  	break;

	case 54: upload(id, eeParms.pompDuty); break;
	case 55: upload(id, eeParms.pompDebounce);  break;
	case 56: upload(id, eeParms.deltaTempPomp);  break;

	case 57: upload(id, eeParms.klepDuty); break;
	case 58: upload(id, eeParms.klepDebounce);  break;
	case 59: upload(id, eeParms.deltaTempKlep);  break;

	#ifdef VOLTAGE_PIN
		case 11: upload(id, vin.val);    	break;
	#endif

	default:
		if( 1==2
		 ||	parentNode.upload(id)>0
		 ||	eeParms.upload(id)>0
		){}
		break;
	}
	return ret;
}


int upload(int id, long val) { return upload(id, val, millis()); }
int upload(int id, long val, unsigned long timeStamp)
{
	nextUpload(id);
	return parentNode.txUpload(id, val, timeStamp);
}
int uploadError(int id, long val)
{
	return parentNode.txError(id, val);
}

int handleParentReq( RxItem *rxItem)  // cmd, to, parm1, parm2
{
	#ifdef DEBUG
		parentNode.debug("Prnt<", rxItem);
	#endif

	if( rxItem->data.msg.node==2
	 || rxItem->data.msg.node==0
	 || rxItem->data.msg.node==parentNode.nodeId )
	{
		return localRequest(rxItem);
	}

	if(parentNode.nodeId==0)
	{
		#ifdef DEBUG
			parentNode.debug("skip", rxItem);
		#endif

		return 0;
	}

	#ifdef DEBUG
		parentNode.debug("forward", rxItem);
	#endif

	#ifndef SKIP_CHILD_NODES
	//	return childNodes.putBuf( req );
	#endif
}


int localRequest(RxItem *rxItem)
{
	#ifdef DEBUG
		parentNode.debug("local", rxItem);
	#endif

	int ret=0;

	switch (  rxItem->data.msg.cmd)
	{
	// case 't':trace(); break;
	//case 'x': parentNode.tw_restart(); break;

	case 's':
		localSetVal(rxItem->data.msg.id, rxItem->data.msg.val);
		break;
//	case 's':
	case 'S':
		localSetVal(rxItem->data.msg.id, rxItem->data.msg.val);
		upload(rxItem->data.msg.id);
		break;
	case 'r':
	case 'R':
		upload(rxItem->data.msg.id);
		break;
	case 'B':
		wdt_enable(WDTO_15MS);
		while(true){
			delay(500);
			asm volatile ("  jmp 0");
		}
		break;
	default:
		eeParms.handleRequest(rxItem);
		// util.handleRequest(rxItem);
		break;
	}

	return ret;
}


void setup()    //TODO
{
	wdt_reset();
	wdt_disable();

	Serial.begin(115200);
	
	pinMode( LED_BUILTIN, OUTPUT);
	pinMode( COLD_WATER_PIN, OUTPUT);

	Serial.println();
	#ifdef DEBUG
		Serial.println(F("DEBUG pomp..."));
	#else
		Serial.println(F("pomp..."));
	#endif

	eeParms.onUpload(upload);
	eeParms.setup();

	int nodeId = NODE_ID;

	parentNode.onReceive( handleParentReq);
	parentNode.onError(uploadError);
	parentNode.onUpload(upload);
	parentNode.nodeId = nodeId;
	parentNode.isParent = true;

	#ifdef TWI_PARENT
		parentNode.begin();
		//parentNode.txBufAutoCommit = true;
	#endif


	#ifdef NETW_PARENT_SPI
		bool isSPIMaster = false;
		parentNode.setup( SPI_PIN, isSPIMaster);
		parentNode.isParent = true;
	#endif


	tempAanvoer.onUpload(upload, TEMP_AANVOER_ID);
	tempAanvoer.onError(uploadError);

	tempMuurIn.onUpload(upload, TEMP_MUUR_IN_ID);
	tempMuurIn.onError(uploadError);
	tempGrensMuur.onUpload(upload, TEMP_GRENS_MUUR_ID);
	tempGrensMuur.onError(uploadError);
	#ifndef DEBUG


		tempKruipRuimte.onUpload(upload, TEMP_KRUIP_RUIMTE_ID);
		tempKruipRuimte.onError(uploadError);

		tempPompUit.onUpload(upload, TEMP_POMP_UIT_ID);
		tempPompUit.onError(uploadError);

		tempAfvoer.onUpload(upload, TEMP_AFVOER_ID);
		tempAfvoer.onError(uploadError);

		tempKeuken.onUpload(upload, TEMP_KEUKEN_ID);
		tempKeuken.onError(uploadError);

		tempRetour2.onUpload(upload, TEMP_MUUR_2_ID);
		tempRetour2.onError(uploadError);


	#endif


	pomp.onCheck(evaluatePomp);
	pomp.onUpload(upload, POMP_ID);
	pomp.setAutoPeriode_s( AUTO_ACTIVATE_PERIODE_IN_SEC);
	pomp.setDebounce_s(eeParms.pompDebounce);  
	pomp.dutyCycleMode = true;
	pomp.numberOfStates = 20; 
	// pomp.setPompDuty(eeParms.pompDuty);
	pomp.sayHello = true;

	klep.onCheck(evaluateKlep);
	klep.onUpload(upload, KLEP_ID);
	klep.setDebounce_s(eeParms.klepDebounce);
	klep.dutyCycleMode = true;
	klep.numberOfStates = 10; 
	klep.sayHello = true;  


	#ifdef DEBUG
		pomp.test(true);  // show buildin led
	#endif
 
	myTimers.nextTimer(TIMER_TRACE, 1);
	myTimers.nextTimer(TIMER_UPLOAD_ON_BOOT, 0);
 

	wdt_reset();
	wdt_enable(WDTO_8S);
}

int evaluateKlep(bool on, bool isRunning){

	// test;
	// if(eeParms.koelen){

	// 	return eeParms.klepDuty>0 ? eeParms.klepDuty: 1;
	// }
	// if(on ) return 0;


	/*
		when temp difference between in and out is enough 
		we stop external hot water so the pomp can do its job
	*/
	if( tempAanvoer.errorCnt > 2
	 && ( tempMuurIn.errorCnt > 2
       || tempGrensMuur.errorCnt > 2 )
	){
		return on ? 0 : -1;
	}	


	float deltaTempInUit =  tempMuurIn.temp - tempGrensMuur.temp;

	if( tempAanvoer.temp >= eeParms.refTemp * 100 
	 || deltaTempInUit >= ( eeParms.deltaTempKlep * 100 )
	){
		return eeParms.klepDuty>0 ? eeParms.klepDuty: 1;

	} else {		
		return on ? 0 : -1;
	}

	return -1;
}

int evaluatePomp(bool on, bool isRunning){

	// return 0; // 0=off, 1=on, -1=nop, >1 = %
	// pomp.dutyCycle is set by a return value > 1 !!

	/*
	 *  when koelen always keep the pomp running for now. 
	 *  maybe later stop when the kamer gets to cold.
	 */
	if(eeParms.koelen){

		return eeParms.pompDuty>0 ? eeParms.pompDuty: 1;
	}

	/*
	 *  When sensor error keep pomp running. 
	 */
	if( tempAanvoer.errorCnt > 2
	 || tempAanvoer.errorCnt > 2  
	){
		if(millis() < 3000) return -1;

		return 1;  	
	}

	float deltaTempInUit =  tempMuurIn.temp - tempGrensMuur.temp;
	/*
	 *  When aanvoer > ref pomp running the configured pompDuty
	 *  Else stop pomp
	 */
	if( tempAanvoer.temp >= eeParms.refTemp * 100
	 || deltaTempInUit >= ( eeParms.deltaTempPomp * 100 )
	){
		return eeParms.pompDuty>0 ? eeParms.pompDuty: 1;
		// return eeParms.pompDuty;

	} else {
		return on ? 0 : -1;
		// if(on) return 0;  	// Serial.println("Switch off");
	}

	return -1;
}

void loop()  //TODO
{
	wdt_reset();

	if( digitalRead(COLD_WATER_PIN) != eeParms.koelen  ) {

		digitalWrite(COLD_WATER_PIN, eeParms.koelen);	
	}


	if(eeParms.koelen != koelen){
		pomp.force = true;
		koelen = eeParms.koelen;
	}

	tempAanvoer.loop();
	tempGrensMuur.loop();
	tempMuurIn.loop();
	#ifndef DEBUG

		tempKruipRuimte.loop();
		tempPompUit.loop();
		tempAfvoer.loop();
		tempKeuken.loop();
		tempRetour2.loop();

	#endif

	pomp.loop();  
	klep.loop();

	eeParms.loop();
	parentNode.loopSerial();
	parentNode.loop();

	if( parentNode.isReady() 
	 && ! parentNode.isTxFull()
	){
		if( myTimers.isTime(TIMER_UPLOAD_ON_BOOT)
		){
			switch( uploadOnBootCount )
			{
				case 1:
					if(millis()<60000) upload(1);
					break;    // boottimerr

				#ifdef VOLTAGE_PIN
					case 3: upload(11); break;  	// Vin
				#endif	
				case 4: upload(3); break;  // upload bootCount
				case 5: upload(50); break;	

				case 7: upload(30); break;  // pomp aan?
				case 8: upload(31); break;  // Cycle?
				case 9: upload(32); break;  // manual?

				case 10: upload(52); break;  // refTemp?
				case 11: upload(53); break;  // koelen?

				case 15: upload(54); break;  // pompDuty?
				case 16: upload(55); break;  // pompDebounce?
				case 17: upload(56); break;  // deltaTempPomp?

				case 20: upload(57); break;  // klepDuty?
				case 21: upload(58); break;  // klepDebounce?
				case 22: upload(59); break;  // deltaTempKlep?

				case 24: upload(40); break;  // klep aan?
				case 25: upload(41); break;  // Cycle?
				case 26: upload(42); break;  // manual?

				case 30: myTimers.timerOff(TIMER_UPLOAD_ON_BOOT); break;			
			}

			uploadOnBootCount++;
			myTimers.nextTimerMillis(TIMER_UPLOAD_ON_BOOT, TWI_SEND_ERROR_INTERVAL);
		}
 

		if( myTimers.isTime(TIMER_UPLOAD_LED)){
			upload(50);
			myTimers.nextTimer(TIMER_UPLOAD_LED);
		}	
	}

	#ifdef DEBUG		
		if( myTimers.isTime(TIMER_TRACE)){ trace();}
	#endif
}


#ifdef DEBUG
void trace( )
{
	myTimers.nextTimer(TIMER_TRACE, TRACE_SEC);

	Serial.print(F("@"));
	Serial.print(millis() / 1000);
	Serial.print(F(" - "));
	Serial.print(F("Pomp refTemp="));Serial.print(eeParms.refTemp);
	Serial.print(F(", koelen="));Serial.print(eeParms.koelen);
	Serial.print(F(", pompDuty=")); Serial.print(eeParms.pompDuty );
	Serial.print(F(", klepDuty=")); Serial.print(eeParms.klepDuty );
	Serial.print(F(", testTemp=")); Serial.print(testTemp);
	Serial.println();

	// parentNode.trace("pn");

	tempAanvoer.trace((char *)"Aanv");
 
	//	tempPin6Kruipruimte.trace((char *)"KruipR");
	//	tempPin6Grens.trace((char *)"Grens");
 
	// pomp.trace("pmp");
	klep.trace("klp");
	Serial.flush();
}
#endif


//void calcPompKPI()
//{
//
//	if( pomp.isRunning() )
//	{
//		millisPompUit = millisPompUit + (millis() - millisPompPrev);
//	}
//	else
//	{
//		millisPompAan = millisPompAan + (millis() - millisPompPrev);
//	}
//	millisPompPrev = millis();
//	Serial.print("millisPompAan=");
//	Serial.println(millisPompAan);
//}