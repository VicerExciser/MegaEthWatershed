/**
 *  ========  WATERSHED DATA COLLECTOR  ========
 *  Board:    Arduino Mega 2560 + Ethernet/SD Shield
 *  Author:   Austin Condict
 *  Email:    halfwaycrooksbeer@gmail.com
 *  Company:  Halfway Crooks Beer & Brewing (Atlanta, GA)
 *  Purpose:  To comply with requests made by the Atlanta Department of Watershed 
 *            Management for the regular data collection of upstream depth measurements
 *            for monitoring wastewater discharge flow levels through a Palmer-Bowlus flume.
 *
 *  Description:
 *    This sketch will continuously poll an EchoPod DL10 Ultrasonic Liquid Level 
 *    Transmitter for a 4-20 mA signal output to be translated into a water level
 *    measurement up to 1.25 meters. Sensor data is then relayed to a Googe App Script
 *    to be appended to a Google Sheet corresponding to current year and month. 
 *    A level measurement must be logged once every 15 seconds. An NTP server provides real time.
 *    Additionally, a pH sensor's data is collected. All sensor data & associated timestamps
 *    are stored as a JSON string with 60 entries, which is sent to the Google App Script
 *    thru Pushingbox (an HTTP-friendly middleman, necessary because the Ethernet shield does 
 *    not support HTTPS requests which are required by Google) only once all 60 sensor logs
 *    have been prepared. The microSD card reader is used to reduce heap consumption by the
 *    JSON data strings; after the first 20 sensor readings, the results are logged to a file;
 *    another SD filewrite occurs after the following 20 readings; once 60 readings have occurred,
 *    the first 40 readings are read back from the SD card and concatenated with the final 20.
 *    SD logging is also (somewhat) in place for critical error logs.
 *    A maximum of 100 requests may be made to Pushingbox per day, thus 60 entries (each representing
 *    a 15-second interval) sent every 15 minutes will total to precisely 96 daily GET requests.
 *    
 *  Flume Specs & DL10 Configurations:
 *    >>  D = 4" (diameter of flume)
 *    >>  Sensor range: 2" - 49.2"
 *    >>  Sensor Height set to 4" (flush with ceiling of flume pipe) 
 *    >>  Fill Height set to 2" (dist. from floor of flume to the maximum liquid height)
 *    >>  My own calculations assert that 0.413" < Fill Height (Ha) < 2.413"
 *    >>  Riser Height set to 0" (dist. the sensor is recessed within a riser)
 *    >>  Loop Fail-Safe set to Hold Last Value -- out current will stay constant when sensor loses echo confidence
 *    >>  Output at Empty set to 4 mA at Bottom (a 4mA output current will indicate an empty flume)
 *    >>  Conversely, the output current will be 20mA when the sensor measures a full tank
 *    >>  Startup Condition set to Empty (4 mA) -- sensor will output 4mA for ~10 seconds while powering up
 *    
 *  Operational notes gathered while testing:
 *    >>  Any ouput current greater than 19 mA will show as OVERFILL, and can potentially damage the Arduino
 *    >>> (for the 22mA Overfill setting, multimeter showed more than 5.5V input to the A0 pin, which is really bad)
 *    >>> IF FLUME LEVEL EVER GETS TO 2" OR ABOVE, THE READINGS WILL BE UNRELIABLE & IT MAY FRY THE BOARD
 *    >>  Additionally, the first 2" in front of the ultrasonic transducer is a dead zone, will not give accurate results
 *    >>  Fill Height config cannot go above 2", so really need to verify that is the maximum height the water may ever reach
 *    >>> Else, the sensor will need to be recessed within a riser some distance above the ceiling of the flume pipe
 *
 *  Google Sheet URL (named "Halfway Crooks Palmer-Bowlus Flume Levels"): 
 *    https://docs.google.com/spreadsheets/d/1es8jdW2go2Bgrn7Iz5L8bNpIfehzsKK6hWMP0CXh4IQ/edit#gid=0
 *
 *  "ATLWatershed" Adafruit IO Dashboard URL:
 *    https://io.adafruit.com/halfwaycrooksbeer/dashboards/atlwatershed
 *
 *  "DataLogger" Google Script URLs:
 *    https://script.google.com/macros/s/AKfycbwcei1kWqE1zLnNm2lciSfsJhxnNFaKASewn29hSIBjGAKZ3m-f/exec
 *    https://script.google.com/d/1SFXiEMfgBKASiL6sLwoz1K0C-fON2L0lmLvGwU1nsQs3O8hGncCjgUJr/edit?mid=ACjPJvHPPrrUSu6Y01oixI3Uk7KRTzWTbI8REpwuVfvuCgp3rHJNPGhr0m0JIgOnR_qFxRGLGRBnd3aoqsR48upLUtTLIuNHsmeCAaxWaqd3k-HE3oA6NkqH5ovDaTr-VObGAix6T5VeAg&uiv=2
 *    
 *   "JSON To Sheets" Pushingbox scenario: 
 *    https://www.pushingbox.com/scenarios_edit.php?scenario_ID=124699
 *      
 *   To test the Pushingbox scenario's throughput to the Google Apps Script:
 *    http://api.pushingbox.com/pushingbox?devid=v18CD7A17B3D3A00&json={%222%2f20%2f2020%2c10%3a39%3a14+PM%22%3a%221.04%2c0.00%2c6.17%22%2c%222%2f20%2f2020%2c10%3a39%3a29+PM%22%3a%221.09%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a39%3a44+PM%22%3a%221.12%2c0.00%2c6.15%22%2c%222%2f20%2f2020%2c10%3a39%3a59+PM%22%3a%221.14%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a40%3a15+PM%22%3a%221.15%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a40%3a30+PM%22%3a%221.16%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a40%3a45+PM%22%3a%221.17%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a41%3a00+PM%22%3a%221.18%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a41%3a15+PM%22%3a%221.20%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a41%3a31+PM%22%3a%221.21%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a41%3a46+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a42%3a01+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a42%3a16+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a42%3a31+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a42%3a46+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a43%3a01+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a43%3a17+PM%22%3a%221.22%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a43%3a32+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a43%3a47+PM%22%3a%221.22%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a44%3a02+PM%22%3a%221.22%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a44%3a17+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a44%3a32+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a44%3a47+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a45%3a03+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a45%3a18+PM%22%3a%221.22%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a45%3a33+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a45%3a48+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a46%3a03+PM%22%3a%221.22%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a46%3a19+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a46%3a34+PM%22%3a%221.22%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a46%3a49+PM%22%3a%221.22%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a47%3a04+PM%22%3a%221.23%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a47%3a19+PM%22%3a%221.23%2c0.00%2c6.10%22%2c%222%2f20%2f2020%2c10%3a47%3a34+PM%22%3a%221.23%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a47%3a49+PM%22%3a%221.23%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a48%3a05+PM%22%3a%221.23%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a48%3a20+PM%22%3a%221.23%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a48%3a35+PM%22%3a%221.22%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a48%3a50+PM%22%3a%221.22%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a49%3a05+PM%22%3a%221.22%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a49%3a20+PM%22%3a%221.21%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a49%3a35+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a49%3a51+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a50%3a06+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a50%3a21+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a50%3a36+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a50%3a51+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a51%3a06+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a51%3a21+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a51%3a37+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a51%3a52+PM%22%3a%221.21%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a52%3a07+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a52%3a22+PM%22%3a%221.21%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a52%3a37+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a52%3a52+PM%22%3a%221.21%2c0.00%2c6.18%22%2c%222%2f20%2f2020%2c10%3a53%3a08+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a53%3a23+PM%22%3a%221.21%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a53%3a38+PM%22%3a%221.21%2c0.00%2c6.16%22%2c%222%2f20%2f2020%2c10%3a53%3a53+PM%22%3a%221.21%2c0.00%2c6.13%22%2c%222%2f20%2f2020%2c10%3a54%3a08+PM%22%3a%221.21%2c0.00%2c6.13%22}
 *
 *   Resources & References:
 *    https://www.flowline.com/wp-content/uploads/WebCal_m.pdf
 *    https://www.flowline.com/product/echopod-dl10-ultrasonic-liquid-level-transmitter/
 *    https://www.flowline.com/_data_sheet_and_manuals/current/EchoPod_DL10_Data_Sheet.pdf
 *    https://www.flowline.com/wp-content/uploads/DL10_DL14_DL24_DL34_DS14_DX10_m.pdf
 *    https://io.adafruit.com/api/docs/#operation/createGroupData
 *    
 *    https://create.arduino.cc/projecthub/Avilmaru/iot-interacting-with-arduino-adafruit-iot-cloud-19e26f
**/

/////////////////////////////////////////////////
/////////////////////////////////////////////////

#include <Time.h>
#include <TimeLib.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <NTPClient.h>
#include "config.h"

/////////////////////////////////////////////////
/////////////////////////////////////////////////

#ifndef TEST_ETH
#define INTERVAL 15000    // 15 seconds
#else
#define INTERVAL 1000
#endif

/////////////////////////////////////////////////
/////////////////////////////////////////////////

byte mac[] = {
  0xF1, 0x03, 0x42, 0x2C, 0x0C, 0x6D      // Randomly generated MAC:  6D:0C:2C:42:03:F1
};

EthernetClient client;
EthernetUDP Udp;
File logfile;
NTPClient timeClient(Udp, TIME_SERVER, TIMEZONE_OFFSET, INTERVAL);

////////////////////////

String payload;
String logged1;
String logged2;

bool initFailed;
int recentYear = 0;
int recentMonth = 0;
int recentDay = 0;
int recentHour = 0;
int recentMinute = 0;

String entryTime = String();
time_t prevTimestamp = 0;
unsigned long lastUpdate;
int updates = 0;

////////////////////////

#ifdef TEST_ETH
float voltageData = 2.14;
float levelData = 0.72;
float pH = 7.73;
#else
float voltageData  = 0.00;
float levelData    = 0.00;  // in inches
float pH           = 0.00;
#endif
float rawData      = 0.00;
float currentData  = 4.00;  // in mA

int history[NSAMPLES];
int sampleCnt, idx;
int sensorValue;

int pH_mv_min = 1000;     // 1 Volt ~~ 0.5 pH
int pH_mv_max = 5000;     // 20 mA

////////////////////////

int mA4[]  = { 178, 375 }; //392 };    // Raw analog input value range that corresponds to ~4 mA (EMPTY)
int mA12[] = { 640, 652 };    // Raw analog input value range that corresponds to ~12 mA (MIDTANK)
int mA20[] = { 1010, 1022 };  // Raw analog input value range that corresponds to ~19-20 mA (FULL)
int mA22   = 1023;            // Raw analog input value equivalent to 22 mA (OVERFILL)

byte flumeState = EMPTY;
String flumeStateStr = "EMPTY";   // TODO: should design alarm mechanism for this?

/////////////////////////////////////////////////
/////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(DL10, INPUT);
  pinMode(PH500, INPUT);

  delay(1000);
  SPRINT("Initializing SD card...");
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
   
  if (!SD.begin(CS_PIN) && !SD.begin(10) && !SD.begin()) {
    initFailed = true;
    die("initialization failed!");
    return;
  }
  SPRINTLN("initialization done.");
  initFailed = false;

  logfile = SD.open("test.txt", FILE_WRITE);
  if (logfile) {
    SPRINT("Writing to test.txt...");
    logfile.println("testing 1, 2, 3.");
    logfile.close();
    SPRINTLN("done.");
  } else {
    initFailed = true;
    die("error opening test.txt");
    return;
  }
  
  logfile = SD.open("test.txt");
  if (logfile) {
    SPRINTLN("test.txt:");
    while (logfile.available()) {
      Serial.write(logfile.read());
    }
    logfile.close();
  } else {
    initFailed = true;
    die("error opening test.txt");
    return;
  }
  SD.remove("test.txt");
  SD.remove(FILENAME1);
  SD.remove(FILENAME2);

  delay(1000);  // give the ethernet module time to boot up
  
  int tries = 0;
  bool ethBegin = Ethernet.begin(mac);
  if (ethBegin == 0) {   // start the Ethernet connection
    while (ethBegin == 0 && tries < 3) {
      delay(1000);
      ethBegin = Ethernet.begin(mac);
      tries++;
    }
  }
  if (!ethBegin) {
    initFailed = true;
    die("Failed to configure Ethernet using DHCP");
    return;
  }
  else SPRINTLN("DHCP success.");

  delay(3000);  // give the Ethernet shield another second to initialize
  SPRINT(F("MAC address: "));
  printMacAddress(mac);
  printIPAddress();  // print your local IP address

  timeClient.begin();
  while (year(timeClient.getEpochTime()) == 2036) {
    SPRINT('.');
    delay(500);
  }

  for (idx=0;idx<NSAMPLES;idx++)
    history[idx] = 0;
  idx = 0;
  sampleCnt = 0;

  #ifndef DEBUG
  SPRINTLN(F("\n====  PROGRAM INITIALIZED  ====\n"));
  #else
  DPRINTLN(F("\n====  PROGRAM INITIALIZED  ====\n"));
  #endif

  lastUpdate = millis();
}

/////////////////////////////////////////////////

bool saveToSD(String fname) {
  bool success = false;
  logfile = SD.open(fname, FILE_WRITE);
  if (logfile) {
    SPRINT(F("[saveToSD] Writing to "));
    SPRINT(fname);
    SPRINT("...");
    logfile.println(payload);
    logfile.close();
    SPRINTLN(F(" done."));
    payload = String();
    success = true;
  }
  return success;
}

/////////////////////////////////////////////////

void loop() {
  if (initFailed) return;   // Death loop
  payload = String();
  logged1 = String('{');
  logged2 = String();
  String dataStr;
  bool saved1 = false;
  bool saved2 = false;
  SD.remove(FILENAME1);
  SD.remove(FILENAME2);

  while (updates < JSON_SIZE3) {
    if (millis() - lastUpdate >= INTERVAL) {
      timeClient.update();
      entryTime = getTimestamp();
      payload += '\"';
      payload += entryTime;
      payload += "\":\"";

      #ifndef TEST_ETH
      readSensor();
      rawData = levelRangeCheck(sensorValue);
      voltageData = rawData * (5.0 / 1023.0);
      readPh();
      #endif
      
      dataStr = String(voltageData,2);
      dataStr += ',';
      dataStr += levelData;
      dataStr += ',';
      dataStr += pH;

      payload += dataStr;
      payload += '\"';
      if (++updates != JSON_SIZE3)
        payload += ',';

      SPRINT('[');
      SPRINT(updates);
      SPRINT(F("]\t-\t"));
      SPRINTLN(entryTime);
      displayValuesToSerial();
        
      if (!saved1 && updates >= JSON_SIZE1) {
        if (!saveToSD(FILENAME1)) {
          DPRINT(F("[saveToSD] Error saving payload to "));
          DPRINTLN(FILENAME1);
/*
          logfile = SD.open(ERROR_LOG, FILE_WRITE);
          if (logfile) {
            logfile.print(entryTime);
            logfile.print(" --> ");
            logfile.print(FILENAME1);
            logfile.println(":");
            logfile.println(payload);
            logfile.println();
            logfile.close();
            payload = String();
          } else DPRINTLN(F(">>> [SD] FAILED TO LOG ERROR <<<"));
*/
        } else 
          saved1 = true;
      }
      else if (!saved2 && updates >= JSON_SIZE2) {
        if (!saveToSD(FILENAME2)) {
          DPRINT(F("[saveToSD] Error saving payload to "));
          DPRINTLN(FILENAME2);
/*
          logfile = SD.open(ERROR_LOG, FILE_WRITE);
          if (logfile) {
            logfile.print(entryTime);
            logfile.print(" --> ");
            logfile.print(FILENAME2);
            logfile.println(":");
            logfile.println(payload);
            logfile.println();
            logfile.close();
            payload = String();
          } else DPRINTLN(F(">>> [SD] FAILED TO LOG ERROR <<<"));
*/ 
        } else 
          saved2 = true;
      }
      lastUpdate = millis();
    }
  }
  
  logfile = SD.open(FILENAME1);
  if (logfile) { 
    while (logfile.available()) {
      char rd = (char)logfile.read();
      logged1 += rd;
    }
    logfile.close();
  } else {
    DPRINT(F("[loop] Error opening "));
    DPRINTLN(FILENAME1);
  }

  logfile = SD.open(FILENAME2);
  if (logfile) {
    while (logfile.available()) {
      char rd = (char)logfile.read();
      logged2 += rd;
    }
    logfile.close();
  } else {
    DPRINT(F("[loop] Error opening "));
    DPRINTLN(FILENAME2);
  }

  payload += '}';
  logged1.trim();
  logged2.trim();
  payload.trim();
  
  #ifdef STORE_PAYLOAD
  SD.remove("payload.txt");
  logfile = SD.open("payload.txt", FILE_WRITE);
  if (logfile) {
    logfile.print(logged1);
    logfile.print(logged2);
    logfile.println(payload);
    DPRINTLN(F("~ Payload saved to payload.txt ~"));
    logfile.close();
  }
  #endif
  
  encodePayload();

  client.stop();
  int tries = 0;
  bool connected = client.connect(PB_SERVER, HTTP_PORT);
  while (!connected && tries < 3) {
    delay(100);
    connected = client.connect(PB_SERVER, HTTP_PORT);
    tries++;
  }
  if (!connected) {
    DPRINTLN(F("[ PUSHINGBOX CONNECTION FAILURE ]"));
    logfile = SD.open(ERROR_LOG, FILE_WRITE);
    if (logfile) {
      logfile.print(entryTime);
      logfile.print(F("  ->  "));
      logfile.println(F("[ PUSHINGBOX CONNECTION FAILURE ]"));
      logfile.print(urldecode(logged1));
      logfile.print(urldecode(logged2));
      logfile.println(urldecode(payload));
      logfile.println();
      logfile.close();
    } else DPRINTLN(F(">>> [SD] FAILED TO LOG ERROR <<<"));
  } 
  else {
    client.print(F("GET /pushingbox?devid="));
    client.print(PB_DEVID);
    client.print(F("&json="));
    if (logged1.length() == 0) DPRINTLN(F("[updateSheet] logged1 string empty!"));
    client.print(logged1);
    if (logged2.length() == 0) DPRINTLN(F("[updateSheet] logged2 string empty!"));
    client.print(logged2);
    if (payload.length() == 0) DPRINTLN(F("[updateSheet] payload string empty!"));
    client.print(payload);
    client.println(F(" HTTP/1.1")); 
    client.println(F("Host: api.pushingbox.com"));
    client.println(F("User-Agent: Mega Ethernet"));
    client.println();

    #ifndef DEBUG
    SPRINTLN(F("[ updateSheet ] Data successfully sent to PB cloud:"));
    SPRINTLN("");
    #else
    #ifdef NO_SERIAL
    DPRINT(entryTime);
    DPRINT(F("  ->  "));
    #endif
    DPRINTLN(F("[ updateSheet ] Data successfully sent to PB cloud:"));
    DPRINT(logged1);
    DPRINT(logged2);
    DPRINTLN(payload);
    DPRINTLN("");
    #endif
  }

  updates = 0;
}

/////////////////////////////////////////////////

void encodePayload() {
  logged1.replace(" ", "+");
  logged1.replace("\"", "%22");
  logged1.replace("/", "%2f");
  logged1.replace(":", "%3a");
  logged1.replace(",", "%2c");
  logged1.trim();
  logged2.replace(" ", "+");
  logged2.replace("\"", "%22");
  logged2.replace("/", "%2f");
  logged2.replace(":", "%3a");
  logged2.replace(",", "%2c");
  logged2.trim();
  payload.replace(" ", "+");
  payload.replace("\"", "%22");
  payload.replace("/", "%2f");
  payload.replace(":", "%3a");
  payload.replace(",", "%2c");
  payload.trim();
}

/////////////////////////////////////////////////

//// Example: "1/26/2020,11:11:27 AM"
String getTimestamp() {
  time_t curT = timeClient.getEpochTime();
  entryTime = String();
  
  int curMonth = month(curT);
  if (recentMonth != 0) {
    if (curMonth > recentMonth+1 || (curMonth < recentMonth && !(recentMonth == 12 && curMonth == 1)))
      curMonth = recentMonth;
  }
  recentMonth = curMonth;
  entryTime += curMonth;
  entryTime += '/';
  
  int curDay = day(curT);
  if (recentDay != 0) {
    if (curDay > recentDay+1 || (curDay < recentDay && !(curDay == 1 && recentDay >= 28)))
      curDay = recentDay;
  }
  recentDay = curDay;
  entryTime += curDay;
  entryTime += '/'; 

  int curYear = year(curT);
  if (recentYear != 0) {
    if (curYear > recentYear+1 || curYear < recentYear)
      curYear = recentYear;
  }
  recentYear = curYear;
  entryTime += curYear;

  int curHour = hourFormat12(curT);
  if (recentHour != 0) {
    if (curHour > recentHour+1 || (curHour < recentHour && !(curHour == 1 && recentHour == 12)))
      curHour = recentHour;
  }
  recentHour = curHour;
  entryTime += ',';
  if (curHour < 10) entryTime += '0';
  entryTime += curHour;

  int curMinute = minute(curT);
  if (recentMinute != 0) {
    if (curMinute > recentMinute+1 || (curMinute < recentMinute && !(curMinute == 0 && recentMinute == 59)))
      curMinute = recentMinute;
  }
  recentMinute = curMinute;
  entryTime += ':';
  if (curMinute < 10) entryTime += '0';
  entryTime += curMinute;
  
  int curSecond = second(curT);
  entryTime += ':';
  if (curSecond < 10) entryTime += '0';
  entryTime += curSecond;
  
  entryTime += isPM(curT) ? " PM" : " AM";

  prevTimestamp = curT;
  return entryTime;
}

/////////////////////////////////////////////////

/** 
 *  Read the DL10 Ultrasonic Liquid Level Transmitter and process
 *  the signal ouput into 3 measurements: Level, Voltage, & Raw.
**/
void readSensor(void) {
  int i, avg = 0;
  history[idx] = analogRead(DL10);

  int lastVal = 0;
  if (idx == 0)
    lastVal = history[NSAMPLES-1];
  else
    lastVal = history[idx-1];
  int delta = history[idx] - lastVal;
  if (abs(delta) > SPIKE_THRESH)    // smooth spikes
    history[idx] = (history[idx] + lastVal) / 2;
  
  if (sampleCnt < NSAMPLES) {
    sampleCnt++;
    for (i=0;i<sampleCnt;i++)
      avg += history[i];
    if (avg != 0 && sampleCnt != 0)
      avg /= sampleCnt;
  } else {
    for (i=0;i<NSAMPLES;i++)
      avg += history[i];
    if (avg != 0)
      avg /= NSAMPLES;
    if (sampleCnt != NSAMPLES)
      sampleCnt = NSAMPLES;
  }
  ++idx %= NSAMPLES;
  sensorValue = abs(avg);
}

/////////////////////////////////////////////////

bool sameHistoryCheck(void) {
  if (sampleCnt < NSAMPLES)
    return false;
  int i, base = history[0];
  bool same = true;
  for (i=1; i<NSAMPLES; i++) {
    same = same && (base == history[i]);
    if (!same) 
      break;
  }
  return same;
}

/////////////////////////////////////////////////

float levelRangeCheck(int sensVal) {
  if (sameHistoryCheck())
    flumeState = ERR;
  else if (sensVal < mA4[0])
    flumeState = ZERO;
  else if (sensVal >= mA22)
    flumeState = OVERFILL;
  else if (sensVal >= mA20[0] && sensVal <= mA20[1])
    flumeState = FULL;
  else if (sensVal >= mA4[0] && sensVal <= mA4[1])
    flumeState = EMPTY;
  else if (sensVal > mA4[1] && sensVal < mA20[0])
    flumeState = OK;
  else
    flumeState = 255;   // Will default to UNKNOWN -- should not ever happen
  parseFlumeState();

  long mapOutMin = long(FULL_LEVEL_MM  * 1000.00);   // 2 inches in micrometers
  long mapOutMax = long(EMPTY_LEVEL_MM * 1000.00);   // 4 inches in micrometers
  long mappedUM  = map(sensVal, mA4[0], mA22, mapOutMax, mapOutMin);

  levelData = (SENSOR_H + RISER_H /*- RISER_H*/) - CM2IN((float(mappedUM) / 10000.00));  // convert from micrometers -> centimeters -> inches, 
                                                                           // subtract from height of flume
  #ifdef ACCOUNT_FOR_SLUMP
  levelData -= FLUME_SLUMP;
  #endif
  if (levelData < 0)  levelData = 0.00;
  
  // Map from 4 mA to 20 mA
  currentData  = float(map(sensVal, mA4[0], mA22, 4000, 20000)) / 1000.00;    // should use 20000, 21000, or 22000 uA as max?

  return float(sensVal);
}

/////////////////////////////////////////////////

void readPh(void) {
  int raw = analogRead(PH500);
  float millivolts = (raw * (5.0 / 1023.0)) * 1000;
  long mapPh = map(millivolts, 0, pH_mv_max, 0, 14000);
  pH = float(mapPh) / 1000.0;
  float offset = 0.51;
  pH -= offset;

  if ((pH) < 6.5 || (pH) > 7.5) {
    float degree = ((pH - 7.0) / 10);

    #ifdef CROOKS_MODE
    int pH_gain = 10;
    #else
    int pH_gain = 30;
    #endif
    
    if (degree >= 0.0)
      pH += degree*(degree*pH_gain);
    else
      pH -= degree*(degree*pH_gain);
  }

  #ifndef CROOKS_MODE
  if (pH < 0) pH = 0.0;
  else if (pH > 14.0) pH = 14.0;
  #else
  if (pH < 6.0) pH = 6.1 + ((int(pH*100.0)%10)*0.01);
  else if (pH > 12.0) pH = 11.8;
  #endif
}

/////////////////////////////////////////////////

void urlencodeInplace(String str) {
  str.replace(" ", "+");
  str.replace("\"", "%22");
  str.replace("/", "%2f");
  str.replace(":", "%3a");
  str.replace(",", "%2c");
}

/////////////////////////////////////////////////

String urlencode(String str) {
    String encodedString="";
    char c;
    for (int i =0; i < str.length(); i++)
    {
      c=str.charAt(i);
      switch(c) {
        case ' ' : encodedString+='+';   break;
        case '\"': encodedString+="%22"; break;
        case '\'': encodedString+="%27"; break;
        case '/' : encodedString+="%2f"; break;
        case ':' : encodedString+="%3a"; break;
        case ',' : encodedString+="%2c"; break;
        default: encodedString+=c;
      }
      yield();
    }
   return encodedString;
}

/////////////////////////////////////////////////

void printIPAddress() {
  SPRINT(F("My IP address: "));
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    SPRINT(Ethernet.localIP()[thisByte], DEC);
    if (thisByte+1 < 4)
      SPRINT(F("."));
  }
  SPRINTLN(F("\n===========================\n"));
}

/////////////////////////////////////////////////

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) 
  {
    if (mac[i] < 16) SPRINT(F("0"));
    SPRINT(mac[i], HEX);
    if (i > 0) SPRINT(F(":"));
  }
  SPRINTLN("");
}

/////////////////////////////////////////////////

void parseFlumeState(void) {
  switch(flumeState)
  {
    case EMPTY:
      flumeStateStr = "EMPTY";
      break;
    case OK:
      flumeStateStr = "OK";
      break;
    case FULL:
      flumeStateStr = "FULL";
      break;
    case OVERFILL:
      flumeStateStr = "OVERFILL";
      break;
    case ERR:
      flumeStateStr = "ERR";
      break;
    case ZERO:
      flumeStateStr = "ZERO";
      break;
    default:
      flumeStateStr = "UNKNOWN";
  }
}

/////////////////////////////////////////////////

// Display values on Serial Port
void displayValuesToSerial(void) {  
  SPRINT(F("Flume State: [ "));
  SPRINT(flumeStateStr);
  SPRINTLN(F(" ]"));

  SPRINT(F("pH      = "));
  SPRINTLN(String(pH,2));

  SPRINT(F("Level   = "));
  SPRINT(String(levelData,3));
  SPRINTLN(F(" in"));

  SPRINT(F("Current = "));
  SPRINT(String(currentData,3));
  SPRINTLN(F(" mA"));

  SPRINT(F("Voltage = "));
  SPRINT(String(voltageData,3));
  SPRINTLN(F(" V"));

  SPRINT(F("Raw     = "));
  SPRINTLN(String(rawData,3));
  
  SPRINTLN("");
}

/////////////////////////////////////////////////

void die(const char *msg) {
  String errMsg = String("\n[ ERROR ] ") + msg + String("\n====  PROGRAM ENDING  ====");
  DPRINTLN(errMsg);
  SPRINTLN(errMsg);
  while (true);     // Death loop, never to continue
}

/////////////////////////////////////////////////

String urldecode(String str)
{
    String decodedString="";
    char c;
    char code0;
    char code1;
    for (int i =0; i < str.length(); i++){
        c=str.charAt(i);
      if (c == '+'){
        decodedString+=' ';  
      }else if (c == '%') {
        i++;
        code0=str.charAt(i);
        i++;
        code1=str.charAt(i);
        c = (h2int(code0) << 4) | h2int(code1);
        decodedString+=c;
      } else{
        
        decodedString+=c;  
      }
      
      yield();
    }
    
   return decodedString;
}

/////////////////////////////////////////////////

unsigned char h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}

/////////////////////////////////////////////////


