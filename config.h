#pragma once

// #define TEST_ETH

/* Comment/uncomment the following line to toggle printing info for debugging to the Serial Monitor */
#define DEBUG 	// Reduced output compared to !NO_SERIAL

/* Leaving the following line uncommented will prevent ANY sort of output from being printed through the serial port */
#define NO_SERIAL

//#define STORE_PAYLOAD

#define ACCOUNT_FOR_SLUMP   // For mitigating false water flow reports due to shape of flume (0.7" dip below sensor)
#define CROOKS_MODE

#define DL10  A2                          // Level sensor return line connected to analog input pin 2
#define PH500 A3

#define IN2CM(inches) ((inches)*(2.54))   // 1" == 2.54 CM
#define CM2IN(cms)    ((cms)/(2.54))
#define TRIM_PRECISION(fp) (float( int( ((fp) * 100) )) / (100.0))	// 1 um = 0.001 mm, 1 millimeter = 1000 micrometers
#define JSON_SIZE1 20
#define JSON_SIZE2 40
#define JSON_SIZE3 60
#define NSAMPLES 8 //16  //8 //32      // For sensor value smoothing
#define LOOP_DELAY 60  //20
#define SPIKE_THRESH 0.14

// On the Ethernet Shield, CS is pin 4. It's set as an output by default.
// Note that even if it's not used as the CS pin, the hardware SS pin 
// (10 on most Arduino boards, 53 on the Mega) must be left as an output 
// or the SD library functions will not work. 
#define SS_PIN 53
#define CS_PIN 4

////////////////////////////////////////////////////////////////////////
//////////  Dimensions  ////////////
////////////////////////////////////
#define SENSOR_H     9.25  //4.6  // 7.00  // 4.1
#define FILL_H       3.00  //2.6  // 3.00  // 2.0
#define RISER_H      0.00  //3.25  //0.00
#define FLUME_SLUMP  1.2  //0.7

#define EMPTY_LEVEL_MM ((float) ((IN2CM(SENSOR_H)) * 10.00))
#define FULL_LEVEL_MM  ((float) ((IN2CM(FILL_H))   * 10.00))
////////////////////////////////////

#define EMPTY       0
#define OK          1
#define FULL        2
#define OVERFILL    3
#define ERR         4
#define ZERO        5

////////////////////////////////////////////////////////////////////////
//////// NTP Server Config /////////
///////////////////////////////////
#define TIME_SERVER "pool.ntp.org"
#define TIMEZONE_OFFSET -18000L
// #define MAX_CONNECT_ATTEMPTS    3
////////////////////////////////////

////////////////////////////////////////////////////////////////////////
////////// Sketch Config ///////////
////////////////////////////////////
#ifdef DEBUG
  #define DPRINT(...)    Serial.print(__VA_ARGS__)
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define DPRINT(...)
  #define DPRINTLN(...)
#endif

#ifndef NO_SERIAL
  #define SPRINT(...)    Serial.print(__VA_ARGS__)
  #define SPRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define SPRINT(...)
  #define SPRINTLN(...)
#endif

#define ERROR_LOG "log.err"
#define FILENAME1 "jsonlog1.txt"
#define FILENAME2 "jsonlog2.txt"

//#define MIN_POST_INTERVAL 86400  /* Can only post to Pushingbox a max of 100 times per day */
#define HTTP_PORT         80     
//#define HTTPS_PORT        443 	 /* For use with `client.connectSSL()` */
////////////////////////////////////

////////////////////////////////////////////////////////////////////////
/////// Adafruit IoT Config ////////
////////////////////////////////////
//#define IO_USERNAME "halfwaycrooksbeer"
//#define IO_GROUP    "hcflumedata"     /* Name of the IO `Group` encapsulating the 3 sensor data `Feeds` is HCFlumeData" */
//#define IO_KEY      "b83d6fbec6784766a33078b7239079da"  /* A.k.a. "AIO Key" */
//#define IO_SERVER   "io.adafruit.com"  /* Name address for Adafruit IOT Cloud */
////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//////// PushingBox Config /////////
////////////////////////////////////
#define PB_DEVID    "v18CD7A17B3D3A00"    /* Device ID on PushingBox for "JSON To Sheets" Scenario */
//#define PB_DEVID    "vA7E131D6D5C1148"    /* Device ID on PushingBox for "Forward Data to Sheets" Scenario */
//#define PB_DEVID    "v9A2488E8F480094"    /* Device ID on PushingBox for "JSON Test" Scenario */
#define PB_SERVER   "api.pushingbox.com"  /* PushingBox API server */
////////////////////////////////////
////////////////////////////////////
