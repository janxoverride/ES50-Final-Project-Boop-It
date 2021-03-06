//TODO
//play audio
//show graphics for each
//display score at end of every task
//add return zero

#include <Adafruit_BNO08x.h> // for Adafruit_BNO08x.h library (IMU)
#include <SD.h> // for SD card
#include <AudioZero.h> // for audio file playing
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // for OLED

// define for devices
#define BNO08X_RESET -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_BNO08x  bno08x(BNO08X_RESET); // Initialize the BNO08x object that we will use to read the IMU

// adafruit logo
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
//bitmap of adafruit logo, nice to see to verify OLED is working
static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

//define ports

#define audio A0
#define joyLR A1
#define envelope A2
#define potentiometer A3
#define photoResistor A4
#define joyUD A5

#define boop 3

//declare variables

bool inGame = 0;
int score = 0;
bool lost = 0;
int task = 0; // task number
bool intask = 0;
int interval = 5000; // timer interval in milliseconds
int counter = 0; // for losing screen

int photoThreshold = 200; /// JUST PLACEHOLDER FOR PHOITORESISTOR
int quietThreshold = 30; // for quiet task
int loudThreshold = 50; // for bark task
int joyThreshold = 100; ///joystick threshold for UD + LR need to test for treat task
float angSpeedThreshold = 10.0; //min rotational velocity for gyro IMU
bool lower = 0; // this is for potentiometer wiggle, means that we haven't hit the lower threshold for the pot
bool upper = 0;// higher threshold for pot has not been met yet
int upperBound = 800; // lower bound for potentiometer reading, ie knob all the way to left
int lowerBound = 200; // lower bound for potentiometer reading, ie knob all the way to left
int shakes = 0; //for shake command, checks how many times IMU has beeen shaken in x y or z axis
int displayScreen = 0; // variable for switch case for screen display
int startTime = 0; // for timer function
int quietStartTime = 0; //for the be quiet timer
int soundOnce = 0; // counter to only play sound once
int shakeThreshold = 30; // accelerometer value to trigger shake a paw

float eulerAngles[3]; //for roll, yaw and pitch of IMU
float eulerAngles2[3]; //created to check angular velocity
sh2_SensorValue_t sensorValue; // Create the variable that will store the incoming sensor data from the IMU

// BITMAPS FOR DISPLAY SCREEN
static const unsigned char PROGMEM losing[] =
{
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xe1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xe1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x0f, 0xff, 0xff, 0xc1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x03, 0xff, 0xff, 0x81, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfc, 0x00, 0xff, 0xcf, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x00, 0xff, 0xcf, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x00, 0x3c, 0x7f, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x00, 0x00, 0x7c, 0x78, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x00, 0x00, 0xf8, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x00, 0x00, 0xf0, 0x3f, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x0f, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x0f, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x00, 0x00, 0x08, 0x0f, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x01, 0xfc, 0x0c, 0x0f, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x00, 0xfc, 0x1e, 0x07, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x01, 0xe0, 0x3f, 0x07, 0x06, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfc, 0x07, 0xe0, 0x7f, 0x87, 0x06, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x0f, 0xe1, 0xff, 0xff, 0x07, 0xff, 0xf3, 0xe4, 0xe6, 0x09, 0xcf, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x0f, 0xe3, 0x03, 0xff, 0x87, 0xff, 0xf1, 0xc4, 0xe4, 0x09, 0xcf, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x1f, 0xf2, 0x03, 0xff, 0x87, 0xff, 0xf0, 0x84, 0xe4, 0xf9, 0xcf, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x3f, 0xfa, 0x02, 0x7f, 0xff, 0xff, 0xf2, 0x24, 0xe4, 0xf8, 0x0f, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x3f, 0xff, 0x00, 0x7f, 0xff, 0xff, 0xf3, 0x64, 0xe4, 0xf8, 0x0f, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x3f, 0xff, 0x81, 0xff, 0xff, 0xcf, 0xf3, 0xe4, 0xe4, 0xf9, 0xcf, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xf3, 0xe4, 0x04, 0x09, 0xcf, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xc3, 0xf3, 0xe6, 0x0e, 0x09, 0xcf, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x73, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0x00, 0x73, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0x00, 0x73, 0xf3, 0xf8, 0x1c, 0x1c, 0x0f, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0x00, 0x73, 0xf3, 0xf0, 0x08, 0x08, 0x0f, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0xff, 0x00, 0x71, 0xf3, 0xf3, 0xc9, 0xf9, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0x00, 0x71, 0xf3, 0xf3, 0xc8, 0x08, 0x3f, 0xff, 0xff, 
0xff, 0xff, 0xe0, 0x3f, 0xff, 0xff, 0xff, 0x00, 0xf1, 0xf3, 0xf3, 0xc8, 0x08, 0x3f, 0xff, 0xff, 
0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xff, 0x01, 0xf1, 0xf3, 0xf3, 0xcf, 0xc9, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xfe, 0x03, 0xf3, 0xf0, 0x10, 0x08, 0x08, 0x0f, 0xff, 0xff, 
0xff, 0xff, 0xf0, 0x3f, 0xff, 0xfe, 0x3c, 0x00, 0x73, 0xf0, 0x18, 0x1c, 0x1c, 0x0f, 0xff, 0xff, 
0xff, 0xff, 0xe0, 0x0f, 0xff, 0xfe, 0x00, 0x00, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xc0, 0x0f, 0xff, 0xfe, 0x00, 0x01, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xe0, 0x03, 0xc3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xf0, 0x07, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xe0, 0x00, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xf0, 0x00, 0x1f, 0xff, 0xff, 0xc1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x00, 0x1f, 0xff, 0xff, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static const unsigned char PROGMEM winning[] =
{
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xfc, 0xff, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xe7, 0xe7, 0xff, 0xfc, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xef, 0xe3, 0xff, 0xf8, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xdf, 0xe0, 0xff, 0xf8, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xbf, 0xc0, 0x7f, 0x70, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xbf, 0xc0, 0x39, 0xfe, 0x7f, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0x7f, 0xc0, 0x03, 0xc7, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0x7f, 0xc0, 0x03, 0x87, 0xe7, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0x7f, 0xe0, 0x00, 0x03, 0xf7, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0xff, 0xe0, 0x00, 0x03, 0xfb, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0xff, 0xe0, 0x78, 0x43, 0xf9, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0xff, 0xe0, 0x78, 0xf1, 0xfd, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfd, 0xff, 0xe1, 0xe1, 0xf1, 0x8d, 0xff, 0xbd, 0xd0, 0x8d, 0xdf, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfd, 0xff, 0xc3, 0xe7, 0xff, 0x8f, 0xff, 0xbd, 0xd7, 0xb5, 0xdf, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfd, 0xff, 0xc3, 0xf4, 0x3f, 0xcf, 0xff, 0xbe, 0xb1, 0x8e, 0x3f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfd, 0xff, 0xc7, 0xfc, 0x2f, 0xff, 0xff, 0xbe, 0xb7, 0xaf, 0x7f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfd, 0xff, 0xc7, 0xfe, 0x1f, 0xff, 0x7f, 0xbf, 0x70, 0xb7, 0x7f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfd, 0xff, 0xcf, 0xff, 0xff, 0xff, 0x3f, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xc3, 0x3f, 0xbf, 0x74, 0x5b, 0x7f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0x81, 0xbf, 0xbf, 0x76, 0xcb, 0x7f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfd, 0xff, 0xef, 0xff, 0xff, 0x81, 0xbf, 0xbf, 0x56, 0xd3, 0x7f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfd, 0xff, 0xcf, 0xff, 0xff, 0x81, 0xbf, 0xbf, 0x26, 0xdb, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfd, 0xff, 0xc7, 0xff, 0xff, 0x81, 0x9f, 0xbf, 0x74, 0x5b, 0x7f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0xff, 0x87, 0xff, 0xff, 0x83, 0x9f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0xff, 0x07, 0xff, 0xff, 0x87, 0xbf, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0xff, 0x87, 0xff, 0xe7, 0x01, 0xbf, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0x7f, 0x03, 0xff, 0xe0, 0x03, 0x3e, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0x7f, 0x03, 0xff, 0xfc, 0x07, 0x3e, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0x7f, 0x01, 0xff, 0xff, 0xff, 0x7e, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xbf, 0x00, 0xff, 0xff, 0xfe, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xdf, 0x00, 0xff, 0xff, 0xfc, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xef, 0x80, 0x7f, 0xff, 0xf3, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xef, 0xe0, 0x1f, 0xff, 0xc7, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xf7, 0xff, 0x1f, 0xff, 0xdf, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static const unsigned char PROGMEM loading[] =
{
// 'pixil-frame-0', 128x64px
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x03, 0xcf, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xcc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xcf, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xce, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xcf, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xce, 0x7c, 0x3e, 0x1f, 0x0f, 0xff, 0x9f, 0x03, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xc0, 0x78, 0x1c, 0x0e, 0x07, 0xff, 0x9f, 0x03, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xc0, 0xf1, 0x88, 0xc6, 0x73, 0xff, 0x9f, 0xcf, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xce, 0x73, 0xc9, 0xe6, 0x73, 0xff, 0x9f, 0xcf, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xcf, 0x33, 0xc9, 0xe6, 0x73, 0xff, 0x9f, 0xcf, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xcf, 0x33, 0xc9, 0xe6, 0x73, 0xff, 0x9f, 0xcf, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xcf, 0x31, 0x88, 0xc6, 0x77, 0xff, 0x9f, 0xcf, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xc0, 0x78, 0x1c, 0x0e, 0x07, 0xf8, 0x01, 0xcf, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xc0, 0xfc, 0x3e, 0x1e, 0x0f, 0xf8, 0x01, 0xcf, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x04, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x06, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x03, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x03, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x03, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x03, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x06, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x0c, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xfc, 0x0f, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xc7, 0xff, 0xfe, 0x1f, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xff, 0xfe, 0x1f, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xff, 0xfe, 0x1f, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xff, 0xfc, 0x0f, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xf8, 0x07, 0xff, 0xc1, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x7f, 0xf0, 0x03, 0xff, 0x83, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x3f, 0xe0, 0x01, 0xff, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xc1, 0xe0, 0xfe, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x07, 0x03, 0xf0, 0x38, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x07, 0xf8, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x0f, 0xfc, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x3f, 0xff, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};


void setReports(void) { // for the accelerometer on IMU
  Serial.println("Setting desired reports");
  if (! bno08x.enableReport(SH2_GAME_ROTATION_VECTOR)) {// for pitch, yaw roll
    Serial.println("Could not enable game vector");
  }
  if (!bno08x.enableReport(SH2_SHAKE_DETECTOR)) { // shake detector function given in library
    Serial.println("Could not enable shake detector");
  }
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) { // accelerometer
    Serial.println("Could not enable accelerometer");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // FOR BNO08X CHIP
  Serial.println("Adafruit BNO08x test!");

  // Attempt to initialize the BNO085, if it fails enter an infinite loop
  if (!bno08x.begin_I2C()) {
    Serial.println("Failed to find BNO08x chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("BNO08x Found!");

  // See definition of setReports() below
  setReports();

  Serial.println("Reading events");
  delay(100);


  // setup SD-card
  Serial.print("Initializing SD card...");
  if (!SD.begin(SDCARD_SS_PIN)) { // must change to this port for MKRZero
    Serial.println(" failed!");
    while(true);
  }
  Serial.println(" done.");



  // FOR SSD1306 OLED SCREEN

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500); // Pause for 0.5 s

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(1000);

}


void timer(float timeInterval, int startTime) {
  int startScore = score; // make sure we are on the same task still
  if(startScore == score && intask == 1 && (millis()-startTime) > timeInterval){ // checks if we exceeded time limit
    intask = 0; //exit out of task and game
    inGame = 0;
    lost = 1;
    counter = 0;
  }
}

int boopIt() {
  if(digitalRead(boop) == 0){
    return 1;
  }
  else{
    return 0;
  }
}

int petIt() {
  if(analogRead(photoResistor) > photoThreshold){
    return 1;
    }
  else{
    return 0;
  }
}

int bark(){
  if(analogRead(envelope) > loudThreshold){
    return 1;
  }
  else{
    return 0;
  }
}


int quiet(){/////ADD TIME REQUIREMENMT LATER
  if((millis() - quietStartTime) > 1200){ //need to be quiet for 1.2 second
    return 1;
  }
  else{
    return 0;
  }
  if(analogRead(envelope) > quietThreshold){ // restart when we pass quiet threshold
    quietStartTime = millis();
  }
}


int checkLower(){ // checks if potentiometer is turned all the way to left
  int pos = analogRead(potentiometer);
  if(pos < lowerBound){
    return 1;
  }
  else{
    return 0;
  }
}

int checkUpper(){ // checks if potiometer is turned all the way to the right
  int pos = analogRead(potentiometer);
  if(pos > upperBound){
    return 1;
  }
  else{
    return 0;
  }
}

int treatIt(){ // checks if the joystick has been moved either LR or UD or both
  int LR = analogRead(joyLR);
  int UD = analogRead(joyUD);
  if((abs(LR-500) + abs(UD-500)) > joyThreshold){
    return 1;
  }
  else{
    return 0;
  }
}

void toEuler(float * angles, sh2_SensorValue_t quat) { //helper function from IMU lab, gives angle of rotation of gyro
  float w = quat.un.gameRotationVector.real;
  float x = quat.un.gameRotationVector.i;
  float y = quat.un.gameRotationVector.j;
  float z = quat.un.gameRotationVector.k;
  float rawAngles[3] = {0, 0, 0}; // angles in radians

  float sqw = w * w;
  float sqx = x * x;
  float sqy = y * y;
  float sqz = z * z;

  rawAngles[2] = atan2(2.0 * (x*y + z*w), (sqx - sqy - sqz + sqw));
  rawAngles[1] = asin(-2.0 * (x*z - y*w) / (sqx + sqy + sqz + sqw));
  rawAngles[0] = atan2(2.0 * (y*z + x*w), (-sqx - sqy + sqz + sqw));
  
  for (int i = 0; i < 3; i++) {
    angles[i] = rawAngles[i] * (180.0/PI); // convert from radians to degrees
  }
}

int shake(){ //using shake detector from bno08x library

  int lastShakes = shakes;
  if (bno08x.wasReset()) { //checks if sensor was reset
    Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue)) { //checks if we are getting sensor values
    // return;
    Serial.print("sensor error");
  }

  if(sensorValue.un.accelerometer.x > shakeThreshold || sensorValue.un.accelerometer.y > shakeThreshold || sensorValue.un.accelerometer.z > shakeThreshold){
    return 1;
  }
  
 /* sh2_ShakeDetector_t detection = sensorValue.un.shakeDetector;
  
    switch (detection.shake) { // if shake was detected in x or y or z then add to shakes
    case SHAKE_X:
      shakes ++;
      break;
    case SHAKE_Y:
      shakes ++;
      break;
    case SHAKE_Z:
      shakes ++;
      break;
    default:
      break;
      }

  if (shakes != lastShakes){
    return 1;
  }
  */
  else{
    return 0;
  }
  
      
}

int encoderMoved(){
  int lastState = analogRead(potentiometer);
  if(abs(analogRead(potentiometer)-lastState) > 50){
    return 1;
  }
  else{
    return 0;
  }
}

//********************************************************************************************************************************************************************************

void loop() {
  // put your main code here, to run repeatedly:
  
  //OLED Presets
  display.clearDisplay(); //clear so we don't draw over the same text
  display.setTextSize(1); //set text size to 1x
  display.setTextColor(SSD1306_WHITE); //text is white
  display.setCursor(10, 0); //cursor near top left
  delay(10);

  if(inGame == 0 && lost == 0){ //**************************************start screen mode ADD DISPLAY PROMPT
    score = 0; //reset score
    //display starting screen here
    display.drawBitmap(0, 0, loading, 128, 64, WHITE); // start screen
    display.display();    
    
    if(soundOnce == 0){
       AudioZero.begin(88200);
       AudioZero.play(SD.open("loading.wav"));
       AudioZero.end();
       soundOnce ++;
      }
    if(boopIt() == 1){
      interval = 5000; //5 seconds for task in the beginning
      inGame = 1;
      intask = 0;
      randomSeed(millis()); // to get random generated, we will use time taken for user to press start as basis
    }
  }

  else if(inGame == 1 && lost == 0){ //**************************** in game mode
    
    if(score > 5){ // timer limit gets shorter when score hits 5
      interval = 3000;
    }
    else if(score > 9){ // timer gets shorter when score hits 9
      interval = 1500;
    }
    else if (score > 14){ // level 3 hardness
      interval = 1000;
    }


    if(intask == 0){ // assign a random task
      display.clearDisplay();

      display.setTextSize(1);
      display.setCursor(38, 15);
      display.println(F("YOUR SCORE"));
      display.setCursor(60, 36);
      display.setTextSize(2);
      display.println(score);
      display.display();
      delay(1000);
      task = random(1,8); // 1 in included, 8 is excluded, so we have 7 tasks
      upper = 0;//resets all ingaFcounme task booleans to default
      lower = 0; //for wiggle command
      shakes = 0; // for shake command
      startTime = millis(); // for timer start time
      intask = 1;
      quietStartTime = millis(); // for be quiet task
      soundOnce = 0; // counter for the command sound to only play once
    }
    
    else if(task == 1 && intask == 1){ //BOOP IT TASK
      display.setCursor(45, 28); //center text
      display.println(F("BOOP IT")); // display command
      display.display();
      if(soundOnce == 0){
       AudioZero.begin(88200);
       AudioZero.play(SD.open("boop.wav"));
       AudioZero.end();
       soundOnce ++;
      }
      timer(interval, startTime);
      if(boopIt() == 1){
        score ++;
        intask = 0;   
      }
    }
    else if(task == 2 && intask == 1){// PET IT TASK
      display.setCursor(46, 28);
      display.println(F("PET IT"));
      display.display();
      if(soundOnce == 0){
       AudioZero.begin(88200);
       AudioZero.play(SD.open("pet.wav"));
       AudioZero.end();
       soundOnce ++;
      }
      timer(interval, startTime);
      if(petIt() == 1){
        score ++;
        intask = 0;
      }
      else if(boopIt() == 1 || encoderMoved() == 1){ //////////////////////////////loss case, add to all cases************************((((((((((((&(*(*(*(*(*
        intask = 0;
        inGame = 0;
        lost = 1; 
      }
    }
    else if(task == 3 && intask == 1){ //BARK AT IT TASK
      display.setCursor(32, 28);
      display.println(F("BARK AT IT"));
      display.println(analogRead(envelope));
      display.display();
      if(soundOnce == 0){
       AudioZero.begin(88200);
       AudioZero.play(SD.open("bark.wav"));
       AudioZero.end();
       soundOnce ++;
      }
      timer(interval, startTime);
      if(bark() == 1){
        score ++;
        intask = 0;
      }
    }
    else if(task == 4 && intask == 1){ // BE QUIET TASK
      display.setCursor(32, 28);
      display.println(F("BE QUIET"));
      display.println(analogRead(envelope));
      display.display();
      if(soundOnce == 0){
       AudioZero.begin(88200);
       AudioZero.play(SD.open("dead.wav"));
       AudioZero.end();
       soundOnce ++;
      }
      timer(interval, startTime);
      if(quiet() == 1){
        score ++;
        intask = 0;
      }
    }
    else if(task == 5 && intask == 1){ // WIGGLE TASK
      display.setCursor(40, 28);
      display.println(F("WIGGLE"));
      display.display();
      if(soundOnce == 0){
       AudioZero.begin(88200);
       AudioZero.play(SD.open("wiggle.wav"));
       AudioZero.end();
       soundOnce ++;
      }
      timer(interval, startTime);
      if(checkLower() == 1){
        lower = 1;
      }
      if(checkUpper() == 1){
        upper = 1;
      }
      if(lower == 1 && upper == 1){
        score ++;
        intask = 0;
      }
    }
    else if(task == 6 && intask == 1){////SHAKE A PAW TASK
      display.setCursor(35, 28);
      display.println(F("SHAKE A PAW")); 
      display.println(shakes);    
      display.display();
      if(soundOnce == 0){
       AudioZero.begin(88200);
       AudioZero.play(SD.open("shake.wav"));
       AudioZero.end();
       soundOnce ++;
      }
      timer(interval, startTime);
      if(shake() == 1){
        score ++;
        intask = 0;
      }
    }
    else if(task == 7 && intask == 1){////TREAT IT TASK
      display.setCursor(38, 28);
      display.println(F("TREAT IT"));
      display.display();
      if(soundOnce == 0){
       AudioZero.begin(88200);
       AudioZero.play(SD.open("treat.wav"));
       AudioZero.end();
       soundOnce ++;
      }
      timer(interval, startTime);
      if(treatIt() == 1){
        score ++;
        intask = 0;
      }
    }
    
  }
  else if(inGame == 0 && lost == 1){ /// LOST GAME SCREEN

    if(counter == 0){
      display.drawBitmap(0, 0, losing, 128, 64, WHITE);
      display.display();
      AudioZero.begin(88200);
      AudioZero.play(SD.open("losing.wav"));
      AudioZero.end();
      //delay(2000);
      display.clearDisplay();
      counter++;
    }
    display.setTextSize(1);
    display.setCursor(38, 5);
    display.println(F("FINAL SCORE"));
    display.setCursor(60, 21);
    display.setTextSize(2.3);
    display.println(score);
    display.setTextSize(1); //set text size to 1x
    display.setCursor(40, 38);
    display.println(F("---------"));
    display.setCursor(23, 50);
    display.println(F("BOOP TO RESTART"));
    display.display();
    
    if(boopIt() == 1){
      lost = 0;
      soundOnce = 0;
    }
  }

  display.display(); // display onto screen
} 
