#include <SPI.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
#include "Adafruit_STMPE610.h"

#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>

#ifdef ESP8266
#define STMPE_CS 16
#define TFT_CS   0
#define TFT_DC   15
#define SD_CS    2
#endif
#ifdef ESP32
#define STMPE_CS 32
#define TFT_CS   15
#define TFT_DC   33
#define SD_CS    14
#endif
#ifdef __AVR_ATmega32U4__
#define STMPE_CS 6
#define TFT_CS   9
#define TFT_DC   10
#define SD_CS    5
#endif
#ifdef ARDUINO_SAMD_FEATHER_M0
#define STMPE_CS 6
#define TFT_CS   9
#define TFT_DC   10
#define SD_CS    5
#endif
#ifdef TEENSYDUINO
#define TFT_DC   10
#define TFT_CS   4
#define STMPE_CS 3
#define SD_CS    8
#endif
#ifdef ARDUINO_STM32_FEATHER
#define TFT_DC   PB4
#define TFT_CS   PA15
#define STMPE_CS PC7
#define SD_CS    PC5
#endif
#ifdef ARDUINO_FEATHER52
#define STMPE_CS 30
#define TFT_CS   13
#define TFT_DC   11
#define SD_CS    27
#endif

#define TFT_RST -1

// Use hardware SPI and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF

#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

double previous_speedval = 0;
int current_speedval = 1;
double previous_distance = 0;
int current_distance = 1;
boolean in_screen = false;
boolean lights = false;

int current_screen = 0; 
// Screens : 
// 0 is riding screen (odometer, speed, voltage)
// 1 is main menu with nav buttons
//      lights on/off
//      Trip Stats button
//      Board Stats Button
//      Board Settings Button
//      Back to Ride Button
// 2 is Trip Stats
//    Option to clear current trip
// 3 is Board stats
// 4 is board settings

// While in any screen but main menu, clicking in middle of screen will take you to main menu.


void setup() {
  // put your setup code here, to run once:
  tft.begin(HX8357D);
  
  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(HX8357_RDPOWMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDCOLMOD);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDDIM);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDDSDR);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);

  Serial.println(F("Benchmark                Time (microseconds)"));

  tft.setRotation(1);
  ts.begin();

}

void loop() 
{
  // put your main code here, to run repeatedly:

  if (current_screen == 0)
  {
    ride_screen();
  }
  
  if (current_screen == 1)
  {
    mainmenu_screen();
  }
  
  if (current_screen == 2)
  {
    tripstats_screen();
  }
  
  if (current_screen == 3)
  {
    boardstats_screen();
  }
  
  if (current_screen == 4)
  {
    boardsettings_screen();
  }
 
}

void ride_screen()
{
  //Ride screen routines
  if (!not ts.bufferEmpty())
  {
    if (in_screen == false)  // this is used to draw screen labels for first time
    {
      in_screen = true;     // this is used to prevent the whole screen being redrawn every loop
      tft.fillScreen(HX8357_BLACK);  // clear the screen
      tft.setCursor(30, 0);
      tft.setTextColor(HX8357_WHITE);
      tft.setTextSize(4);
      tft.println("Speed");

      tft.setCursor(270, 0);
      tft.setTextColor(HX8357_WHITE);
      tft.setTextSize(4);
      tft.println("Distance");

      tft.drawLine(225, 0, 225, 700, WHITE);
  }
  speedcheck(); // check and update speed on screen
  distancecheck();  // check and update distance on screen
  
  
    
  }
  else
  {
    TS_Point p=ts.getPoint();
    while(ts.touched())
      p= ts.getPoint();
    current_screen = 1;
    in_screen = false;
  }
  
}

void mainmenu_screen()
{
  //Main menu routines
  if (!not ts.bufferEmpty())
  {
    if (in_screen == false)  // this is used to draw screen labels for first time
    {
      in_screen = true;
      tft.fillScreen(HX8357_BLACK); // clear the screen

      tft.setCursor(120, 0);
      tft.setTextColor(HX8357_WHITE);
      tft.setTextSize(4);
      tft.println("Main Menu");  // title of screen


      tft.fillRect(5,60,150,100,RED); //draw red button
      tft.drawRect(5,60,150,100,WHITE); //draw white outline for button
      tft.setCursor(40, 100);
      tft.setTextColor(BLACK);
      tft.setTextSize(3);
      tft.println("RIDE");

      tft.fillRect(165,60,150,100,GREEN); //draw red button
      tft.drawRect(165,60,150,100,WHITE); //draw white outline for button
      tft.setCursor(190, 80);
      tft.println("LIGHTS");
      tft.setCursor(190, 110);
      tft.println("OFF");

      tft.fillRect(325,60,150,100,BLUE); //draw red button
      tft.drawRect(325,60,150,100,WHITE); //draw white outline for button
      tft.setCursor(365, 80);
      tft.println("TRIP");
      tft.setCursor(360, 110);
      tft.println("STATS");

      tft.fillRect(5,170,150,100,YELLOW); //draw red button
      tft.drawRect(5,170,150,100,WHITE); //draw white outline for button
      tft.setCursor(10, 210);
      tft.setTextSize(3);
      tft.println("SETTINGS");
      
      tft.fillRect(165,170,150,100,CYAN); //draw red button
      tft.drawRect(165,170,150,100,WHITE); //draw white outline for button
      tft.setCursor(180, 210);
      tft.println("VOLTAGE");

      tft.fillRect(325,170,150,100,MAGENTA); //draw red button
      tft.drawRect(325,170,150,100,WHITE); //draw white outline for button
      tft.setCursor(360, 190);
      tft.println("BOARD");
      tft.setCursor(360, 220);
      tft.println("STATS");
    }
  }
  else
  {
    TS_Point p = ts.getPoint();
    
    while(ts.touched())
      p= ts.getPoint();
      while ( ! ts.bufferEmpty() )
      { 
        p = ts.getPoint();
      }
   // in_screen = false;
    tft.setTextColor(WHITE);
    tft.setCursor(0, 0);
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

    int y = p.x;
    int x = p.y;


    if( y > 270 && y < 400)  // ride, lights, or drive stats
    {
      if( x > 15 && x < 90)  // ride button!
      {
        current_screen = 0;
        in_screen = false;
      }
      if( x > 105 && x < 195)  // lights button!
      {
        if(lights)
        {
          lights = false;
          tft.fillRect(190, 110, 55, 35, GREEN);
          tft.setTextColor(BLACK);
          tft.setCursor(190, 110);
          tft.println("OFF");
        }
        else
        {
          lights = true;
          tft.fillRect(190, 110, 55, 35, GREEN);
          tft.setTextColor(BLACK);
          tft.setCursor(190, 110);
          tft.println("ON");
        }
        
        
      }
      if( x > 205 && x < 300)  // trip stats button!
      {
        current_screen = 2;
        in_screen=false;
      }
    }
    tft.fillRect(0, 0, 50, 50, BLACK);
    tft.setCursor(0, 0);
    tft.println(x);
      
  }
   
}

void tripstats_screen()
{
  //Trip Stats routines

  if (!not ts.bufferEmpty())
  {
    if (in_screen == false)  // this is used to draw screen labels for first time
    {
        in_screen = true;
        tft.fillScreen(HX8357_BLACK);  // clear the screen
        tft.setCursor(120, 0);
        tft.setTextColor(HX8357_WHITE);
        tft.setTextSize(4);
        tft.println("Trip Stats");
    }
  }
  else
  {
    TS_Point p=ts.getPoint();
    while(ts.touched())
      p= ts.getPoint();
    current_screen = 1;
    in_screen = false;
  }
}

void boardstats_screen()
{
  //Board Stats routines
   tft.fillScreen(HX8357_BLACK);  // clear the screen
}

void boardsettings_screen()
{
  //Board Settings routines
   tft.fillScreen(HX8357_BLACK);  // clear the screen
}

void speedcheck()
{
   if (round(previous_speedval) != current_speedval)
  {
    current_speedval = round(previous_speedval);

    tft.fillRect(40, 140, 150, 150, BLACK);

    tft.setCursor(40, 140);
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(8);
    tft.println(current_speedval);
  }
  
  previous_speedval = previous_speedval + 1;
  delay(500);
  

  if (current_speedval >= 45)
  {
    previous_speedval = 1;
    current_speedval = 0;
  }
}

void distancecheck()
{
  if (round(previous_distance) != current_distance)
  {
    current_distance = round(previous_distance);

    tft.fillRect(300, 140, 150, 150, BLACK);

    tft.setCursor(300, 140);
    tft.println(current_distance);


  }
  previous_distance = previous_distance + .2;
  delay(500);
}

void voltagecheck()
{
  
}

