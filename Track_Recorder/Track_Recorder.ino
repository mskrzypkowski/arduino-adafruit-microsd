#include <SPI.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <SdFat.h>
#include <avr/sleep.h>
SdFat SD;

SoftwareSerial mySerial(8, 7);
Adafruit_GPS GPS(&mySerial);

#define GPSECHO  true
#define LOG_FIXONLY false 
#define chipSelect 10
#define ledPin 13
#define startPin 2
#define endPin 6 

boolean usingInterrupt = false;
boolean doLog = false;

File logfile;

void error(uint8_t errno) {
  while(1) {
    uint8_t i;     
    for (i=0; i<errno; i++) {
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
    }
      delay(100);
    for (i=errno; i<10; i++) {
      delay(200);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\r\nWelcome to Track Recorder");
  pinMode(ledPin, OUTPUT);
  pinMode(startPin,INPUT);
  pinMode(endPin,INPUT);
  pinMode(10, OUTPUT);

    if (!SD.begin(chipSelect)) {  
    Serial.println("Card init. failed!");
    error(2);
  }

}

void loop() {

  if(digitalRead(startPin) == HIGH){

//    if (!GPS.fix){
//          Serial.println("Not enough satellites detected. Please wait and try again later. Dioda?");
//          delay(1000);
//        }

//    if (GPS.fix){   
      uint8_t run1time = 0;    
      if(run1time < 1){
        
        char filename[15];
        strcpy(filename, "GPSLOG00.gpx");
        for (uint8_t i = 0; i < 100; i++) {
          filename[6] = '0' + i/10;
          filename[7] = '0' + i%10;
          // create if does not exist, do not open existing, write, sync after write
          if (! SD.exists(filename)) {
            break;
          }
        }
      
        logfile = SD.open(filename, FILE_WRITE);
        if(!logfile) {
          Serial.print("Couldnt create "); 
          Serial.println(filename);
          error(3);
        }
        Serial.print("Writing to "); 
        Serial.println(filename);
      
        logfile.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        logfile.println("<gpx version=\"1.0\">");
        logfile.println("<name>Track recorder</name>");
        logfile.println("<trk><name>Track</name><number>1</number><trkseg>");
        
      
        // connect to the GPS at the desired rate
        GPS.begin(9600);
        GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
        GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 100 millihertz (once every 10 seconds), 1Hz or 5Hz update rate
        GPS.sendCommand(PGCMD_NOANTENNA);
        useInterrupt(true);
      
        Serial.println("Ready!");
      
        Serial.println("Logging started.");
        doLog = true; 
        delay(1000);
  
      run1time++;
//      }
    }
  }


  if (GPS.newNMEAreceived()) {
    char *stringptr = GPS.lastNMEA();
    if (!GPS.parse(stringptr))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
    
    if (LOG_FIXONLY && !GPS.fix) {
      Serial.print("No Fix");
      return;
    }

  if(doLog){
//
//     if (!GPS.fix){
//          Serial.println("Logging paused - searching for satellites");
//        }
//    
//    if (GPS.fix) {
        logfile.print("<trkpt lat=\"");
        logfile.print(GPS.latitudeDegrees, 8);
        logfile.print("\" lon=\"");
        logfile.print(GPS.longitudeDegrees, 8);
        logfile.print("\"><ele>");
        logfile.print(GPS.altitude);
        logfile.print("</ele>");
        logfile.print("<speed>");
        logfile.print(GPS.speed);
        logfile.print("</speed><time>");
        logfile.print("20");
        logfile.print(GPS.year);
        logfile.print("-");
        logfile.print(GPS.month);
        logfile.print("-");
        logfile.print(GPS.day);
        logfile.print("T");
        logfile.print(GPS.hour);
        logfile.print(":");
        logfile.print(GPS.minute);
        logfile.print(":");
        logfile.print(GPS.seconds);
        logfile.print("Z</time></trkpt>");
        logfile.println();
        logfile.flush();
        Serial.println("Logged");
      } 
// }    
  }

   if(digitalRead(endPin) == HIGH && doLog == true){
        logfile.println("</trkseg></trk>");
        logfile.println("</gpx>");
        logfile.flush();
        doLog = false;
        Serial.println("End of logging.");
        delay(500);
   }
}    // end of loop() function


// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } 
  else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}



/* End code */

