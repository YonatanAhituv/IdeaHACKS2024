/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Arduino.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <ArduinoJson.h>

const char* ssid = "ZENBOOK 7206";
const char* password = "T508?s52";

String serverName = "192.168.137.222";   // REPLACE WITH YOUR Raspberry Pi IP ADDRESS

String serverPath = "/push_sensors";     // The default serverPath should be upload.php

const int serverPort = 5000;

WiFiClient client;



#define BUTTON 34  // must be an analog pin, use "An" notation!


Adafruit_MPU6050 mpu;
// For the Adafruit shield, these are the default.
#define TFT_DC 4
#define TFT_CS 5

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);


float xval[100] = {0};
float yval[100] = {0};
float zval[100] = {0};
float xavg;
float yavg;
float zavg;
float threshold = 2.0;
  
int steps = 0;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(9600);
  Wire.begin();
  pinMode(BUTTON, INPUT);
  Serial.println("ILI9341 Test!"); 
 
  tft.begin();

  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
  
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  printData();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}


void loop(void) {
  updateSteps();
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (digitalRead(BUTTON) == HIGH) {
     //Serial.println(digitalRead(BUTTON));
     sendData();
     printData();

  } else {
    //Serial.println(digitalRead(BUTTON));
  }
  delay(100);
  
}

void updateSteps() {
   sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float totvect[100]={0};
  float totave[100]={0};
  float xaccl[100]={0};
   float yaccl[100]={0};
    float zaccl[100]={0};
   
  int stepsToAdd = 0;
for (int i=1;i<100;i++)
{
  xaccl[i]=a.acceleration.x;
  delay(1);
  yaccl[i]=a.acceleration.y;
  delay(1);
  zaccl[i]=a.acceleration.z;
  delay(1);
  totvect[i] = sqrt(((xaccl[i]-xavg)* (xaccl[i]-xavg))+ ((yaccl[i] - yavg)*(yaccl[i] - yavg)) + ((zval[i] - zavg)*(zval[i] - zavg)));
  totave[i] = (totvect[i] + totvect[i-1]) / 2 ;

//cal steps 
if (totave[i]>threshold)
{
  stepsToAdd++;
}
}
stepsToAdd /= 50;
steps += stepsToAdd;
}

void sendData() {
  String getAll;
  String getBody;
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  //String fb = "{\"steps\":"+String(steps)+"\n\"temperature\":"+ String(temp.temperature) + "\n}";
  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");    
    StaticJsonBuffer<200> jsonBuffer;
  
  // Build your own object tree in memory to store the data you want to send in the request
  JsonObject& root = jsonBuffer.createObject();
  root["steps"] = steps;
  root["temperature"] = temp.temperature;
  
  // Generate the JSON string
  root.printTo(Serial);
  
  Serial.print("POST ");
  Serial.println(serverPath);

  client.print("POST ");
  client.print(serverPath);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(WiFi.localIP());
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.print(root.measureLength());
  client.print("\r\n");
  client.println();
  root.printTo(client);

    // String head = "Content-Type: application/json\r\nAccept: application/json\r\n\r\n";

    // uint32_t jsonLength = fb.length();
    // uint32_t extraLen = head.length() + tail.length();
    // uint32_t totalLen = jsonLength + extraLen;
  
    // client.println("POST " + serverPath + " HTTP/1.1");
    // client.println("Host: " + serverName);
    // client.print(head);
    // client.println("Content-Length: " + String(totalLen));
    // client.println(fb);
    
    
    int timoutTimer = 10000;
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + timoutTimer) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (getAll.length()==0) { state=true; }
          getAll = "";
        }
        else if (c != '\r') { getAll += String(c); }
        if (state==true) { getBody += String(c); }
        startTimer = millis();
      }
      if (getBody.length()>0) { break; }
    }
    Serial.println();
    client.stop();
    Serial.println(getBody);
  }
  else {
    getBody = "Connection to " + serverName +  " failed.";
    Serial.println(getBody);
  }
}

void printData() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  
  tft.setTextSize(2);
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values */
  tft.print("Acceleration X: ");
  tft.print(a.acceleration.x);
  tft.print(", Y: ");
  tft.print(a.acceleration.y);
  tft.print(", Z: ");
  tft.print(a.acceleration.z);
  tft.println(" m/s^2");

  tft.print("Rotation X: ");
  tft.print(g.gyro.x);
  tft.print(", Y: ");
  tft.print(g.gyro.y);
  tft.print(", Z: ");
  tft.print(g.gyro.z);
  tft.println(" rad/s");

  tft.print("Temperature: ");
  tft.print(temp.temperature);
  tft.println(" degC");

   tft.print("Steps: ");
  tft.print(steps);
  
  tft.println("");
}
