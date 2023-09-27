// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>
#include "ClosedCube_HDC1080.h"
#include <Button.h>
#include <FastLED.h>

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
   #define Serial SerialUSB
#endif

#define NUM_LEDS 5
#define DATA_PIN 3
CRGB leds[NUM_LEDS];

RTC_DS1307 rtc;
const int chipSelect = 10;
File dataFile;
Button button(2); // Connect your button between pin 2 and GND


ClosedCube_HDC1080 hdc1080; // Creation of the object

int location = 0;

int state = 0;
long time = 0;


void setup () {
  Serial.begin(9600);
  // Open serial communications and wait for port to open:
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2023, 9, 27, 12, 12, 0));
  }





  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(SS, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1) ;
  }
  Serial.println("card initialized.");
  
  // Open up the file we're going to log to!
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (! dataFile) {
    Serial.println("error opening datalog.txt");
    // Wait forever since we cant write data
    while (1) ;
  }
  hdc1080.begin(0x40); // initialization of the sensor

  button.begin();

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  for (int i = 0; i < 5; i++){
    leds[i] = CRGB(255,0,0);
  }
  FastLED.show();
}

void loop () {

  switch (state){
    case 0:
      if (button.pressed()) {
        state = 1;
        Serial.println("state = 1");
        time = millis();
        FastLED.clear();
        leds[location] = CRGB(0,0,255);
        FastLED.show();
      }
      
      break;
    
    case 1:
      
        if (button.pressed()) {
          location ++;
          if (location == 5) location = 0;
          // set location light and show leds
          Serial.println(location);
          for (int i = 0; i < 5; i++){
            if (i == location) leds[i] = CRGB(0,0,255);
            else leds[i] = CRGB(0,0,0);
          }
          FastLED.show();

        } 
      if (millis() - time > 15000) {
        state = 2;
        for (int i = 0; i < 5; i++){
            if (i != location) leds[i] = CRGB(0,255,0);
          }
          FastLED.show();
          time = millis();
      }
      break;
    
    case 2:
      if (millis() > time + 10000) {
        state = 3;
        FastLED.clear();
        FastLED.show();
      }
      break;
    
    case 3:

      
  
/*
  if (millis() < 5000){
    if (button.pressed()) {
      location ++;
      if (location == 5) location = 0;
      // set location light and show leds
      Serial.println(location);
    }

  }
  else {
  */

  

  DateTime now = rtc.now();
  String date = String(now.year())+"/"+String(now.month())+"/"+String(now.day());
  String time = String(now.hour())+":"+String(now.minute())+":"+String(now.second());
  String temp = String(hdc1080.readTemperature());
  String humidity = String(hdc1080.readHumidity());
  String dataString = "|yyyy/mm/dd|hh:mm:ss|temp|humidity|location|";
  dataString += ",";
  dataString += date;
  dataString += ",";
  dataString += time;
  dataString += ",";
  dataString += temp;
  dataString += ",";
  dataString += humidity;
  dataString += ",";
  dataString += String(location);

  dataFile.println(dataString);

  // print to the serial port too:
  Serial.println(dataString);
  
  // The following line will 'save' the file to the SD card after every
  // line of data - this will use more power and slow down how much data
  // you can read but it's safer! 
  // If you want to speed up the system, remove the call to flush() and it
  // will save the file only every 512 bytes - every time a sector on the 
  // SD card is filled with data.
  dataFile.flush();
  
  // Take 1 measurement every 500 milliseconds
  delay(1000);
  break;
  }
}
