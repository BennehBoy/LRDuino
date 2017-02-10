// LRDuino by Ben Anderson
// Version 0.96  (STM32 Only)

#include <Adafruit_SSD1306.h>
#include "LRDuinoGFX.h"
#include <Fonts/FreeSansBoldOblique12pt7b.h>
#include <Fonts/FreeSansBoldOblique24pt7b.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_HMC5883_U.h>

#define ELM_TIMEOUT 1000
#define ELM_BAUD_RATE 9600
#define ELM_PORT Serial3

#include "ELM327.h"

// Following pinout details are for Maple Mini
#define OLED_RESET  12
#define OLED_DC     13  //14 //DC
// 15 is I2C SDA (for ADXL)
// 16 is I2C SCL
#define OLED_CLK    6  // SPI_SCK
#define OLED_MOSI   4  // SPI_MOSI
#define OLED_CS     18
#define OLED_CS_2   19
#define OLED_CS_3   20
#define OLED_CS_4   22
#define OLED_CS_5   25
#define OLED_CS_6   26
#define OLED_CS_7   27
#define OLED_CS_8   28
#define MAX_CS      29
#define MAX_MISO    5 // SPI MISO
#define ROTBUT      21 // our input button
#define A0          11 // boost
#define A1          10 // tbox temp
#define A2          9  // Oil pressure
#define A3          8  // Oil Temp
#define A4          7  // Coolant Level
#define PIEZO       3

#define DIVISOR     4095

// MAX31856 registers
#define NumRegisters 10
byte RegisterValues[] =    {0x90,  0x03,   0xFC,   0x7F,   0xC0,   0x07,     0xFF,     0x80,     0x00,     0x00 };
String RegisterNames[] =   {"CR0", "CR1", "MASK", "CJHF", "CHLF", "LTHFTH", "LTHFTL", "LTLFTH", "LTLFTL", "CJTO"};
byte RegisterAddresses[] = {0x00,  0x01,   0x02,   0x03,   0x04,   0x04,     0x06,     0x07,     0x08,     0x09 };

byte status;
Elm327 Elm;

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

//HARDWARE SPI
Adafruit_SSD1306 display1(OLED_DC, OLED_RESET, OLED_CS);
Adafruit_SSD1306 display2(OLED_DC, OLED_RESET, OLED_CS_2);
Adafruit_SSD1306 display3(OLED_DC, OLED_RESET, OLED_CS_3);
Adafruit_SSD1306 display4(OLED_DC, OLED_RESET, OLED_CS_4);
Adafruit_SSD1306 display5(OLED_DC, OLED_RESET, OLED_CS_5);
Adafruit_SSD1306 display6(OLED_DC, OLED_RESET, OLED_CS_6);
Adafruit_SSD1306 display7(OLED_DC, OLED_RESET, OLED_CS_7);
Adafruit_SSD1306 display8(OLED_DC, OLED_RESET, OLED_CS_8);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// This is all the parameters and variables for our sensors

typedef struct
{
  bool senseactive;
  bool master;
  uint8_t slaveID;
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
  //active   master slaveID warnstatus    sensefault senseglyphs sensevals  units maxvals minvals peakvals warnhivals warnlovals
  {true,   true,  99,     false,        0,         trbBMP,     0,         1,    32,     0,      0,       29,        -999}, // Boost
  {true,   true,  99,     false,        0,         tboxBMP,    0,         0,    150,    -40,    -40,     140,       -999}, // Transfer Box Temp
  {true,   true,  99,     false,        0,         egtBMP,     0,         0,    900,    -40,    -40,     750,       -999}, // EGT
  {true,   true,  4,      false,        0,         eopBMP,     0,         1,    72,     0,      0,       60,        20},   // Oil Pressure
  {true,   false, 99,     false,        0,         eotBMP,     0,         0,    150,    -40,    -40,     100,       -999}, // Oil Temp
  {true,   true,  99,     false,        0,         coollev,    0,         2,    1,      0,      1,       999,       1},    // Coolant Level
  {true,   true,  7,      false,        0,         D2BMP,      0,         3,    45,     -45,    0,       30,        -30},  // Vehicle Roll
  {true,   false, 99,     false,        0,         D2BMP,      0,         3,    60,     -60,    0,       45,        -45},  // Vehicle Pitch
  {true,   true,  99,     false,        0,         compass,    0,         3,    360,    0,      0,       999,       -999}, // Magnetometer
  {true,   true,  99,     false,        0,         Gauge,      750,       4,    4500,   0,      0,       4500,      600},  // RPM
  {true,   true,  99,     false,        0,         Gauge,      0,         5,    100,    -30,    0,       100,       -30},  // Roadspeed
  {false,  true,  99,     false,        0,         OBDII,      0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  true,  99,     false,        0,         OBDII,      0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  true,  99,     false,        0,         OBDII,      0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  true,  99,     false,        0,         OBDII,      0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  true,  99,     false,        0,         OBDII,      0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  true,  99,     false,        0,         OBDII,      0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  true,  99,     false,        0,         OBDII,      0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  true,  99,     false,        0,         OBDII,      0,         1,    60,     0,      0,       45,        -45},  // DUMMY
  {false,  true,  99,     false,        0,         OBDII,      0,         1,    60,     0,      0,       45,        -45}   // DUMMY
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
  // HC05 init
  Elm.begin();

  // Pin setup
  pinMode (OLED_CS, OUTPUT);
  digitalWrite(OLED_CS, HIGH);
  pinMode (OLED_CS_2, OUTPUT);
  digitalWrite(OLED_CS_2, HIGH);
  pinMode (OLED_CS_3, OUTPUT);
  digitalWrite(OLED_CS_3, HIGH);
  pinMode (OLED_CS_4, OUTPUT);
  digitalWrite(OLED_CS_4, HIGH);
  pinMode (OLED_CS_5, OUTPUT);
  digitalWrite(OLED_CS_5, HIGH);
  pinMode (OLED_CS_6, OUTPUT);
  digitalWrite(OLED_CS_6, HIGH);
  pinMode (OLED_CS_7, OUTPUT);
  digitalWrite(OLED_CS_7, HIGH);
  pinMode (OLED_CS_8, OUTPUT);
  digitalWrite(OLED_CS_8, HIGH);
  pinMode (MAX_CS, OUTPUT);
  digitalWrite(MAX_CS, HIGH);

  InitializeChannel(MAX_CS); // Init the MAX31856

  display1.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, true); //construct our displays
  display2.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
  display3.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
  display4.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
  display5.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
  display6.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
  display7.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
  display8.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);

  display1.clearDisplay();   // clears the screen and buffer
  display2.clearDisplay();   // clears the screen and buffer
  display3.clearDisplay();   // clears the screen and buffer
  display4.clearDisplay();   // clears the screen and buffer
  display5.clearDisplay();   // clears the screen and buffer
  display6.clearDisplay();   // clears the screen and buffer
  display7.clearDisplay();   // clears the screen and buffer
  display8.clearDisplay();   // clears the screen and buffer

  display1.display();
  display2.display();
  display3.display();
  display4.display();
  display5.display();
  display6.display();
  display7.display();
  display8.display();

  //configure pin8 as an input and enable the internal pull-up resistor, this is for a button to control the rotation of sensors around the screen
  pinMode(ROTBUT, INPUT_PULLUP);

  accel.begin(); //initialise ADXL345
  mag.begin();

  // read our boost sensor rawADC value since at this point it should be atmospheric pressure...
  //atmos = readBoost(0,0);  // not actually used at this point so could be rmeoved

  // count the number of active sensors
  // first we need to dimension the array
  for (uint8_t thisSensor = 0; thisSensor < totalsensors; thisSensor++) {
    if (Sensors[thisSensor].senseactive == true && Sensors[thisSensor].master == true) { // don't count slaves
      sensecount++;

    }
  }

  // set up our analogue inputs on STM32
  for (int x = 7; x < 12; x++) {
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
    delay(10);
  }

  if (currentMillis - previousMillis > interval) { // only read the sensors and draw the screen if 250 millis have passed
    // save the last time we updated
    previousMillis = currentMillis;

    // SENSOR READING

    if (Sensors[0].senseactive) {
      Sensors[0].sensevals = readBoost(A0, 0); // read boost off A0 and store at index 0
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

    if (Sensors[9].senseactive) {
      int intRPM = 0;
      int status;
      status = Elm.engineRPM(intRPM);
      Sensors[9].sensevals = intRPM;

      if (status != ELM_SUCCESS) {
        Serial.print("ERROR: ");
        Serial.println(status);
      }
    }

    if (Sensors[10].senseactive) {
      byte intMPH = 0;
      int status;
      status = Elm.vehicleSpeed(intMPH);
      Sensors[10].sensevals = int(intMPH);

      if (status != ELM_SUCCESS) {
        Serial.print("ERROR: ");
        Serial.println(status);
      }
    }

    // DRAW DISPLAYS
    drawDISPLAY(display1, 1);
    drawDISPLAY(display2, 2);
    drawDISPLAY(display3, 3);
    drawDISPLAY(display4, 4);
    drawDISPLAY(display5, 8);
    drawDISPLAY(display6, 7);
    drawDISPLAY(display7, 6);
    drawDISPLAY(display8, 5);
  }
}

// 4 screens - use index values like this to rotate around the displays
// 1 2
// 4 3

// 6 screens - use index values like this to rotate around the displays
// 1 2 3
// 6 5 4

// 8 screens - use index values like this to rotate around the displays
// 1 2 3 4
// 8 7 6 5

void drawDISPLAY(Adafruit_SSD1306 &refDisp, uint8_t index) { // DISPLAY 1 is our Main guage display

  uint8_t sensor0 = posrot(index);

  if (sensor0 == 8) { // draw compass
    drawCompass(32, 32, 30, refDisp, sensor0);
    drawSensor(15, 20, refDisp, sensor0, false);
  } else if (Sensors[sensor0].slaveID != 99) { // draw paired sensors
    drawSensor(0, 0, refDisp, sensor0, true);
    drawSensor(33, 0, refDisp, Sensors[sensor0].slaveID, true);
  } else if ((sensor0 == 9) || (sensor0 == 10)) {
    drawOBD(refDisp, sensor0);
  } else {
    drawSensor(0, 0, refDisp, sensor0, true); // draw all other sensors with a standard bargraph
    drawBarGraph(refDisp, sensor0);
  }

refDisp.display();
refDisp.clearDisplay();
}


// Helper Functions

void drawSensor(uint8_t y, uint8_t x, Adafruit_SSD1306 &refDisp, uint8_t sensor, bool icons) {
  uint8_t xoffset = 0;
  String temp;
  int8_t rolltemp = 0;

  refDisp.setTextWrap(false);

  refDisp.setFont(&FreeSansBoldOblique12pt7b); //switch to a nice ttf font 12x7
  display_item(46 + x, y + 9 + 15, valIfnoErr(sensor), 1, refDisp); // x should only be given a value if we are not showing icons (eg for the compass display)

  temp = valIfnoErr(sensor);
  xoffset = (temp.length() * 13) + 5 ; // work out width of the characters so we can move the cursor to the correct position to display our units symbol

  if (Sensors[sensor].sensefault > 0 || sensor == 5) { // normal size text if it's an error message or it's our low coolant warning sensor
    display_item(46 + x + xoffset, y + 9 + 15, units(sensor), 1, refDisp);
  }  else {
    refDisp.setFont(); // switch to small standard font
    display_item(46 + x + xoffset, y + 9, units(sensor), 1, refDisp);
  }

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
      if (icons) {
        refDisp.drawBitmap(0, y, D2aWARN, 32, 32, WHITE);
      }
    }

  } else {
    //ALL OTHER SENSORS
    if (icons) {
      refDisp.drawBitmap(0, y, Sensors[sensor].senseglyphs, 32, 32, WHITE);
    }
  }
  // DO sensor visual warnings
  if (hiloWARN(sensor, true) && icons) {
    refDisp.drawBitmap(100, y + 4, triBMP, 24, 24, WHITE); //outut the warning triangle
  }
  if (faultWARN(sensor) == 1 && icons) {
    refDisp.drawBitmap(100, y + 4, NoConn, 24, 24, WHITE); //output the disconnected sensor icon
  }
  refDisp.setFont(); //reset to basic font
}

void drawOBD(Adafruit_SSD1306 &refDisp, uint8_t sensor) {
  uint8_t xoffset = 0;
  String temp;
  int8_t rolltemp = 0;

  refDisp.setTextWrap(false);

  refDisp.setFont(&FreeSansBoldOblique12pt7b); //switch to a nice ttf font 12x7
  
  refDisp.drawBitmap(95, 38, Sensors[sensor].senseglyphs, 32, 32, WHITE); //draw the sensor icon
  
  display_item(40, 60, units(sensor), 1, refDisp);
  
  refDisp.setFont(&FreeSansBoldOblique24pt7b);
  
  display_item(10, 34, valIfnoErr(sensor), 1, refDisp); // draw the value

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
  display_item(8 + padding + (100 / scalerange * (Sensors[sensor].sensepeakvals + scaleposmin)), 33, String(Sensors[sensor].sensepeakvals), 1, refDisp); // set cursor with padding & write the peak val
  display_item(8, 57, String(Sensors[sensor].senseminvals), 1, refDisp); // draw the minumum value
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
  display_item(100 + padding, 57, String(Sensors[sensor].sensemaxvals), 1, refDisp);
}

void display_item(int x, int y, String token, int txt_size, Adafruit_SSD1306 &refDisp) {
  refDisp.setCursor(x, y);
  refDisp.setTextColor(WHITE);
  refDisp.setTextSize(txt_size);
  refDisp.print(token);
  refDisp.setTextSize(1); // Back to default text size
}

void arrow(int x2, int y2, int x1, int y1, int alength, int awidth, int colour, Adafruit_SSD1306 &refDisp) {
  float distance;
  int dx, dy, x2o, y2o, x3, y3, x4, y4, k;
  distance = sqrt(pow((x1 - x2), 2) + pow((y1 - y2), 2));
  dx = x2 + (x1 - x2) * alength / distance;
  dy = y2 + (y1 - y2) * alength / distance;
  k = awidth / alength;
  x2o = x2 - dx;
  y2o = dy - y2;
  x3 = y2o * k + dx;
  y3 = x2o * k + dy;
  x4 = dx - y2o * k;
  y4 = dy - x2o * k;
  refDisp.drawLine(x1, y1, x2, y2, colour);
  refDisp.drawLine(x1, y1, dx, dy, colour);
  refDisp.drawLine(x3, y3, x4, y4, colour);
  refDisp.drawLine(x3, y3, x2, y2, colour);
  refDisp.drawLine(x2, y2, x4, y4, colour);
}

void drawCompass(uint8_t centreX, uint8_t centreY, uint8_t radius, Adafruit_SSD1306 &refDisp, uint8_t sensor) {
  int dxo, dyo, dxi, dyi, dx, dy;
  refDisp.drawCircle(centreX, centreY, radius, WHITE); // Draw compass circle
  for (float i = 0; i < 360; i = i + 22.5) {
    dxo = radius * cos(i * 3.14 / 180);
    dyo = radius * sin(i * 3.14 / 180);
    dxi = dxo * 0.95;
    dyi = dyo * 0.95;
    refDisp.drawLine(dxi + centreX, dyi + centreY, dxo + centreX, dyo + centreY, WHITE);
  }
  display_item((centreX - 2), (centreY - 24), "N", 1, refDisp);
  display_item((centreX - 2), (centreY + 17), "S", 1, refDisp);
  display_item((centreX + 19), (centreY - 3), "E", 1, refDisp);
  display_item((centreX - 23), (centreY - 3), "W", 1, refDisp);

  dx = (0.7 * radius * cos((Sensors[sensor].sensevals - 90) * 3.14 / 180)) + centreX; // calculate X position for the screen coordinates - can be confusing!
  dy = (0.7 * radius * sin((Sensors[sensor].sensevals - 90) * 3.14 / 180)) + centreY;
  arrow(dx, dy, centreX, centreY, 2, 2, WHITE, refDisp);
}

bool hiloWARN(uint8_t sensor, bool toggle) {
  // this function toggles a an error flag if the current sensor is above it's high warning parameter or below it's low warning paramater - since the display is redrawn every 250ms it appears to flash
  if (Sensors[sensor].sensefault > 0 && sensor != 5) { // we don't want to display a high or low warning if there's a sensor fault (ie wiring issue etc).
    return (false);
  }
  if (Sensors[sensor].sensevals > Sensors[sensor].sensewarnhivals || Sensors[sensor].sensevals < Sensors[sensor].sensewarnlowvals) { // if we're under the min or over the max then warn!
    if (Sensors[sensor].warnstatus == true) { // if we're already in a wanring state
      if (toggle) { // only used when being called from inside the display because we use this to flash the warning icons - tones are dealt with outside the display loop (eg if we have more sensors than we can draw at once)
        Sensors[sensor].warnstatus = false; // we toggle the value so that the warning triangle flashes based on the interval we are looping at in loop()
      }
      return (false);
    } else {
      if (toggle) {
        Sensors[sensor].warnstatus = true;
      }
      return (true);
    }
  } else { // otherwise return false
    if (toggle) {
      Sensors[sensor].warnstatus = false;
    }
    return false;
  }
  return (Sensors[sensor].warnstatus); // return the current value in the case that there's a connection issue
}

void audibleWARN(uint8_t sensor) {
  // sound the buzzer if their's a warning condition
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
    case 4:
      return (F("rpm"));
    case 5:
      return (F("mph"));
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
  uint8_t count = 0;
  uint8_t pos[sensecount];

  // now we populate the array with the active sensors
  for (uint8_t locthisSensor = 0; locthisSensor < totalsensors; locthisSensor++) {
    if (Sensors[locthisSensor].senseactive == true && Sensors[locthisSensor].master == true) {
      pos[count] = locthisSensor;
      count++;
    }
  }
  // uint8_t pos[] = {0, 1, 2, 3, 4, 5, 6};
  // return the correct sensor for the current location
  location = location - 1 + rotation;

  if (location > count - 1) {
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
  return (doFaults(DIVISOR / 100, raw, int(steinhart), index));
}


int readBoost(uint8_t sensor, uint8_t index) {
  int rawval;
  float kpaval;
  float boost;
  rawval = analogRead(sensor);       // Read MAP sensor raw value on analog port 0
  kpaval = rawval * 0.4878;             // convert to kpa
  boost = kpaval * 0.145038 - 14.5038;  // Convert to psi and subtract atmospheric (sensor is absolute pressure)
  // process any faults
  return (doFaults(DIVISOR / 100, rawval, int(boost), index));
}

int readMAX(uint8_t index) {
  // Make sure you remove the delay(250) from the adafruit_MAX31856 readThermocoupleTemperature() - otherwise the screen rotation and refresh slows down to approx every 500ms
  int t = int(ReadTemperature(MAX_CS));
  //t = max.readThermocoupleTemperature();
  // process any faults
  return (doFaults(readFault(MAX_CS), 0, t, index));
}

int readPress(uint8_t sensor, uint8_t index) {
  //just a dummy at present
  int p;
  p = analogRead(sensor);
  // process any faults
  return (doFaults(DIVISOR / 100, p, p, index));
}

bool readCoolantLevel(uint8_t sensor, uint8_t index) {
  // sensor is normally closed
  // use a pulldown resistor to enable fault monitoring
  int CoolantLevel;
  CoolantLevel = analogRead(sensor);
  // process any faults
  return ((bool)doFaults(DIVISOR / 2, CoolantLevel, CoolantLevel, index));
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
  if (axis) {
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
  //Serial.print("X: "); Serial.print(event.magnetic.x); Serial.print("  ");
  //Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
  //Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  ");Serial.println("uT");

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
  if (heading < 0)
    heading += 2 * PI;

  // Check for wrap due to addition of declination.
  if (heading > 2 * PI)
    heading -= 2 * PI;

  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180 / M_PI;

  //Serial.print("Heading (degrees): "); Serial.println(headingDegrees);
  return (int(headingDegrees));
}

// MAX31856 SPI CODE


void InitializeChannel(int Pin) {
  for (int i = 0; i < NumRegisters; i++) {
    WriteRegister(Pin, i, RegisterValues[i]);
  }
}

uint8_t readFault(int Pin) {
  return ReadSingleRegister(Pin, 0x0F);
}

byte ReadSingleRegister(int Pin, byte Register) {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE3));
  digitalWrite(Pin, LOW);
  delayMicroseconds(1);
  SPI.transfer(Register & 0x7F); //set bit 7 to 0 to ensure a read command
  delayMicroseconds(1);
  byte data = SPI.transfer(0);
  digitalWrite(Pin, HIGH);
  SPI.endTransaction();
  return data;
}

unsigned long ReadMultipleRegisters(int Pin, byte StartRegister, int count) {
  //reads up to 4 sequential registers
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE3));
  digitalWrite(Pin, LOW);
  unsigned  long data = 0;
  SPI.transfer(StartRegister & 0x7F); //force bit 7 to 0 to ensure a read command
  delayMicroseconds(1);

  for (int i = 0; i < count; i++) {
    data = (data << 8) | SPI.transfer(0); //bitshift left 8 bits, then add the next register
  }
  digitalWrite(Pin, HIGH);
  SPI.endTransaction();
  return data;
}

void WriteRegister(int Pin, byte Register, byte Value) {
  byte Address = Register | 0x80; //Set bit 7 high for a write command
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE3));
  digitalWrite(Pin, LOW);
  delayMicroseconds(1);
  SPI.transfer(Address);
  delayMicroseconds(1);
  SPI.transfer(Value);
  digitalWrite(Pin, HIGH);
  SPI.endTransaction();
}

double ReadTemperature(int Pin) {
  double temperature;
  long data;
  data = ReadMultipleRegisters(Pin, 0x0C, 4);
  // Strip the unused bits and the Fault Status Register
  data = data >> 13;
  // Negative temperatures have been automagically handled by the shift above :-)
  // Convert to Celsius
  temperature = (double) data * 0.0078125;
  // Return the temperature
  return (temperature);
}
