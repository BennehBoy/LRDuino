// LRDuino by Ben Anderson
// Version 0.94  (STM32 Only)

#include <Adafruit_SSD1306.h>
#include <Adafruit_MAX31856.h>
#include "LRDuinoGFX.h"
#include <Fonts/FreeSansBoldOblique12pt7b.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_HMC5883_U.h>

// Following pinout details are for Maple Mini
#define OLED_RESET  12  //LED
#define OLED_CS     13
#define OLED_DC     14  //MISO DC
// 15 is I2C SDA (for ADXL)
// 16 is I2C SCL
#define OLED_CLK    17  //D0
#define OLED_MOSI   18  //D1
#define OLED_CS_2   19
#define OLED_CS_3   20
//#define OLED_CS_4   22
//#define OLED_CS_5   25
//#define OLED_CS_6   26

#define MAX_CS      28 //Multiple software SPI because adafruit SSD1306 & MAX31856 libraries won't play nicely on the same bus
#define MAX_DC      29 //MISO DI
#define MAX_CLK     30 //CLK
#define MAX_MOSI    31 //DO
#define ROTBUT      21 // our input button
#define DIVISOR     4095
#define A0          11 // boost
#define A1          10 // tbox temp
#define A2          9  // Oil pressure
#define A3          8  // Oil Temp
#define A4          7  // Coolane Level
#define PIEZO       3  // No more pins !!!

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

Adafruit_MAX31856 max = Adafruit_MAX31856(MAX_CS, MAX_DC, MAX_MOSI, MAX_CLK); // seperate buses
//Adafruit_MAX31856 max = Adafruit_MAX31856(MAX_CS, OLED_DC, OLED_MOSI, OLED_CLK); // shared bus

Adafruit_SSD1306 display1(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
Adafruit_SSD1306 display2(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS_2);
Adafruit_SSD1306 display3(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS_3);
//Adafruit_SSD1306 display4(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS_4);
//Adafruit_SSD1306 display5(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS_5);
//Adafruit_SSD1306 display6(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS_6);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// This is all the parameters and variables for our sensors

typedef struct
{
 bool senseactive;   
 bool warnstatus;    
 uint8_t sensefault;        
 const unsigned char* senseglyphs;
 int sensevals;
 const uint8_t senseunits;
 const int sensemaxvals; 
 const int8_t senseminvals;
 int sensepeakvals; 
 const int sensewarnhivals;
 const int sensewarnlowvals;
} SingleSensor;

SingleSensor Sensors[20] = {
// active  warnstatus    sensefault senseglyphs sensevals  units maxvals minvals peakvals warnhivals warnlovals
  {true,   false,        0,         trbBMP,     0,         1,    32,     0,      0,       29,        -999}, // Boost
  {true,   false,        0,         tboxBMP,    0,         0,    150,    -40,    -40,     140,       -999}, // Transfer Box Temp
  {true,   false,        0,         egtBMP,     0,         0,    900,    -40,    -40,     750,       -999}, // EGT
  {true,   false,        0,         eopBMP,     0,         1,    72,     0,      0,       60,        20},   // Oil Pressure
  {true,   false,        0,         eotBMP,     0,         0,    150,    -40,    -40,     100,       -999}, // Oil Temp
  {true,   false,        0,         coollev,    0,         2,    1,      0,      1,       999,       1},    // Coolant Level
  {true,   false,        0,         D2BMP,      0,         3,    45,     -45,    0,       30,        -30},  // Vehicle Roll
  {true,   false,        0,         D2BMP,      0,         3,    60,     -60,    0,       45,        -45},  // Vehicle Pitch
  {true,   false,        0,         compass,    0,         3,    360,    0,      0,       999,       -999}, // Magnetometer
  {false,  false,        0,         trbBMP,     0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  false,        0,         trbBMP,     0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  false,        0,         trbBMP,     0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  false,        0,         trbBMP,     0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  false,        0,         trbBMP,     0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  false,        0,         trbBMP,     0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  false,        0,         trbBMP,     0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  false,        0,         trbBMP,     0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  false,        0,         trbBMP,     0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  false,        0,         trbBMP,     0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  false,        0,         trbBMP,     0,         1,    60,     0,      0,       45,        -45}   // DUMMY
};

uint8_t sensecount = 0;
const uint8_t totalsensors = 20; //this is the actual number of definitons above
uint8_t rotation = 0; // incremented by 1 with each button press - it's used to tell the drawdisplay functions which sensor data they should output.

// the follow variable is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long previousMillis = 0;        // will store last time the displays were updated
uint8_t interval = 250;         // interval at which to update displays(milliseconds)
//int atmos = 215;                //somewhere to store our startup atmospheric pressure - unused at present

void setup()   {
  //start serial connection
  Serial.begin(9600);  //uncomment to send serial debug info
  Serial.println(totalsensors);
  
  display1.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, true); //construct our displays
  display2.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
  display3.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
//  display4.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
//  display5.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
//  display6.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
  
  display1.clearDisplay();   // clears the screen and buffer
  display2.clearDisplay();   // clears the screen and buffer
  display3.clearDisplay();   // clears the screen and buffer
//  display4.clearDisplay();   // clears the screen and buffer
//  display5.clearDisplay();   // clears the screen and buffer
//  display6.clearDisplay();   // clears the screen and buffer  

  //configure pin8 as an input and enable the internal pull-up resistor, this is for a button to control the rotation of sensors around the screen
  pinMode(ROTBUT, INPUT_PULLUP);

  max.begin(); //initialise the MAX31856
  max.setThermocoupleType(MAX31856_TCTYPE_K); // and set it to a K-type thermocouple - adjust for you hardware!

  accel.begin(); //initialise ADXL345
  mag.begin();

  // read our boost sensor rawADC value since at this point it should be atmospheric pressure...
  //atmos = readBoost(0,0);  // not actually used at this point so could be rmeoved

  // count the number of active sensors
  // first we need to dimension the array
  for (uint8_t thisSensor = 0; thisSensor < totalsensors; thisSensor++) {
    if (Sensors[thisSensor].senseactive==true) {
      sensecount++;

    }
  }

  // set up our analogue inputs on STM32
  for (int x =4; x < 12; x++) {
    pinMode(x, INPUT_ANALOG);
  }
}

void loop() {

  unsigned long currentMillis = millis(); //store the time

  // USER INTERACTION
  bool butVal = digitalRead(ROTBUT); // read the button state
  if (butVal == LOW) { //  && currentMillis - previousMillis > interval
    rotation = rotation + 1; // rotate the screens if the button was pressed
    previousMillis = previousMillis - (interval + 1); // force an update of the screens.
    if (rotation == sensecount) { // this should be total number of sensors in the main sensor array
      rotation = 0;
    }
    delay(250);
  }

  if (currentMillis - previousMillis > interval) { // only read the sensors and draw the screen if 250 millis have passed
    // save the last time we updated
    previousMillis = currentMillis;

    // SENSOR READING

    if (Sensors[0].senseactive) {
      Sensors[0].sensevals = 20; //readBoost(A0, 0); // read boost off A0 and store at index 0
      updatePEAK(0); // TURBO
      audibleWARN(0);
    }

    if (Sensors[1].senseactive) {
      Sensors[1].sensevals = readERR2081(A1, 1); // read A1, currently the Gearbox oil temp sensor
      updatePEAK(1); // TBOX OIL TEMP
      audibleWARN(1);
    }
    
    if (Sensors[2].senseactive) {
      Sensors[2].sensevals = readMAX(2); //read EGT from the Max31856
      updatePEAK(2); // EGT
      audibleWARN(2);
    }
    
    if (Sensors[3].senseactive) {
      Sensors[3].sensevals = readPress(A2, 3); // placeholder at the moment but should be very similar to the boost reading if a cheap pressure sensor is used (ie one which returns a linear voltage 0-5v based on presure)
      updatePEAK(3); // OIL PRESSURE
      audibleWARN(3);
    }
    
    if (Sensors[4].senseactive) {
      Sensors[4].sensevals = readERR2081(A3, 4); // read A7, store at index 4 currently the Engine oil temp sensor
      updatePEAK(4); // OIL TEMP
      audibleWARN(4);
    }

    if (Sensors[5].senseactive) {
      Sensors[5].sensevals = readCoolantLevel(A4, 5); // read A6, to check if coolant level is low
      audibleWARN(5);
      //updatePEAK(5); // Coolant Level - no need to set a max as this is boolean
    }
    
    if (Sensors[6].senseactive) {
      Sensors[6].sensevals = readADXL(6, false); // Inclinometer - Y (roll) angle
      audibleWARN(6);
    }
    
    if (Sensors[7].senseactive) {
      Sensors[7].sensevals = readADXL(6, true); // Inclinometer - Y (roll) angle
      audibleWARN(7);
    }    

    if (Sensors[8].senseactive) {
      Sensors[8].sensevals = readHMC5883(8); // Magnetometer
      // no audible warning for compass heading
    }  

    // DRAW DISPLAYS
    drawDISPLAY1();
    drawDISPLAY2();
    drawDISPLAY3();
//    drawDISPLAY4();
//    drawDISPLAY5();
//    drawDISPLAY6();    
  }
}

void drawDISPLAY1(void) { // DISPLAY 1 is our Main guage display

  uint8_t sensor0 = posrot(1);

  drawSensor(0, display1, sensor0);

  drawBarGraph(display1, sensor0);
  
  display1.display();
  display1.clearDisplay();
}

void drawDISPLAY2(void) { // DISPLAY 2 shows 2 sensors
  uint8_t sensor2 = posrot(2);
  uint8_t sensor5 = posrot(5);

  drawSensor(0, display2, sensor2);

  drawSensor(33, display2, sensor5);
//  drawBarGraph(display2, sensor2);
  display2.display();
  display2.clearDisplay();
}

void drawDISPLAY3(void) { // DISPLAY 3 shows 2 sensors
  uint8_t sensor3 = posrot(3);
  uint8_t sensor4 = posrot(4);

  drawSensor(0, display3, sensor3);

  drawSensor(33, display3, sensor4);
//  drawBarGraph(display3, sensor3);
  display3.display();
  display3.clearDisplay();
}

//void drawDISPLAY4(void) { // DISPLAY 3 shows 2 sensors
//  uint8_t sensor6 = posrot(6);
//
//  drawSensor(33, display4, sensor6);
//  drawBarGraph(display4, sensor6);
//  display4.display();
//  display4.clearDisplay();
//}
//
//void drawDISPLAY5(void) { // DISPLAY 3 shows 2 sensors
//  uint8_t sensor5 = posrot(5);
//
//  drawSensor(33, display5, sensor5);
//  drawBarGraph(display5, sensor5);
//  display5.display();
//  display5.clearDisplay();
//}
//
//void drawDISPLAY6(void) { // DISPLAY 3 shows 2 sensors
//  uint8_t sensor4 = posrot(4);
//  
//  drawSensor(33, display6, sensor4);
//  drawBarGraph(display6, sensor4);
//  display6.display();
//  display6.clearDisplay();
//}



// Helper Functions

void drawSensor(uint8_t y, Adafruit_SSD1306 &refDisp, uint8_t sensor) {
  uint8_t xoffset = 0;
  String temp;
  int8_t rolltemp = 0;
  refDisp.setTextSize(1);
  refDisp.setTextColor(WHITE);
  refDisp.setTextWrap(false);
  refDisp.setFont(&FreeSansBoldOblique12pt7b); //switch to a nice ttf font 12x7
  refDisp.setCursor(46, y + 9 + 15);
  refDisp.println(valIfnoErr(sensor));
  temp = valIfnoErr(sensor);
  xoffset = (temp.length() * 13) + 5 ; // work out width of the characters so we can move the cursor to the correct position to display our units symbol

  if (Sensors[sensor].sensefault > 0 || sensor == 5) { // normal size text if it's an error message or it's our low coolant warning sensor
    refDisp.setTextSize(1);
    refDisp.setCursor(46 + xoffset, y + 9 + 15);
  }  else {
    refDisp.setFont();
    refDisp.setTextSize(1);
    refDisp.setCursor(46 + xoffset, y + 9);
  }

  refDisp.println(units(sensor));
  
  if (sensor == 6) { // INCLINOMETER ONLY (ANIMATED)
    rolltemp = Sensors[sensor].sensevals;
    if (rolltemp > -5 && rolltemp < 5) { // centred
      refDisp.drawBitmap(0, y, D2a0, 32, 32, WHITE);
    } else if (rolltemp > -10 && rolltemp <= -5) { //-5 deg
      refDisp.drawBitmap(0, y, D2a5L, 32, 32, WHITE);
    } else if (rolltemp > -15 && rolltemp <= -10) { //-10 deg
      refDisp.drawBitmap(0, y, D2a10L, 32, 32, WHITE);
    } else if (rolltemp > -20 && rolltemp <= -15) { //-15 deg
      refDisp.drawBitmap(0, y, D2a15L, 32, 32, WHITE);
    } else if (rolltemp > -25 && rolltemp <= -20) { //-20 deg
      refDisp.drawBitmap(0, y, D2a20L, 32, 32, WHITE);
    } else if (rolltemp > -30 && rolltemp <= -25) { //-25 deg
      refDisp.drawBitmap(0, y, D2a25L, 32, 32, WHITE);
    } else if (rolltemp > -35 && rolltemp <= -30) { //-30 deg
      refDisp.drawBitmap(0, y, D2a30L, 32, 32, WHITE);
    } else if (rolltemp > -40 && rolltemp <= -35) { //-35 deg
      refDisp.drawBitmap(0, y, D2a35L, 32, 32, WHITE);
    } else if (rolltemp > -45 && rolltemp <= -40) { //-40 deg
      refDisp.drawBitmap(0, y, D2a40L, 32, 32, WHITE);
    } else if (rolltemp > -50 && rolltemp <= -45) { //-45 deg
      refDisp.drawBitmap(0, y, D2a45L, 32, 32, WHITE);

    } else if (rolltemp >= 5 && rolltemp < 10) { //5 deg
      refDisp.drawBitmap(0, y, D2a5R, 32, 32, WHITE);
    } else if (rolltemp >= 10 && rolltemp < 15) { //10 deg
      refDisp.drawBitmap(0, y, D2a10R, 32, 32, WHITE);
    } else if (rolltemp >= 15 && rolltemp < 20) { //20 deg
      refDisp.drawBitmap(0, y, D2a15R, 32, 32, WHITE);
    } else if (rolltemp >= 20 && rolltemp < 25) { //20 deg
      refDisp.drawBitmap(0, y, D2a20R, 32, 32, WHITE);
    } else if (rolltemp >= 25 && rolltemp < 30) { //25 deg
      refDisp.drawBitmap(0, y, D2a25R, 32, 32, WHITE);
    } else if (rolltemp >= 30 && rolltemp < 35) { //30 deg
      refDisp.drawBitmap(0, y, D2a30R, 32, 32, WHITE);
    } else if (rolltemp >= 35 && rolltemp < 40) { //35 deg
      refDisp.drawBitmap(0, y, D2a35R, 32, 32, WHITE);
    } else if (rolltemp >= 40 && rolltemp < 45) { //40 deg
      refDisp.drawBitmap(0, y, D2a40R, 32, 32, WHITE);
    } else if (rolltemp >= 45 && rolltemp < 50) { //45 deg
      refDisp.drawBitmap(0, y, D2a45R, 32, 32, WHITE);

      // WARNING CASE
    } else if (rolltemp < -45 || rolltemp > 45) { // WARNING!
      refDisp.drawBitmap(0, y, D2aWARN, 32, 32, WHITE);
    }
  
  } else if (sensor == 7) { // INCLINOMETER ONLY (ANIMATED)
    rolltemp = Sensors[sensor].sensevals;
    if (rolltemp > -10 && rolltemp < 10) { // centred
      refDisp.drawBitmap(0, y, D2p0, 32, 32, WHITE);
    } else if (rolltemp > -20 && rolltemp <= -10) { //-10 deg
      refDisp.drawBitmap(0, y, D2p10L, 32, 32, WHITE);
    } else if (rolltemp > -30 && rolltemp <= -20) { //-20 deg
      refDisp.drawBitmap(0, y, D2p20L, 32, 32, WHITE);
    } else if (rolltemp > -40 && rolltemp <= -30) { //-30 deg
      refDisp.drawBitmap(0, y, D2p30L, 32, 32, WHITE);
    } else if (rolltemp > -50 && rolltemp <= -40) { //-40 deg
      refDisp.drawBitmap(0, y, D2p40L, 32, 32, WHITE);
    } else if (rolltemp > -60 && rolltemp <= -50) { //-50 deg
      refDisp.drawBitmap(0, y, D2p50L, 32, 32, WHITE);
    } else if (rolltemp > -70 && rolltemp <= -60) { //-60 deg
      refDisp.drawBitmap(0, y, D2p60L, 32, 32, WHITE);

    } else if (rolltemp >= 10 && rolltemp < 20) { //10 deg
      refDisp.drawBitmap(0, y, D2p10R, 32, 32, WHITE);
    } else if (rolltemp >= 20 && rolltemp < 30) { //20 deg
      refDisp.drawBitmap(0, y, D2p20R, 32, 32, WHITE);
    } else if (rolltemp >= 30 && rolltemp < 40) { //30 deg
      refDisp.drawBitmap(0, y, D2p30R, 32, 32, WHITE);
    } else if (rolltemp >= 40 && rolltemp < 50) { //40 deg
      refDisp.drawBitmap(0, y, D2p40R, 32, 32, WHITE);
    } else if (rolltemp >= 50 && rolltemp < 60) { //50 deg
      refDisp.drawBitmap(0, y, D2p50R, 32, 32, WHITE);
    } else if (rolltemp >= 60 && rolltemp < 70) { //60 deg
      refDisp.drawBitmap(0, y, D2p60R, 32, 32, WHITE);


      // WARNING CASE
    } else if (rolltemp < -60 || rolltemp > 60) { // WARNING!
      refDisp.drawBitmap(0, y, D2aWARN, 32, 32, WHITE);
    }


    
  } else {
    //ALL OTHER SENSORS
    refDisp.drawBitmap(0, y, Sensors[sensor].senseglyphs, 32, 32, WHITE);
  }
  // DO sensor visual warnings
  if (hiloWARN(sensor, true)) {
    refDisp.drawBitmap(100, y + 4, triBMP, 24, 24, WHITE); //outut the warning triangle
//    if (PIEZO > 0) {
//      tone(PIEZO, 2000, 200);
//    }
  }
  if (faultWARN(sensor) == 1) {
    refDisp.drawBitmap(100, y + 4, NoConn, 24, 24, WHITE); //output the disconnected sensor icon
  }
  refDisp.setFont(); //reset to basic font
}

void drawBarGraph(Adafruit_SSD1306 &refDisp, uint8_t sensor) {
  int padding = 0;
  float scalerange = 0;
  int scaleposmin = 0;
  
  refDisp.drawLine(11, 52, 11, 54, WHITE); // draw our gauge and scale markings
  refDisp.drawLine(64, 52, 64, 54, WHITE);
  refDisp.drawLine(116, 52, 116, 54, WHITE);
  refDisp.drawRect(11, 42, 106, 10, WHITE); //Border of the bar chart
  if (Sensors[sensor].senseminvals < 0) { // Work out a positive range of values that we need to plot
    scaleposmin = Sensors[sensor].senseminvals * -1;
    scalerange = scaleposmin + Sensors[sensor].sensemaxvals;
  } else {
    scaleposmin = Sensors[sensor].senseminvals;
    scalerange = Sensors[sensor].sensemaxvals - scaleposmin;
  }
  refDisp.fillRect(14, 44, (100 / scalerange * (Sensors[sensor].sensevals + scaleposmin)), 6, WHITE); //Draws the bar depending on the sensor value

  refDisp.drawLine(13 + (100 / scalerange * (Sensors[sensor].sensepeakvals + scaleposmin)), 41, 13 + (100 / scalerange * (Sensors[sensor].sensepeakvals + scaleposmin)), 50, WHITE); // draw the peak value line;
  if (Sensors[sensor].sensevals < 100) { // adjust padding for the low value so it looks nice
    padding = 0;
  } else {
    padding = -4;
  }
  refDisp.setCursor(8 + padding + (100 / scalerange * (Sensors[sensor].sensepeakvals + scaleposmin)), 33); // set cursor with padding
  refDisp.setTextSize(1);
  refDisp.println(String(Sensors[sensor].sensepeakvals)); // and write the peak val
  refDisp.setCursor(8, 57);
  refDisp.println(String(Sensors[sensor].senseminvals)); // draw the minumum value
  refDisp.setCursor(58, 57);
  if (Sensors[sensor].senseminvals < 100) { // adjust padding for the low value so it looks nice
    padding = 8;
  } else {
    padding = 0;
  }
  refDisp.println(String(static_cast<int>(((Sensors[sensor].sensemaxvals - Sensors[sensor].senseminvals) / 2) - scaleposmin))); // draw the midpoint value
  if (Sensors[sensor].sensemaxvals < 100) { // adjust padding for the high value so it looks nice/doesnt wrap off screen
    padding = 10;
  } else {
    padding = 5;
  }
  refDisp.setCursor(100 + padding, 57);
  refDisp.println(String(Sensors[sensor].sensemaxvals)); // draw the maximum value
}

bool hiloWARN(uint8_t sensor, bool toggle) {
  // this function toggles a an error flag if the current sensor is above it's high warning parameter or below it's low warning paramater - since the display is redrawn every 250ms it appears to flash
  if (Sensors[sensor].sensefault > 0 && sensor !=5) { // we don't want to display a high or low warning if there's a sensor fault (ie wiring issue etc).
    return (false);
  }

  
  if (Sensors[sensor].sensevals > Sensors[sensor].sensewarnhivals || Sensors[sensor].sensevals < Sensors[sensor].sensewarnlowvals) {
    if (Sensors[sensor].warnstatus == true) {
      if (toggle) {
        Sensors[sensor].warnstatus = false; // we toggle the value so that the warning triangle flashes based on the interval we are looping at in loop()
      }
      return(false);
    } else {
      if (toggle) {
        Sensors[sensor].warnstatus = true;
      }
      return(true);
    }
  } else {
    if (toggle) {
      Sensors[sensor].warnstatus = false;
    }
    return false;
  }
  return (Sensors[sensor].warnstatus);
}

void audibleWARN(uint8_t sensor) {
  if (hiloWARN(sensor, false)) {
    if (PIEZO > 0) {
      tone(PIEZO, 2000, 200);
    }
  }
}

uint8_t faultWARN(uint8_t sensor) {
  // this function alternates a flag between 1 & 2 if it is set - since the display is redrawn every 250ms it appears to flash
  if (Sensors[sensor].sensefault > 0) {
    if (Sensors[sensor].sensefault == 1) {
      Sensors[sensor].sensefault = 2; // we toggle the value so that the fault icon flashes based on the interval we are looping at in loop()
    } else {
      Sensors[sensor].sensefault = 1;
    }
  }
  return (Sensors[sensor].sensefault);
}

void toggleFault(uint8_t sensor) {
  // toggles the fault strate of a sensor (to make our warning symbols flash)
  if (Sensors[sensor].sensefault == 2) {
    Sensors[sensor].sensefault = 2; // 2 is animation off
  } else {
    Sensors[sensor].sensefault = 1; // 1 is animation on
  }
}

void updatePEAK(uint8_t sensor) {
  // stores the current value of a sensor if it is above the previously stored high value
  if (Sensors[sensor].sensevals >= Sensors[sensor].senseminvals) { // only do this if the value is above the min
    if (Sensors[sensor].sensevals <= Sensors[sensor].sensemaxvals) { // only do this if the value is below the max
      if (Sensors[sensor].sensevals > Sensors[sensor].sensepeakvals) {
        Sensors[sensor].sensepeakvals = int(Sensors[sensor].sensevals); //if we have a new max then store it
      }
    }
  }
}

String units(uint8_t sensor) { // returns the units associated with the sensor, or some fault text
  // if a fault is set return ERR
  if (Sensors[sensor].sensefault > 0 && sensor != 5) {
    return (F("Er"));
  }
  // if no fault then return the correct units (saves us some memory usage)

  switch (Sensors[sensor].senseunits) {
    case 0:
      return (F("C"));
    case 1:
      return (F("psi"));
    case 2:
      if (Sensors[sensor].sensefault > 0) {
        return (F("Low"));
      }
      return (F("OK"));
    case 3:
      return (F("o"));
  }
}

String valIfnoErr(uint8_t sensor) { //prevents values being displayed if we are in fault state OR this is a boolean sensor (coolant level)
  String text = String(Sensors[sensor].sensevals);
  // if a fault is set return an empty string

  if (sensor == 5) {
    return (F(""));
  }
  if (Sensors[sensor].sensefault > 0 || sensor == 5) {
    return (F(""));
  }
  return (text);
}

uint8_t posrot(uint8_t location) { // this is used to shift our array of data around the screens

uint8_t count=0;
uint8_t pos[sensecount];

// now we populate the array with the active sensors
  for (uint8_t locthisSensor = 0; locthisSensor < totalsensors; locthisSensor++) { 
    if (Sensors[locthisSensor].senseactive == true) {
      pos[count]=locthisSensor;
      count++;
    }
  }
  //uint8_t pos[] = {0, 1, 2, 3, 4, 5, 6};
// return the correct sensor for the current location  
  location = location - 1 + rotation;

  if (location > count-1) {
    location = location % count;
  }
  return (pos[location]);
}

int doFaults(int constraint, int checkval, int retval, uint8_t index) { //
  if (checkval < constraint) {
    toggleFault(index); //fault!
    Sensors[index].sensepeakvals = Sensors[index].senseminvals; //set the peak value to minimum (tidies the display)
    retval = Sensors[index].senseminvals; //return minimum value
  } else {
    Sensors[index].sensefault = 0; // no fault
  }
  return (retval);
}

// Sensor reading code.

int readERR2081(uint8_t sensor, uint8_t index) {
  int raw = 0;     // variable to store the raw ADC input value
  float Vin = 3.3; // variable to store the measured VCC voltage
  float Vout = 0;  // variable to store the output voltage
  int R2 = 1000;   // variable to store the R2 value
  float Rth = 0;   // variable to store the thermistor value

  // THERMISTOR CODE
  raw = analogRead(sensor);      // Reads the Input PIN
  Vout = (Vin / DIVISOR) * raw;     // Calculates the Voltage on the Input PIN
  Rth = ((R2 * Vin) / Vout) - R2;   // Calculates the Resistance of the Thermistor
  float steinhart;                  // This next stage calculates the temp from the resistance
  steinhart = Rth / 2012.2;         // (R/Ro)  therm @ 25C = 1986
  steinhart = log(steinhart);       // ln(R/Ro)
  steinhart /= 3502;                // 1/B * ln(R/Ro) b coefficient = 3344
  steinhart += 1.0 / (25 + 273.15); // + (1/To) nominal temp is 25c
  steinhart = 1.0 / steinhart;      // Invert
  steinhart -= 273.15;              // convert to C
  // END THERMISTOR CODE

  // FAULT checking
  // Sensors should be connected with a 1K pulldown resistor - if there's is a connection fault a low raw read will indicate this.
  return (doFaults(DIVISOR/100, raw, int(steinhart), index));
}


int readBoost(uint8_t sensor, uint8_t index) {
  int rawval;
  float kpaval;
  float boost;
  rawval = analogRead(sensor);       // Read MAP sensor raw value on analog port 0
  kpaval = rawval * 0.4878;             // convert to kpa
  boost = kpaval * 0.145038 - 14.5038;  // Convert to psi and subtract atmospheric (sensor is absolute pressure)
  // process any faults
  return (doFaults(DIVISOR/100, rawval, int(boost), index));
}

int readMAX(uint8_t index) {
  // Make sure you remove the delay(250) from the adafruit_MAX31856 readThermocoupleTemperature() - otherwise the screen rotation and refresh slows down to approx every 500ms
  int t;
  t = max.readThermocoupleTemperature();
  // process any faults
  return (doFaults(max.readFault(), 0, t, index));
}

int readPress(uint8_t sensor, uint8_t index) {
  //just a dummy at present
  int p;
  p = analogRead(sensor);
  // process any faults
  return (doFaults(DIVISOR/100, p, p, index));
}

bool readCoolantLevel(uint8_t sensor, uint8_t index) {
  // sensor is normally closed
  // use a pulldown resistor to enable fault monitoring
  int CoolantLevel;
  CoolantLevel = analogRead(sensor);
  // process any faults
  return ((bool)doFaults(DIVISOR/2, CoolantLevel, CoolantLevel, index));
}

int readADXL(uint8_t index, bool axis) {  // false is Y true is X
  sensors_event_t event;
  accel.getEvent(&event);
  double ax, ay, az;

  ax = event.acceleration.x;
  ay = event.acceleration.y;
  az = event.acceleration.z;
  
  //  we don't care about yaw - leaving code commented here for reference
  //  double zAngle = atan( sqrt(square(ax) + square(ay)) / az);
  //  zAngle *= 180.00;
  //  zAngle /= 3.141592;
  if(axis) {
    double xAngle = atan( ax / (sqrt(sq(ay) + sq(az))));
    xAngle *= 180.00;
    xAngle /= 3.141592;
    return (int(xAngle));
  }
  double yAngle = atan( ay / (sqrt(sq(ax) + sq(az))));
  yAngle *= 180.00;
  yAngle /= 3.141592;    
  return (int(yAngle));
}

int readHMC5883(uint8_t index)
{
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);
 
  /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
  Serial.print("X: "); Serial.print(event.magnetic.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  ");Serial.println("uT");

  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 0.025;
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 
  
  Serial.print("Heading (degrees): "); Serial.println(headingDegrees);
  return(int(headingDegrees));
}  

