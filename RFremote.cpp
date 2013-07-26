/*
 * RFremote
 * Version 0.1 July, 2013
 * Copyright 2013 Renato Aloi
 * For details, see http://www.seriallink.com.br
 *
 * Interrupt code based on EMTECOrec by Renato Aloi
 * Also influenced by IRremote by Ken Shirriff @ http://arcfn.com
 *
 * Code based on PPA TOK 433Mhz Code Learning remote control  
 */
#include "Arduino.h"
#include "RFremote.h"

volatile RFparams   rfparams;
//SignalPatternParams _signalparams;

void intSinal();

// Method: Constructor
// Desc: Put FACTORY DEFAULT initial values to variables and arrays
RFrecv::RFrecv()
{
	init();
	
	// Default config for PPA TOK 433Mhz remote
	_signalparams.spaceMin = 10000;
	_signalparams.spaceMax = 15000;
	_signalparams.dotMin = 450;
	_signalparams.dotMax = 550;
	_signalparams.traceMin = 950;
	_signalparams.traceMax = 1050;
	_signalparams.skipFirst = 0;
	_signalparams.skipLast = 0;
}

// Method: Constructor
// Desc: Put CUSTOM initial values to variables and arrays
RFrecv::RFrecv(SignalPatternParams *signalparams)
{
	init();
	
	// custom config
	_signalparams.spaceMin = signalparams->spaceMin;
	_signalparams.spaceMax = signalparams->spaceMax;
	_signalparams.dotMin = signalparams->dotMin;
	_signalparams.dotMax = signalparams->dotMax;
	_signalparams.traceMin = signalparams->traceMin;
	_signalparams.traceMax = signalparams->traceMax;
	_signalparams.skipFirst = signalparams->skipFirst;
	_signalparams.skipLast = signalparams->skipLast;
}

void RFrecv::init()
{
	rfparams.lock = 0;
	rfparams.eof = 0;
	rfparams.diff = 0;
	rfparams.tempo = micros();
 	rfparams.state = 0;
	rfparams.idx = 0;
  
	foundIdx[0] = 0;
	foundIdx[1] = 0;
  
	resetBuffer();
	resetCmd();
}

// Method: begin
// Desc: Configures pins and interrupts
void RFrecv::begin()
{
	// LED debug pin config
	pinMode(LED_DEBUG, OUTPUT);
	digitalWrite(LED_DEBUG, LOW);
	
	// RF pin config
	pinMode(PORTA_RF, INPUT);
	digitalWrite(PORTA_RF, HIGH);
	
	// Interrupt to trigger when RF pin state change
	attachInterrupt(INT_RF, intSinal, CHANGE);
	
	
}

// Method: available
// Desc: Check for data arrival, for pattern match, for data timmings
//       and fill cmd array if result is true
unsigned char RFrecv::available()
{
	unsigned char ret = 0;
	
	// Check if there is a lock from interrupt
	// Check also if not end of buffer
	if (rfparams.lock && !rfparams.eof)
	{
		// Putting state and time into buffer
		buffState[rfparams.idx] = rfparams.state;
		buffDiff[rfparams.idx]  = rfparams.diff;
		
		// release buffer lock
		rfparams.lock = 0;
		
		// increment idx
   	rfparams.idx++;
   	if (rfparams.idx > BUFF_SIZE - 1)
   	{
   		// if end of buffer, set flag eof 
   		// and put zero to idx
     	rfparams.eof = 1;
     	rfparams.idx = 0;
   	}
	}
	else if (rfparams.eof)
	{
		// if end of buffer, starts to check 
		// signal pattern and data
		if (gotPattern())
		{
			// if signal pattern is correct
			// check data and fill cmd[] with command data
			if (gotData())
			{
				// if got here, command available!
				ret = 1;
				
				
				if (DEBUG)
				{
					rfparams.lock = 1;	
					unsigned long t = millis() + 200;			
					digitalWrite(LED_DEBUG, HIGH);
					while(t>millis());
					t = millis() + 200;
					digitalWrite(LED_DEBUG, LOW);
					while(t>millis());
					t = millis() + 200;
					digitalWrite(LED_DEBUG, HIGH);
					while(t>millis());
					t = millis() + 200;
					digitalWrite(LED_DEBUG, LOW);
					while(t>millis());
					t = millis() + 200;
					rfparams.lock = 0;							
				}		
	
				
			}
		}
		// check if there is errors
		if (!ret)
		{
			// reset command array in case of error
  			resetCmd();
		}
		
		// release eof flag
		rfparams.eof = 0;
		
		// clear the buffer to start over again
		resetBuffer();
	}	
	
	return ret;
}

// Method: gotPattern
// Desc: Check if pattern recog is achieved
unsigned char RFrecv::gotPattern()
{	
	unsigned char foundItTwice = 0;
	unsigned char foundCount = 0;
	
	// loop through buffer timmings to find
	// two (2) delays between 10000us to 15000us (its configurable)
	for (int i = 0; i < BUFF_SIZE; i++)
	{	
		// check if pattern is found
		if (buffDiff[i] > _signalparams.spaceMin && buffDiff[i] < _signalparams.spaceMax)
		{
			foundIdx[foundCount] = i;
			
			/*		
			if (foundCount == 0) 
				foundIdx[foundCount] = i + _signalparams.skipFirst;
			else
				foundIdx[foundCount] = i - _signalparams.skipLast;
			*/
			
			foundCount++;
		}
		
		// if found two items that correspond to pattern
		// return true
		if (foundCount > 1)
		{
			foundItTwice = 1;
			break;
		}
	}
	return foundItTwice;
}

// Method: gotData
// Desc: Check if data has correct timmings and fill cmd array
unsigned char RFrecv::gotData()
{
	unsigned char isError = 0;
	unsigned int  cmdIdx  = 0;

	// Check if gotPattern found the two pattern's needed here
	if (foundIdx[0] && foundIdx[1]) 
	{
		// loop through found indexes
		for (int i = foundIdx[0] + 1 + _signalparams.skipFirst; i < foundIdx[1] - _signalparams.skipLast; i++)
		{
			// check if its a 500us timming
			// so that case fill 1 byte into cmd array
		  if (buffDiff[i] > _signalparams.dotMin && buffDiff[i] < _signalparams.dotMax)
		  {
		    cmd[cmdIdx] = (buffState[i] ? '1' : '0');
		  }
		  // check if its a 1000us timming
			// so that case fill 2 byte into cmd array
		  else if (buffDiff[i] > _signalparams.traceMin && buffDiff[i] < _signalparams.traceMax)
		  {
		    cmd[cmdIdx] = (buffState[i] ? '1' : '0');
		    cmdIdx++;
		    cmd[cmdIdx] = (buffState[i] ? '1' : '0');
		  }
		  else
		  {
		  		// if timmings not 500us or 1000us,
		  		// so, it is an invalid signal, discard
		    isError = 1;
		    break;
		  }
	
			// increment cmd array index
			// and check for buffer overflow
		  cmdIdx++;
		  if (cmdIdx > CMD_SIZE - 1)
		  {
		    isError = 1;
		    break;
		  }
		}
	}
	else
	{
		// if foundIdx's values are zero (0)
		// means not valid pattern, abort buffer
		isError = 1;
	}
	
	
	return !isError;
}

// Method: resetBuffer
// Desc: Initiate buffer's arrays
void RFrecv::resetBuffer()
{
  for (int i = 0; i < BUFF_SIZE; i++)
  {
    buffState[i] = 0;
    buffDiff[i] = 0;
  }
}

// Method: resetCmd
// Desc: Initiate cmd's arrays
void RFrecv::resetCmd()
{
  for (int i = 0; i < CMD_SIZE; i++)
  {
    cmd[i] = '\0';
  }
}

// Method: intSinal
// Desc: Interrupt function triggered by attachInterrupt()
void intSinal()
{
	// Run only if buffer lock is lose
	// and is not buffer's end
	if (!rfparams.lock && !rfparams.eof)
	{
		// Lock immediately!
		rfparams.lock = 1;
		
		// Read pin's state and calculate time interval
		rfparams.state = digitalRead(PORTA_RF);
		rfparams.diff = micros() - rfparams.tempo;
		rfparams.tempo = micros();
		
		// Debug led, it blinks trhough state changes
		if (DEBUG_INT) digitalWrite(LED_DEBUG, rfparams.state);
	}
}

