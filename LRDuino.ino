// LRDuino by Ben Anderson
// Version 0.88

#include <Adafruit_SSD1306.h>
#include <Adafruit_MAX31856.h>
#include <FaBo3Axis_ADXL345.h>

// Following pinout details are for Ardunio Nano - be sure to adjust for your hadrware
#define OLED_RESET  13  //LED
#define OLED_CS     12
#define OLED_DC     11  //MISO DC
#define OLED_CLK    10  //D0
#define OLED_MOSI    9  //D1
#define OLED_CS_2    4
#define OLED_CS_3    3
#define MAX_CS       5  //Multiple software SPI because adafruit SSD1306 & MAX31856 libraries won't play nicely on the same bus
#define MAX_DC       2  //MISO DI
#define MAX_CLK      7  //CLK
#define MAX_MOSI     6  //DO 

Adafruit_MAX31856 max = Adafruit_MAX31856(MAX_CS, MAX_DC, MAX_MOSI, MAX_CLK); // seperate buses
//Adafruit_MAX31856 max = Adafruit_MAX31856(MAX_CS, OLED_DC, OLED_MOSI, OLED_CLK); // shared bus

Adafruit_SSD1306 display1(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
Adafruit_SSD1306 display2(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS_2);
Adafruit_SSD1306 display3(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS_3);

FaBo3Axis accel;

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// GFX deingitions for our icons - stored in flash memory

const unsigned char PROGMEM trbBMP [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x7F, 0xFF, 0xFF, 0x00, 0x7F, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0xE0,
  0x00, 0x00, 0x60, 0x60, 0x00, 0x38, 0x60, 0x70, 0x00, 0x0E, 0xE0, 0x30, 0x00, 0x0E, 0xE0, 0x30,
  0xF8, 0x2F, 0xFC, 0x38, 0xF8, 0xF8, 0x3C, 0x38, 0x19, 0xF8, 0x1C, 0x18, 0x18, 0x18, 0x10, 0x98,
  0x18, 0x08, 0x1F, 0x98, 0x18, 0x3C, 0x3F, 0x1F, 0x1C, 0x7E, 0xFC, 0x1F, 0x0C, 0x77, 0xF0, 0x00,
  0x0C, 0x46, 0x38, 0x00, 0x0C, 0x46, 0x1C, 0x00, 0x0E, 0x06, 0x00, 0x00, 0x07, 0x02, 0x00, 0x00,
  0x03, 0x80, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFE, 0x00, 0x7F, 0xFF, 0xFE,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM tboxBMP [] = {
  0x00, 0x07, 0xE0, 0x00, 0x01, 0xE7, 0xE7, 0x80, 0x01, 0xFF, 0xFF, 0x80, 0x03, 0xFF, 0xFF, 0xC0,
  0x03, 0xF0, 0x0F, 0xC0, 0x07, 0xE0, 0x07, 0xE0, 0x3F, 0x83, 0x03, 0xFC, 0x3F, 0x03, 0x00, 0xFC,
  0x3E, 0x03, 0xF0, 0x7C, 0x3E, 0x03, 0xF0, 0x7C, 0x1C, 0x03, 0x00, 0x38, 0x38, 0x03, 0x00, 0x18,
  0x38, 0x03, 0xF0, 0x1C, 0xF8, 0x03, 0xF0, 0x1F, 0xF8, 0x03, 0x00, 0x1F, 0xF8, 0x03, 0x00, 0x1F,
  0xF8, 0x03, 0xF0, 0x1F, 0xF8, 0x03, 0xF0, 0x1F, 0x38, 0x03, 0x00, 0x1C, 0x18, 0x07, 0x80, 0x18,
  0x3C, 0x0F, 0xC0, 0x3C, 0x3E, 0x0F, 0xC0, 0x7C, 0x3F, 0x07, 0x80, 0xFC, 0x3F, 0x83, 0x01, 0xFC,
  0x1F, 0xC0, 0x03, 0xF8, 0x07, 0xE0, 0x07, 0xC0, 0x01, 0xF8, 0x1F, 0x80, 0x01, 0xFF, 0xFF, 0xC0,
  0x00, 0xFF, 0xFF, 0x80, 0x00, 0xC7, 0xC3, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM egtBMP [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00,
  0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x7F, 0xC0, 0x00,
  0x00, 0x7F, 0xC0, 0x00, 0x00, 0x7F, 0xC0, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00,
  0x00, 0x78, 0x00, 0x00, 0x00, 0x7F, 0xC0, 0x00, 0x00, 0x7F, 0xC0, 0x00, 0x00, 0x7F, 0xC0, 0x00,
  0x00, 0x78, 0x01, 0xC0, 0x00, 0x78, 0x03, 0xE0, 0x00, 0x78, 0x03, 0xFC, 0xFE, 0x7F, 0xDF, 0xFE,
  0xFE, 0x7F, 0xDF, 0xFF, 0xFE, 0x7F, 0x9F, 0xFF, 0x00, 0x78, 0x0F, 0xF2, 0x00, 0x78, 0x07, 0xE0,
  0x00, 0xFC, 0x01, 0xC0, 0x01, 0xFE, 0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x01, 0xFE, 0x00, 0x00,
  0x01, 0xFE, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM eopBMP [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xFF, 0x00, 0x00, 0x7C, 0x7E, 0x00, 0x00,
  0xEE, 0x18, 0x00, 0x7C, 0xC7, 0xFF, 0x03, 0xFE, 0xFF, 0xFF, 0x9F, 0xC6, 0x3F, 0x01, 0xFF, 0x00,
  0x07, 0x00, 0xE6, 0x00, 0x03, 0x00, 0x06, 0x02, 0x03, 0x00, 0x0C, 0x07, 0x03, 0x00, 0x18, 0x0F,
  0x03, 0xFF, 0xF0, 0x0F, 0x01, 0xFF, 0xE0, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM eotBMP [] = {
  0x00, 0x01, 0x80, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00,
  0x00, 0x03, 0xFF, 0xC0, 0x00, 0x03, 0xFF, 0xC0, 0x00, 0x03, 0xFF, 0xC0, 0x00, 0x03, 0xC0, 0x00,
  0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xFF, 0xC0, 0x00, 0x03, 0xFF, 0xC0, 0x00, 0x03, 0xFF, 0xC0,
  0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xFF, 0xC0, 0x00, 0x03, 0xFF, 0xC0,
  0x00, 0x03, 0xFF, 0xC0, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x78, 0x07, 0xE0, 0x07,
  0xFE, 0x07, 0xE3, 0x1F, 0xFF, 0xCF, 0xF3, 0xFE, 0x07, 0xC7, 0xE3, 0xF0, 0x01, 0xC7, 0xE0, 0xE0,
  0x00, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x18, 0x18, 0x1C, 0x7E, 0x7E, 0x7E, 0x7E,
  0xFF, 0xFF, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0x81, 0x81, 0x81, 0x81, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM triBMP [] = {
  0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x7E, 0x00, 0x00, 0xFF, 0x00, 0x00,
  0xFF, 0x00, 0x01, 0xE7, 0x80, 0x01, 0xE7, 0x80, 0x03, 0xC3, 0xC0, 0x03, 0xC3, 0xC0, 0x07, 0x99,
  0xE0, 0x07, 0x99, 0xE0, 0x0F, 0x18, 0xF0, 0x0F, 0x18, 0xF0, 0x1E, 0x18, 0x78, 0x1E, 0x18, 0x78,
  0x3C, 0x00, 0x3C, 0x3C, 0x18, 0x3C, 0x78, 0x18, 0x1E, 0x78, 0x00, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFC,
};

const unsigned char PROGMEM NoConn [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x01, 0xFC, 0x00, 0x03, 0x8C, 0x00, 0x07, 0x06, 0x00,
  0x0E, 0x06, 0x00, 0x18, 0x0C, 0x00, 0x19, 0x1C, 0x00, 0x13, 0x38, 0x00, 0x0E, 0x70, 0x00, 0x0C,
  0xE0, 0x03, 0x9B, 0xC0, 0x07, 0x70, 0x00, 0x0C, 0x60, 0x00, 0x1C, 0xD0, 0x00, 0x39, 0x90, 0x00,
  0x70, 0x30, 0x00, 0x60, 0x70, 0x00, 0x60, 0xE0, 0x00, 0x61, 0xC0, 0x00, 0x33, 0x80, 0x00, 0x3F,
  0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

const unsigned char PROGMEM D2BMP [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x03, 0xFC, 0x00, 0x00, 0x3F, 0x86, 0x00,
  0x01, 0xF8, 0x03, 0x00, 0x0F, 0xC0, 0x03, 0x00, 0x3E, 0x00, 0x01, 0x80, 0x70, 0x00, 0x01, 0x80,
  0x60, 0x00, 0x00, 0x80, 0x60, 0x00, 0x00, 0xC0, 0x20, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00, 0x20,
  0x20, 0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x30, 0x20, 0x00, 0x00, 0x10, 0x20, 0x00, 0x00, 0x10,
  0x20, 0x00, 0x00, 0x10, 0x20, 0x00, 0x00, 0x10, 0x20, 0x00, 0x00, 0xF8, 0x30, 0x00, 0x07, 0xF8,
  0x30, 0x00, 0x78, 0x78, 0x10, 0x03, 0xC0, 0x3C, 0x18, 0x3C, 0x00, 0x3C, 0x09, 0xE0, 0x00, 0x3C,
  0x0F, 0x00, 0x00, 0x3C, 0x0F, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00,
  0x07, 0x80, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char PROGMEM coollev [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x18, 0x18, 0x1C,
  0x7E, 0x7E, 0x7E, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0x81, 0x81, 0x81, 0x81,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x07, 0xE0, 0x0F, 0xFC, 0x1F, 0xF8, 0x3F,
  0xFF, 0x7F, 0xFE, 0xFF, 0x0F, 0xF8, 0x1F, 0xF0, 0x03, 0xE0, 0x07, 0xC0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x18, 0x18, 0x1C, 0x7E, 0x7E, 0x7E, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF,
  0xC3, 0xC3, 0xC3, 0xC3, 0x81, 0x81, 0x81, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


// This is all the parameters and variables for our sensors - collumn 6 is a dummy for a boolean input (Engine Coolant Level)

//                                    Turbo       Tbox temp     EGT       Oil Press   Oil Temp  Cool Lev  Roll
bool warnhistatus[] =                 { false,    false,        false,    false,      false,    false,    false   }; // flag for alteranting warning animations
uint8_t sensefault[]=                 { 0,        0,            0,        0,          0,        0,        0       }; // flag set when sensor in error state.
const unsigned char* senseglyphs[] =  { trbBMP,   tboxBMP,      egtBMP,   eopBMP,     eotBMP,   coollev,   D2BMP  }; // pointers to our bitmaps
int sensevals[] =                     { 0,        0,            -20,      50,         101,      0,        0       }; // bogus start values for debug
const uint8_t senseunits[] =          { 1,        0,            0,        1,          0,        2,        3       }; // 0 for C 1 for psi 2 for "OK" 3 for "deg"
const int sensemaxvals[] =            { 32,       150,          900,      72,         150,      1,        45      }; // the maximum scale to use in our graph for each sensor
const int8_t senseminvals[] =         { 0,        -40,          -40,      0,          -40,      0,        -45     }; // the minimum scale to use in our graph for each sensor
int sensepeakvals[] =                 { 0,        -40,          -40,      0,          -40,      1,        0       }; // somewhere for us to store peak values
const int sensewarnhivals[] =         { 30,       140,          750,      60,         100,      999,      30      }; // values from RAVE that we want to warn of HIGH vals
//int sensewarnlowvals[] =            { -999,     -999,         -999,     9,          -999,     0,        0       }; // values from RAVE that we want to warn of LOW vals (set to outside range for no warning)

uint8_t rotation = 0; // incremented by 1 with each button press, resets to 0 after 5

// the follow variable is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long previousMillis = 0;        // will store last time the displays were updated
uint8_t interval = 250;         // interval at which to update displays(milliseconds)
//int atmos = 215;                //somewhere to store our startup atmospheric pressure - unused at present

void setup()   {
  //start serial connection
  //Serial.begin(9600);  //uncomment to send serial debug info
  
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)  
  display1.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, true);
  display2.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);
  display3.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, false);

  display1.clearDisplay();   // clears the screen and buffer
  display2.clearDisplay();   // clears the screen and buffer
  display3.clearDisplay();   // clears the screen and buffer

  //configure pin8 as an input and enable the internal pull-up resistor, this is for a button to control the rotation of sensors around the screen
  pinMode(8, INPUT_PULLUP);
  
  max.begin(); //initialise the MAX31856

  accel.configuration(); // initialise ADXL345
  accel.powerOn();

  max.setThermocoupleType(MAX31856_TCTYPE_K); // and set it to a K-type thermocouple - adjust for you hardware!
  
  // read our boost sensor rawADC value since at this point it should be atmospheric pressure...
  //atmos = readBoost(0,0);  // not actually used at this point so could be rmeoved

}

void loop() {

  unsigned long currentMillis = millis(); //store the time

  // USER INTERACTION
  bool butVal = digitalRead(8); // read the button state
  if (butVal == LOW) {
    rotation = rotation + 1; // rotate the screens if the button was pressed
    previousMillis = previousMillis - (interval + 1); // force an update of the screens.
    if (rotation == 7) {
      rotation = 0;
    }
  }


  if (currentMillis - previousMillis > interval) { // only read the sensors and draw the screen if 250 millis have passed
    // save the last time we updated
    previousMillis = currentMillis;

    // SENSOR READING

    sensevals[0] = readBoost(0,0); // read boost off A0 and store at index 0
    updatePEAK(0); // TURBO

    sensevals[1] = readERR2081(1,1); // read A1, currently the Gearbox oil temp sensor
    updatePEAK(1); // TBOX OIL TEMP

    sensevals[2] = readMAX(2); //read EGT from the Max31856
    updatePEAK(2); // EGT

    sensevals[3] = readPress(2,3); // placeholder at the moment but should be very similar to the boost reading if a cheap pressure sensor is used (ie one which returns a linear voltage 0-5v based on presure)
    updatePEAK(3); // OIL PRESSURE

    sensevals[4] = readERR2081(7,4); // read A7, store at index 4 currently the Engine oil temp sensor
    updatePEAK(4); // OIL TEMP

    sensevals[5] = readCoolantLevel(6,5); // read A6, to check if coolant level is low
    //updatePEAK(5); // Coolant Level - no need to set a max as this is boolean

    sensevals[6] = readADXL(6); // Inclinometer - Y (roll) angle
    updatePEAK(6);
  
    // DRAW DISPLAYS
    drawDISPLAY1();
    drawDISPLAY2();
    drawDISPLAY3();
  }
}

void drawDISPLAY1(void) { // DISPLAY 1 is our Main guage display
  // probably lots of room for improvement
  int padding = 0;
  float scalerange = 0;
  int scaleposmin = 0;
  uint8_t sensor0 = posrot(1);
  //uint8_t sensor0 = 6;

  drawSensor(0, display1, sensor0);

  display1.drawLine(11, 52, 11, 54, WHITE); // draw our gauge and scale markings
  display1.drawLine(64, 52, 64, 54, WHITE);
  display1.drawLine(116, 52, 116, 54, WHITE);
  display1.drawRect(11, 42, 106, 10, WHITE); //Border of the bar chart
  if (senseminvals[sensor0] < 0) { // Work out a positive range of values that we need to plot
    scaleposmin = senseminvals[sensor0] * -1;
    scalerange = scaleposmin + sensemaxvals[sensor0];
  } else {
    scaleposmin = senseminvals[sensor0];
    scalerange = sensemaxvals[sensor0] - scaleposmin;
  }
  display1.fillRect(14, 44, (100 / scalerange * (sensevals[sensor0] + scaleposmin)), 6, WHITE); //Draws the bar depending on the sensor value

  display1.drawLine(13 + (100 / scalerange * (sensepeakvals[sensor0] + scaleposmin)), 41, 13 + (100 / scalerange * (sensepeakvals[sensor0] + scaleposmin)), 50, WHITE); // draw the peak value line;
  if (sensevals[sensor0] < 100) { // adjust padding for the low value so it looks nice
    padding = 0;
  } else {
    padding = -4;
  }
  display1.setCursor(8 + padding + (100 / scalerange * (sensepeakvals[sensor0] + scaleposmin)), 33); // set cursor with padding
  display1.setTextSize(1);
  display1.println(String(sensepeakvals[sensor0])); // and write the peak val
  display1.setCursor(8, 57);
  display1.println(String(senseminvals[sensor0])); // draw the minumum value
  display1.setCursor(58, 57);
  if (senseminvals[sensor0] < 100) { // adjust padding for the low value so it looks nice
    padding = 8;
  } else {
    padding = 0;
  }
  display1.println(String(static_cast<int>(((sensemaxvals[sensor0] - senseminvals[sensor0]) / 2) - scaleposmin))); // draw the midpoint value
  if (sensemaxvals[sensor0] < 100) { // adjust padding for the high value so it looks nice/doesnt wrap off screen
    padding = 10;
  } else {
    padding = 5;
  }
  display1.setCursor(100 + padding, 57);
  display1.println(String(sensemaxvals[sensor0])); // draw the maximum value

  doWarnings(sensor0, 100, 4, display1);
 
  display1.display();
  display1.clearDisplay();
}



void drawDISPLAY2(void) { // DISPLAY 2 shows 2 sensors
  uint8_t sensor2 = posrot(2);
  uint8_t sensor5 = posrot(5);

  drawSensor(0, display2, sensor2);

  drawSensor(33, display2, sensor5);  

  display2.display();
  display2.clearDisplay();
}

void drawDISPLAY3(void) { // DISPLAY 3 shows 2 sensors
  uint8_t sensor3 = posrot(3);
  uint8_t sensor4 = posrot(4);

  drawSensor(0, display3, sensor3);

  drawSensor(33, display3, sensor4);  

  display3.display();
  display3.clearDisplay();
}

// Helper Functions

void drawSensor(uint8_t y, Adafruit_SSD1306 &refDisp, uint8_t sensor) {
  uint8_t xoffset=0;
  String temp;
  refDisp.setTextSize(2);
  refDisp.setTextColor(WHITE);
  refDisp.setTextWrap(false);
  refDisp.setCursor(46, y+9);
  refDisp.println(valIfnoErr(sensor));
  temp = valIfnoErr(sensor);
  xoffset = (temp.length() *12)+1 ; // work out width of the characters so we can move the cursor to the correct position to display our units symbol

  if (sensefault[sensor] > 0 || sensor == 5) { // normal size text if it's an error message or it's our low coolant warning sensor
      refDisp.setTextSize(2);
  }  else {
      refDisp.setTextSize(1);
  }
  refDisp.setCursor(46+xoffset, y+9);
  refDisp.println(units(sensor));
  refDisp.drawBitmap(0, y, senseglyphs[sensor], 32, 32, WHITE);

  // DO sensor warnings
  doWarnings(sensor, 100, y+4, refDisp);
}

void doWarnings(uint8_t sensorZ, uint8_t x, uint8_t y, Adafruit_SSD1306 &refDisp) {
  // this function draws an icon and co-ords x,y if there is an error state set
  
  if (highWARN(sensorZ)) {
    refDisp.drawBitmap(x, y, triBMP, 24, 24, WHITE); //outut the warning triangle
  }
  if (faultWARN(sensorZ)==1) {
    refDisp.drawBitmap(x, y, NoConn, 24, 24, WHITE); //output the disconnected sensor icon
  }
  // LOW VALUE WARNING
  //  if (sensevals[sensor0] < sensewarnlowvals[sensor0]) {
  //do soemthing
  //  }  
}


bool highWARN(uint8_t sensorZ) {
  // this function toggles a an error flag if the current sensor is above it's high warning parameter - since the display is redrawn every 250ms it appears to flash
  if (sensevals[sensorZ] > sensewarnhivals[sensorZ]) {
    if (warnhistatus[sensorZ] == true) {
      warnhistatus[sensorZ] = false; // we toggle the value so that the warning triangle flashes based on the interval we are looping at in loop()
    } else {
      warnhistatus[sensorZ] = true;
    }
  } else {
    warnhistatus[sensorZ] = false;
  }
  return (warnhistatus[sensorZ]);
}



uint8_t faultWARN(uint8_t sensorZ) {
  // this function alternates a flag between 1 & 2 if it is set - since the display is redrawn every 250ms it appears to flash
  if (sensefault[sensorZ] > 0) {
    if (sensefault[sensorZ] == 1) {
      sensefault[sensorZ] = 2; // we toggle the value so that the fault icon flashes based on the interval we are looping at in loop()
    } else {
      sensefault[sensorZ] = 1;
    }
  } 
  return (sensefault[sensorZ]);
}



void toggleFault(uint8_t sensorPin) {
  // toggles the fault strate of a sensor
  if (sensefault[sensorPin]==2) {
    sensefault[sensorPin]=2; // 2 is animation off
  } else {
    sensefault[sensorPin]=1; // 1 is animation on
  }
}



void updatePEAK(uint8_t sensorPin) {
  // stores the current value of a sensor if it is above the previously stored high value
  if (sensevals[sensorPin] >= senseminvals[sensorPin]) { // only do this if the value is above the min
    if (sensevals[sensorPin] <= sensemaxvals[sensorPin]) { // only do this if the value is below the max
      if (sensevals[sensorPin] > sensepeakvals[sensorPin]) {
        sensepeakvals[sensorPin] = int(sensevals[sensorPin]); //if we have a new max then store it
      }
    }
  }
}



String units(uint8_t sensor) { // returns the units associated with the sensor, or a some fault text
  // if a fault is set return ERR
  if (sensefault[sensor] > 0 && sensor !=5) {
    return(F("Er"));
  }  
  // if no fault then return the correct units (saves us some memory usage)

  switch(senseunits[sensor]) {
    case 0:
      return(F("C"));
    case 1: 
      return(F("psi"));
    case 2:
      if (sensefault[sensor] > 0) {
        return(F("LOW"));
      } 
      return(F("OK"));
    case 3:
      return(F("o"));
  }
}

String valIfnoErr(uint8_t sensor) { //prevents values being displayed is we are in fault state OR this is a bollean sensor (coolant level)
  String text = String(sensevals[sensor]);
  // if a fault is set return an empty string

  if (sensor == 5) {
    return(F(""));
  }  
  if (sensefault[sensor] > 0 || sensor == 5) {
    return(F(""));
  }  
    return(text);
}

uint8_t posrot(uint8_t location) { // this is used to shift our array of data around the screens
  uint8_t pos[] = {0, 1, 2, 3, 4, 5, 6};
  location = location - 1 + pos[rotation];
  if (location > 6) {
    location = location % 7;
  }
  return(location);
}

// Sensor reading code.

int readERR2081(uint8_t sensorPin, uint8_t index) {
  int raw = 0;           // variable to store the raw ADC input value
  float Vin = 3.3;           // variable to store the measured VCC voltage
  float Vout = 0;        // variable to store the output voltage
  int R2 = 1000;         // variable to store the R2 value
  float Rth = 0;          // variable to store the thermistor value

  // THERMISTOR CODE
  raw = analogRead(sensorPin);    // Reads the Input PIN
  Vout = (Vin / 1023.0) * raw;    // Calculates the Voltage on the Input PIN
  Rth = ((R2 * Vin) / Vout) - R2; //Calculates the Resistance of the Thermistor

  float steinhart;                 //This next stage calculates the temp from the resistance
  steinhart = Rth / 2012.2;     // (R/Ro)  therm @ 25C = 1986
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= 3502;                   // 1/B * ln(R/Ro) b coefficient = 3344
  steinhart += 1.0 / (25 + 273.15); // + (1/To) nominal temp is 25c
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  // END THERMISTOR CODE

  // FAULT checking
  // Sensors should be connected with a 1K pulldown resistor - if there's is a connection fault a low raw read will indicate this.
  if (raw <10) {
    toggleFault(index);
    sensepeakvals[index]=senseminvals[index];
    steinhart = senseminvals[index];
  } else {
    sensefault[index]=0; // no fault
  }
  return (int(steinhart)); // read the input pin for the temp sensor
}


int readBoost(uint8_t sensorPin, uint8_t index) {
  int rawval;
  float kpaval;
  float boost;
  rawval = analogRead(sensorPin); // Read MAP sensor raw value on analog port 0

  kpaval = rawval * 0.4878; // convert to kpa
  boost = kpaval * 0.145038 - 14.5038; // Convert to psi and subtract atmospheric (sensor is absolute pressure)
  if (rawval <10) {
    toggleFault(index);
    sensepeakvals[sensorPin]=senseminvals[index];
    boost = senseminvals[index];
  } else {
    sensefault[index]=0; // no fault
  }
  return (int(boost));
}

int readMAX(uint8_t index) {
  int t;
  t = max.readThermocoupleTemperature();
  // process any faults
  if (max.readFault()) {
    toggleFault(index);
    sensepeakvals[index]=senseminvals[index];
    t=senseminvals[index];
  } else {
    sensefault[index]=0; // no fault 
  }
  return (t);
}

int readPress(uint8_t sensorPin, uint8_t index) {
  //just a dummy at present
  int p;
  p = analogRead(sensorPin);

  // process any faults
  if (p <10) {
    toggleFault(index);
    sensepeakvals[index]=senseminvals[index];
    p=senseminvals[index];
  } else {
    sensefault[index]=0; // no fault 
  }
  return (p);
}

bool readCoolantLevel(uint8_t sensorPin, uint8_t index) {
  // sensor is normally closed
  // use a pulldown resistor to enable fault monitoring
  int CoolantLevel;
  CoolantLevel = analogRead(sensorPin);

  // process any faults
  if (CoolantLevel <512) { // if we're pulled low then either the sensor is signalling low coolant, or we have a continuity issue
    toggleFault(index);
    return (false);
  } 
  sensefault[index]=0; // no fault
  return (true);
}

int readADXL(uint8_t index) {
  int ax, ay, az;
  accel.readXYZ(&ax,&ay,&az);

  // we're only interested in one axis for vehicle roll
  
  //  double xAngle = atan( ax / (sqrt(square(ay) + square(az))));
  double yAngle = atan( ay / (sqrt(sq(ax) + sq(az))));
  //  double zAngle = atan( sqrt(square(ax) + square(ay)) / az);
  
  //  xAngle *= 180.00;   
  yAngle *= 180.00;   
  //  zAngle *= 180.00;
  //  xAngle /= 3.141592; 
  yAngle /= 3.141592; 
  //  zAngle /= 3.141592;

  //Serial.print("    yAngle: ");
  //Serial.println(yAngle);
  return(int(yAngle));
}

