
#include <TFT_eSPI.h>  //Graphics library for display
// #include <SPI.h>
#include <Adafruit_AHTX0.h>
// #include <Wire.h>
#include "Free_Fonts.h"
#include <SPI.h>
#include <Wire.h>
// #include <WiFi.h>
#include <WiFiManager.h> 
#include <FirebaseESP32.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include "secrets.h"
#include <WiFiClientSecure.h>

//TFT Declarations

TFT_eSPI tft = TFT_eSPI();                   //tft
TFT_eSprite background = TFT_eSprite(&tft);  //Main sprite
TFT_eSprite tongue = TFT_eSprite(&tft);      //Tongue Sprite
TFT_eSprite eyebrow = TFT_eSprite(&tft);
TFT_eSprite drop = TFT_eSprite(&tft);

//////////
//IMAGES//
//////////

//Mouths
#include "Smile.h"
#include "PantMouth.h"
#include "Tongue.h"
#include "bigSmile.h"
#include "barfmouth1.h"
#include "Frown.h"
#include "Shiver.h"
#include "SmallFrown.h"
#include "tiredmouth.h"

//Eyes
#include "tiredeye.h"
#include "Sunglasses.h"
#include "Blink.h"
#include "wink.h"
#include "squint.h"
#include "eyebrowsadl.h"
#include "eyebrowsadr.h"
#include "Eye.h"

//Other
#include "Sun.h"
#include "Thermometer.h"
#include "Drop.h"
#include "Ice.h"

#define WIFI_SSID "Virus"
#define WIFI_PASSWORD "12345678"

#define FIREBASE_HOST "espledonoff-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "ZnIORSB3cOXMVmPFZiQdBkSMsNgwhNhsvaKhM0QU"

const char* mqtt_server = "broker.emqx.io";

#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client1(net);

////////////////////
//Global Variables//
////////////////////
WiFiClient espClient;
PubSubClient client(espClient);
bool deviceregistration=false,registration_status=false,display_screen=false;
String comment;
int xaxis=0,yaxis=0;
String macAddress;
String a="new";
String madd,mac;
char cbuff[18];
String result;
String checkValue = "11:22:12:444:444";
bool matchFound = false;

String jsonData;
DynamicJsonDocument doc(1024);
DynamicJsonDocument doc1(1024);

DeserializationError error = deserializeJson(doc, jsonData);
String firebasePath;

FirebaseData firebaseData;
FirebaseJson json;

unsigned long currentTime = millis();  //Reads current time
unsigned long prevTime = 0;            //Previous time button was pressed

Ticker timer;
Ticker timer1;
bool publishFlag = false;
bool mqttflag = false;
bool pollflag=false;

// unsigned long currentTime;
// int lastCount;
void publishCallback();
void mqttCallback();
void publish_on_app();
void reconnect();
void displayonscreen(String comment,int xaxis,int yaxis);
void callback(char* topic, byte* message, unsigned int length) ;
///////////
//BUTTONS//
///////////

//Button pins
const byte buttonPins[] = { 27, 32 };
int buttonPin = 25;

int lastStates[] = { HIGH, HIGH };  //Last/previous state of the button HIGH/Off or LOW/On
int increments[] = { -1, 1 };       //Increments button counter, left button subtracts, right button adds one

unsigned long lastClicks[] = { 0, 0 };  //Last/previous click of one of the buttons

int bounceDelay = 50;  //Delay in miliseconds for debouncing the button

int count = 1;  //Variable to hold the count of the button

const int numButtons = 2;  //Number of buttons

///////////////////
//MOISTURE SENSOR//
///////////////////

const int soilPin = 34;  //Pin for the capacitve soil moisture sensor

const int numReadings = 200;  //Number of readings it takes and then averages

int readings[numReadings];  //Stores sensor readings
int readIndex = 0;          //The index number of current reading
int total = 0;              //The total readings
int average = 0;            //Average number of the readings

int MaxS = 3550;  //Estimated maxium sensor reading based on air value
int MinS = 1650;  //Estimated minuim sensor reading based on water value

int moistSoilMin;  //Minium range of well watered soil
int moistSoilMax;  //Maximum range of well watered soil

int tooWet = 80;  //Value that the soil becomes to wet

int moistPercent;  //Percentage of moister in soil 0-100%
int prevMoistPercent;

unsigned long prevSoilRead = 0;  //Previous time of soil read
int waitSoil = 5;                //Time between soil readings to debounce
int moisture;                    //Holds mapping value 0-8

////////////////
//LIGHT SENSOR//
////////////////

const int ANALOG_READ_PIN = 35;  // or A0
const int RESOLUTION = 12;       // Could be 9-12

const int lightPin = 35;              //Pin for the capacitve light light sensor
const int numlightReadings = 100;     //Number of lightReadings it takes and then averageLights
int lightReadings[numlightReadings];  //Stores sensor lightReadings
int readLightIndex = 0;               //The index number of current reading
int totalLight = 0;                   //The total lightReadings
int averageLight = 0;                 //averageLight number of the lightReadings

int MaxL = 4095;  //Estimated maxium sensor reading based on air value
int MinL = 400;   //Estimated minuim sensor reading based on water value

int lightPercent;  //Percentage of lighter in light 0-100%

unsigned long prevlightRead = 0;  //Previous time of light read
int waitlight = 5;                //Time between light lightReadings to debounce
int light,i=0;                        //Holds mapping value 0-8

////////////////////////
//TEMP/HUMIDITY SENSOR//
////////////////////////

Adafruit_AHTX0 aht;

int fahrenheit;
int humid;
void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
    WiFiManager wm;
    bool res;
    displayonscreen("Connecting to Wifi",0,1);

    res = wm.autoConnect("SproutSitter","12345678"); // password protected ap
    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        displayonscreen("Connected to WiFi",0,1);

    }

    Serial.println("Connected to WiFi");
    displayonscreen("Connected to WiFi",0,1);
    Serial.println(WiFi.macAddress());
    macAddress = WiFi.macAddress();
    result=a+macAddress.substring(0,10);

    displayonscreen("Connecting to MQTT",0,1);
    Serial.println("Connecting to MQTT");
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    Serial.println("MQTT Connected");
    displayonscreen("Connected to MQTT",0,1);
    connectAWS();
    // client.publish("test","message");

    timer.attach(60, publishCallback); // 600 seconds = 10 minutes
    timer1.attach(5, mqttCallback);
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Serial.printf("Get string... %s\n", Firebase.getString(firebaseData, F("/Devices/macaddress/")) ? firebaseData.to<const char *>() : firebaseData.errorReason().c_str());
    jsonData=Firebase.getString(firebaseData, F("/Devices/macaddress")) ? firebaseData.to<const char *>() : firebaseData.errorReason().c_str();
    DeserializationError error = deserializeJson(doc, jsonData);

  // Check if there was an error in parsing the JSON
    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return;
    }

    for (JsonPair kv : doc.as<JsonObject>()) {
        String key = kv.key().c_str();
        String value = kv.value().as<String>();
        i=i+1;
        Serial.print("Key: ");
        Serial.print(key);
        Serial.print(" Value: ");
        Serial.println(value);
        // devices[i]=value;
        if (value == macAddress) {
        Serial.print("Match found! Key: ");
        Serial.println(key);
        matchFound = true;
        displayonscreen("Device already registered",0,1);
        deviceregistration=true;
        registration_status=true;
        break;
        }
        String modifiedJsonData;
        serializeJson(doc, modifiedJsonData);
        Serial.println("Modified JSON data:");
        Serial.println(modifiedJsonData);

    }


  // aht.begin();
  ////  TFT display set up

  // tft.init();
  // tft.setRotation(1);
  // tft.fillScreen(TFT_WHITE);

  //Background sprite setup, main sprite
  background.setColorDepth(8);
  background.createSprite(240, 240);
  background.setSwapBytes(true);
  background.fillSprite(TFT_WHITE);

  //Tongue Sprite Setup
  tongue.createSprite(54, 68);
  tongue.setSwapBytes(true);
  tongue.setPivot(27, 68);

  //Eyebrow sprite setup
  eyebrow.createSprite(42, 38);
  drop.createSprite(34, 44);
  drop.fillSprite(TFT_WHITE);

  randomSeed(micros());  //Generates random number based on micro seconds
                         //  blinkRate = (random(3, 10))*1000;

  for (int button = 0; button < numButtons; button++) {  //Delcares the buttons pinMode
    pinMode(buttonPins[button], INPUT_PULLUP);
  }

  pinMode(buttonPin, INPUT_PULLUP);

  analogReadResolution(RESOLUTION);

  int reading = analogRead(soilPin);
  //int range = map(value, sMin, sMax, 0, 100);

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {  //Stores all the readings into array
    readings[thisReading] = 0;
  }
}

void loop() {
   
   if (!registration_status)
   {
    // Serial.println("In registration_status condition");

    if (!display_screen)
    {
    // Serial.println("In display screen condition");
    displayonscreen("Enter this mac address in Mobile App:  "+macAddress,0,1);
    display_screen=true;
    }
   
    // Serial.println("In registration if condition @@@@@@@@");
    if (!matchFound and deviceregistration==true) {
    
    doc[result] = macAddress;
    Serial.println("No match found. Added value with key 'new_device'.");
    firebasePath = "/Devices/macaddress/" + result;
    Serial.printf("Set string... %s\n", Firebase.setString(firebaseData, firebasePath, macAddress) ? "ok" : firebaseData.errorReason().c_str());
    displayonscreen("Device Registered Successfully",0,1);
    deviceregistration=false;
    registration_status=true;
  }

   }

      if (publishFlag)
  {
    publishMessage();
    Serial.println("============================Published on aws =====================");
    publishFlag = false; // Reset the flag
  }
      if (mqttflag)
  {
    publish_on_app();
    Serial.println("============================Published on mqtt =====================");
    mqttflag = false; // Reset the flag
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (!client1.connected()) {
    connectAWS();
  }
  client1.loop();
  // publishMessage();
  // client1.loop();
  // delay(2000);

  static int lastCount;
  static String page = "Page One";

  currentTime = millis();
  lastCount = count;

  readButtons();
  readSoil();
  readLight();

  if (count != lastCount){
    background.fillScreen(TFT_WHITE);
    Serial.println(page);
  }

  switch (count) {  //Switch to page depending on button count
    case 1: //Main page, displays emotions based on plant health from sensor inputs
      page = "Page One";
      page1();
      break;
    case 2: //Displays soil moisture levels
      page = "Page Two";
      page2();
      break;
    case 3: //Displays temperature and humidity levels
      page = "Page Three";
      page3();
      break;
    case 4: //Displays light levels
      page = "Page Four";
      page4();
      break;
  }

  background.pushSprite(0,0);
}
void publishCallback()
{
  publishFlag = true;
}

 void mqttCallback()
 {
  mqttflag = true;
 }
void publishreadings()
{
 Serial.println("sending data");




}
void readButtons() {

  for (int button = 0; button < numButtons; button++)  //Read all the buttons
  {
    byte currentButtonState = digitalRead(buttonPins[button]);                    //Declares currentButtonState as the current button read
    if (currentButtonState != lastStates[button] && currentButtonState == LOW) {  // If the button was pressed
      if (currentTime - prevTime > bounceDelay) {                                 // And is debounced
        count += increments[button];                                              // Add increments to counter
        prevTime += bounceDelay;
        if (count > 4) count = 1;  // Verifies that the count is whithin the pages
        if (count < 1) count = 4;
      }
    }
    lastStates[button] = currentButtonState;  // Updates lastStates to equal the current Button State
  }
}

void readMiddleButton() {
  static int lastState = HIGH;
  static unsigned long prevTimeM = 0;


  byte currentButtonState = digitalRead(buttonPin);                    //Declares currentButtonState as the current button read
  if (currentButtonState != lastState && currentButtonState == LOW) {  // If the button was pressed
    if (currentTime - prevTimeM > bounceDelay) {                       // And is debounced
      Serial.println("Middle Pressed");
      prevTimeM += bounceDelay;
    }
  }
  lastState = currentButtonState;  // Updates lastStates to equal the current Button State
}


void readSoil() {  //Reads soil moisture sensor
  
  if (currentTime - prevSoilRead >= waitSoil) {
    // subtract the last reading:
    total = total - readings[readIndex];
    // read from the sensor:
    readings[readIndex] = analogRead(soilPin);
    // add the reading to the total:
    total = total + readings[readIndex];
    // advance to the next position in the array:
    readIndex = readIndex + 1;

    // if we're at the end of the array...
    if (readIndex >= numReadings) {
      // ...wrap around to the beginning:
      readIndex = 0;
    }

    // calculate the average:
    average = total / numReadings;
  }
  prevSoilRead += waitSoil;

  prevMoistPercent = moistPercent;
  moistPercent = map(average, MaxS, MinS, 0, 100);
  moisture = map(average, MaxS, MinS, 0, 8);
}

void readLight() {  //Reads light light sensor
  if (currentTime - prevlightRead >= waitlight) {
    // subtract the last reading:
    totalLight = totalLight - lightReadings[readLightIndex];
    // read from the sensor:
    lightReadings[readLightIndex] = analogRead(lightPin);
    // add the reading to the total:
    totalLight = totalLight + lightReadings[readLightIndex];
    // advance to the next position in the array:
    readLightIndex = readLightIndex + 1;

    // if we're at the end of the array...
    if (readLightIndex >= numlightReadings) {
      // ...wrap around to the beginning:
      readLightIndex = 0;
    }

    // calculate the averageLight:
    averageLight = totalLight / numlightReadings;
  }
  prevlightRead += waitlight;

  lightPercent = map(averageLight, MaxL, MinL, 0, 100);
  light = map(averageLight, MaxL, MinL, 0, 8);
  //Serial.print(lightPercent);
  //Serial.println("%");
}

enum eyeStyle { blinks,
                normal,
                squints,
                sad,
                winks,
                tired,
                shades,
                hot,
                freeze };

//EYE ANIMATION CONTROL//

void eyes(int style) {  //Controls eye graphics and animations

  switch (style) {

    case blinks:  //Blinking animation, switches between normal eye image to closed eye image

      static int blinkRate = 0;            //Variable for time between blinks
      static unsigned long prevBlink = 0;  //Previous blink time
      static bool swapEye = true;          //Checks if eye image has changed


        if (swapEye) {  //If swap eye is true, display eyes
          background.fillRect(20, 60, 88, 18, TFT_WHITE);
          background.fillRect(132, 60, 88, 18, TFT_WHITE);
          background.pushImage(20, 20, 88, 88, Eye);
          background.pushImage(132, 20, 88, 88, Eye);

          //blinkRate = (random(3, 10)) * 1000;  //Updates wait delay time

        } else {  //Else, the swap eye is false, displays blink
          background.fillRect(20, 20, 88, 88, TFT_WHITE);
          background.fillRect(132, 20, 88, 88, TFT_WHITE);
          background.pushImage(20, 60, 86, 16, Blink);
          background.pushImage(132, 60, 86, 16, Blink);

          //blinkRate = 1000;  //Updates wait delay time
        }

      if (currentTime - prevBlink >= blinkRate) {  //If the delay time interval has passed
        swapEye = !swapEye;                        //swapEye is equal to opposite boolean
        if(swapEye) blinkRate = (random(3, 10)) * 1000; else blinkRate = 1000; 
        prevBlink = currentTime;  //Previous blink time is equal to current time
      }

      break;


    case normal:  //Displays basic eye image in standard position
      background.pushImage(20, 20, 88, 88, Eye);
      background.pushImage(132, 20, 88, 88, Eye);
      break;


    case squints:  //Displays squint eye image
      background.pushImage(18, 20, 90, 76, squint);
      background.pushImage(132, 20, 90, 76, squint);
      break;

    case sad:  //Displays blinking eyes with the addition of sad eybrows
      eyes(blinks);
      eyebrow.pushImage(0, 0, 42, 38, eyebrowsadl);
      eyebrow.pushToSprite(&background, 2, 2, TFT_WHITE);
      eyebrow.pushImage(0, 0, 42, 38, eyebrowsadr);
      eyebrow.pushToSprite(&background, 196, 2, TFT_WHITE);
      break;

    case tired:  //Displays sleepy eye image
      background.pushImage(30, 85, 70, 36, tiredeye);
      background.pushImage(140, 85, 70, 36, tiredeye);
      break;

    case shades:  //Displays shades
      // background.pushImage(5, 20, 230, 86, Sunglasses);
      break;

    case hot:             //Displays blinking eyes with drop of sweat Blinking function needs to be modified to work properly with this code
      static int ym = 2;  //y veritcal cords
      static unsigned long prevT = 0;
      static unsigned long waitTime = 3000;
      static int speed = 2;  // Rate at which tongue sprite will move
      static int prevYm = 0;

      if (currentTime - prevT >= waitTime) {  //If three seconds have passed
        //Serial.println("Three seconds have passed");
        prevYm = ym;
        ym += speed;
        if (ym >= 242) {
          ym = 0;
          waitTime = (random(1, 5)) * 1000;
          prevT = currentTime;
        }
      }

      background.fillRect(206, prevYm, 32, 44, TFT_WHITE);
      eyes(blinks);
      drop.pushImage(0, 0, 32, 44, Drop);
      drop.pushToSprite(&background, 206, ym, TFT_WHITE);

      break;

    case freeze:  //Displays eyes and ice for cold emoji
      eyes(blinks);
      drop.pushImage(0, 0, 34, 44, ice);
      drop.pushToSprite(&background, 204, 0, TFT_WHITE);

      break;
  }
}

enum Mouths { pants,
              yum,
              happy,
              smile,
              sick,
              upset,
              sleepy,
              cold };

//MOUTH ANIMATION CONTROL//

void mouth(int style) {  //Function to control mouth animations and graphics
  //Pushes images to sprite
  switch (style) {

    case pants:  //Animation for panting mouth, tongue lowers and rises

      static int pant = 135;      //X veritcal cordanites
      static int pantRate = 2.5;  // Rate at which tongue sprite will move
      static int prevPant = 0;

      background.fillRect(93, prevPant, 54, 68, TFT_WHITE);
      background.pushImage(46, 115, 148, 64, PantMouth);
      background.pushImage(93, pant, 54, 68, Tongue);

      prevPant = pant;
      pant = pant + pantRate;  //Increments pant rate

      if (pant <= 135 || pant >= 170) {  //If pant out of range
        pantRate = -pantRate;            //pantRate is negative, turns opposite
      }
      break;

    case yum:  //Animation of smile with tongue rotating from pivot to make it appear as a licking mouth

      static int x = 120;
      static int y = 195;
      static int lickRate = 3;
      static int prevAngle = 0;
      static int angle = 180;

      background.setPivot(120, 145);

      tongue.fillRect(0, 0, 54, 68, TFT_WHITE);
      background.fillRect(38, 140, 164, 88, TFT_WHITE);
      tongue.pushRotated(&background, prevAngle);  //Push tongue sprite to background sprite

      background.pushImage(38, 140, 164, 88, Smile);
      tongue.pushImage(0, 0, 54, 68, Tongue);             //Push Tongue image to Tongue sprite
      tongue.pushRotated(&background, angle, TFT_WHITE);  //Push tongue sprite to background sprite


      prevAngle = angle;
      angle = angle + lickRate;


      if (angle <= 120 || angle >= 220) {  //If number goes out of bounds
        lickRate = -lickRate;              //Turn lickRate negative
      }
      break;

    case happy:  //Standard happy face, switches between smile and big smile in random intervals

      static bool swapSmile = false;
      static int smileRate = (random(3, 10)) * 1100;
      static unsigned long prevSmile = 0;

      if (currentTime - prevSmile >= smileRate) {  //If the delay time interval has passed
        swapSmile = !swapSmile;                    //swapEye is equal to opposite boolean

        if (swapSmile) {  //If swap eye is true, display eyes
          background.fillRect(38, 140, 164, 82, TFT_WHITE);
          background.pushImage(38, 140, 164, 88, Smile);

          smileRate = (random(3, 60)) * 1000;  //Updates wait delay time

        } else {  //Else, the swap eye is false, displays blink
          background.fillRect(38, 140, 164, 88, TFT_WHITE);
          background.pushImage(38, 140, 164, 82, bigSmile);

          smileRate = (random(3, 10)) * 1100;  //Updates wait delay time
        }

        prevSmile = currentTime;  //Previous blink time is equal to current time
      }

      break;

    case smile:
      background.pushImage(38, 140, 164, 88, Smile);
      break;

    case sick:
      background.pushImage(44, 152, 152, 60, barfmouth1);
      break;

    case upset:
      background.pushImage(70, 120, 132, 74, smallfrown);
      break;

    case sleepy:
      background.pushImage(83, 160, 44, 48, tiredmouth);
      break;

    case cold:  //Shiver animation, mouth moves up and down quickly to appear as shivering
      static int prevY = 0;
      static int Y = 150;
      static int move = 2;

      background.fillRect(52, prevY, 136, 48, TFT_WHITE);
      drop.pushImage(0, 0, 34, 44, ice);
      drop.pushToSprite(&background, 80, 192, TFT_WHITE);
      background.pushImage(52, Y, 136, 48, shiver);

      prevY = Y;
      Y = Y + move;  //Increments Y

      if (Y <= 148 || Y >= 154) {  //If Y out of range
        move = -move;              //move is negative, turns opposite
      }
      break;
  }
}

void page1() {
  static int emoji = 0;
  static int prevEmoji = 0;
  static int read = moistPercent;
  static int interval = 400;
  static bool watering = false;
  static unsigned long lastTime;

  //If emoji is not equal to prevEmoji clear screen;
  if (emoji != prevEmoji){
    background.fillScreen(TFT_WHITE);
  }
  prevEmoji = emoji;

  //if water rising action 1 if water standing or falling action 2

 if (currentTime - lastTime >= interval){ //Every 100 millis
    read = moistPercent; //read moisture
    lastTime = currentTime; //resetTime
  }

  if (moistPercent > 5 && moistPercent < 60 && moistPercent > read + 2) { //If moisture reading has increased in 100 mills 
    watering = true; //Displays yum licking emoji when being watered
    lastTime = currentTime;
  } else {
    watering = false;
   //lastTime = currentTime;
  }

if (watering){
    emoji = 5;
    eyes(squints);//How too make animation continue playing until water level stops rising?
    mouth(yum);
}else if (watering == false){
  if (moistPercent < 5) {//Happy emoji displayed if not in soil
    emoji = 1;
    eyes(blinks);
    mouth(smile);
  }
  if (moistPercent < 20 && moistPercent >= 5) {//Thirsty emoji displayed if soil is dry
    emoji = 2;
    eyes(blinks);
    mouth(pants);
  }
  if (moistPercent >= 20 && moistPercent <= 60) {//Happy emoji displayed for Optimal Moisture range
    emoji = 3;
    eyes(blinks);
    mouth(smile);
  }
  if (moistPercent > 60) {  //Displays Sick emoji when soil is too wet
    emoji = 4;
    eyes(blinks);
    mouth(sick);
  }
}

  // if(tooLight && soilMoist){//Sunglasses emoji for when it's too light

  // }
  // if(tooDark && soilMoist){//Sleeping emoji for when it's too dark

  // }
  // if(tooHot && soilMoist && lightGood){//Sweating emoji for when it's too hot

  // }
  // if(tooCold && soilMoist && lightGood){//Shivering emoji for when it's too cold

  // }

} 

void page2() {                 //Page 2 - Moisture sensor levels

  int soilMoist = map(average, MaxS, MinS, 240, 0);//mapped sensor reading
  static int prevMoist = 0;    //Previous mapped sensor reading

  background.fillScreen(TFT_CYAN);
  background.fillRect(0, 0, 240, soilMoist, TFT_WHITE);

  background.fillCircle(120, 120, 80, TFT_BLACK);
  background.setFreeFont(FSSBO24);
  background.setTextDatum(MC_DATUM);
  background.setTextColor(TFT_WHITE);
  background.drawString(String(moistPercent) + "%", 120, 120);

}

void page3() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);  // populate temp and humidity objects with fresh data
  fahrenheit = temp.temperature * 1.8 + 32;
  humid = humidity.relative_humidity;

  background.pushImage(0, 10, 108, 216, Thermometer);
  background.fillCircle(160, 60, 55, TFT_YELLOW);
  background.fillCircle(160, 180, 55, TFT_SKYBLUE);
  background.setTextDatum(MC_DATUM);
  background.setTextColor(TFT_BLACK);
  background.setFreeFont(FSSBO24);

  background.drawString(String(fahrenheit) + "F", 160, 60);
  background.drawString(String(humid) + "%", 160, 180);
}

void page4() {
  background.setTextDatum(MC_DATUM);
  background.setTextColor(TFT_BLACK);
  background.setFreeFont(FSSBO24);
  background.pushImage(10, 10, 220, 220, Sun);
  background.drawString(String(lightPercent) + "%", 120, 120);
}
void reconnect() {
 
  madd = macAddress;
  char cbuff[madd.length() + 1]; //Finds length of string to make a buffer
  madd.toCharArray(cbuff, madd.length() + 1); 


  while (!client.connected()) {
    if (client.connect(cbuff)) {
      Serial.println("MQTT Connected");
      // client.subscribe("DEVICEONBOARDING");
      // String topic1,topic2;
      // topic1="DEVICEONBOARDING/"+macAddress;
      // topic2="SETTINGS/"+macAddress;
      // client.subscribe(topic1.c_str());
      // client.subscribe(topic2.c_str());
      client.subscribe(("DEVICEONBOARDING/" + String(macAddress)).c_str());
      client.subscribe(("MAXS/" + String(macAddress)).c_str());
      client.subscribe(("MINS/" + String(macAddress)).c_str());
      client.subscribe(("POLL/" + String(macAddress)).c_str());


      // client.subscribe("SETTINGS/"+macAddress.c_str());

      client.subscribe("set");
    } else {
      Serial.print("failed to connect with MQTT with state ");
      Serial.print(client.state());
      delay(5000);
    }
  }
}


void callback(char* topic, byte* message, unsigned int length) 
{
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  String messageTemp;
  String receivedMessage = "";
  for (int i = 0; i < length; i++) {
    receivedMessage += (char)message[i];
  }
    Serial.println();
    
  Serial.println("Message received: " + receivedMessage);
  // DynamicJsonDocument doc1(2048);
  DeserializationError error = deserializeJson(doc1, receivedMessage);

  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }
if (String(topic) == "DEVICEONBOARDING/"+macAddress ) 
{
   Serial.println("message received on topic: DEVICEONBOARDING/"+macAddress);
   deviceregistration=doc1["registered"];
   
  
}
if (String(topic) == "POLL/"+macAddress ) 
{
   Serial.println("message received on topic: POLL/"+macAddress);
   String payload = "{\"status\":\"true\"}";
   if(!pollflag)
   {
   client.publish(("POLL1/" + String(macAddress)).c_str(),payload.c_str());
   pollflag=true;
   }
   
  
}


if (String(topic) == "MAXS/"+macAddress) 
{
    Serial.println("message received on topic: MAXS/"+macAddress);
    MaxS=doc1["maxs"];
    Serial.print("maxS value is ");
    Serial.println(MaxS);
    Serial.println();
}


if (String(topic) == "MINS/"+macAddress) 
{
  Serial.println("message received on topic: MINS/"+macAddress);
  int ta;

  // MaxS=doc1["maxs"];
  MinS=doc1["mins"];
  Serial.print("minS value is " );
  Serial.println(MinS);
  Serial.println();
  // Serial.print("t value is ");
  // Serial.println(ta);
  // delay(5000);
}


}

// void displayonscreen(String comment,int xaxis,int yaxis)
// {
//       display.setTextSize(1);
//       display.setTextColor(WHITE);
//       display.setCursor(xaxis, yaxis);
//       display.println(comment);
//       display.display();
//       delay(2000);
//       display.clearDisplay();


// }

void publish_on_app()
{

  lightPercent=10;
  moistPercent=20;
  fahrenheit=21;
  humid=200;
  // DynamicJsonDocument doc2(200);
  doc1["lightpercent"]=lightPercent;
  doc1["moistpercent"] = moistPercent;
  doc1["temp_f"] = fahrenheit;
  doc1["humid"]=humid;
  doc1["macaddress"]=macAddress;
  String jsonString,pubtopic;
  // pubtopic="READINGS/"+macAddress;
  serializeJson(doc1, jsonString);
  Serial.println("publish redings on app");
  client.publish(("READINGS/" + String(macAddress)).c_str(), jsonString.c_str());


}

void displayonscreen(String comment,int xaxis,int yaxis)
{

  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(FSSBO24);
  tft.setCursor(xaxis, yaxis);
  tft.print(comment);
  delay(2000);
  tft.fillScreen(TFT_WHITE);
  // background.drawString(String(humid) + "%", 160, 180);
}

void connectAWS()
{
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client1.setServer(AWS_IOT_ENDPOINT, 8883);
  // Create a message handler
  client1.setCallback(messageHandler);
  displayonscreen("Connecting to AWS IOT",0,1);

  Serial.println("Connecting to AWS IOT");
  while (!client1.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client1.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  // Subscribe to a topic
  // client1.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
  displayonscreen("AWS IoT Connected!",0,1);
}

void publishMessage()
{ 
  lightPercent=10;
  moistPercent=20;
  fahrenheit=21;
  humid=200;
  DynamicJsonDocument doc2(1024);
  doc2["time"]=0;
  doc2["lightpercent"]=lightPercent;
  doc2["moistpercent"] = moistPercent;
  doc2["temp_f"] = fahrenheit;
  doc2["humid"]=humid;
  doc2["mac_address"]=macAddress;
  String jsonbuffer;
  serializeJson(doc2, jsonbuffer); 
  client1.publish(AWS_IOT_PUBLISH_TOPIC, jsonbuffer.c_str());
}
void messageHandler(char* topic, byte* payload, unsigned int length)
{

}
