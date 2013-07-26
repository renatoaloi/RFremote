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
#ifndef RFremote_h
#define RFremote_h

#include "Arduino.h"

#define INT_RF              0
#define PORTA_RF            2
#define LED_DEBUG           13
#define DEBUG               1
#define DEBUG_INT           0
#define BUFF_SIZE           180
#define CMD_SIZE            150

/* old v0.1 pattern params
#define SIGNAL_PATTERN_MIN  10000
#define SIGNAL_PATTERN_MAX  15000
#define SIGNAL_1BYTE_MIN    450
#define SIGNAL_1BYTE_MAX    550
#define SIGNAL_2BYTE_MIN    950
#define SIGNAL_2BYTE_MAX    1050
*/

// new pattern params struct
typedef struct
{
	unsigned int  spaceMin;
	unsigned int  spaceMax;
	unsigned int  dotMin;
	unsigned int  dotMax;
	unsigned int  traceMin;
	unsigned int  traceMax;
	unsigned int  skipFirst;
	unsigned int  skipLast;
} SignalPatternParams;

typedef struct
{
	unsigned char state;
	unsigned char lock;
	unsigned char eof;
	unsigned int  diff;
	unsigned long tempo;
	unsigned char idx;
} RFparams; 

extern volatile RFparams rfparams;

// Receiver class
class RFrecv
{
  public:
    RFrecv();
    RFrecv(SignalPatternParams *signalparams);
    void begin();
    unsigned char available();
    unsigned char cmd[CMD_SIZE];
    
  private:
  		SignalPatternParams _signalparams;
  	  unsigned char buffState[BUFF_SIZE];
  	  unsigned int  buffDiff[BUFF_SIZE];
  	  unsigned char foundIdx[2];
  	  
  	  void init();
    void resetBuffer();
    void resetCmd();
    unsigned char gotPattern();
    unsigned char gotData();
};

// TODO: Transmitter class
// TODO2: create typedef struct to return more information about the signal decoded



#endif