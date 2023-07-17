// #include <ESP8266WiFi.h>
// #include <FirebaseESP8266.h>
#include <WiFi.h>
#include <WiFiManager.h> 
#include <FirebaseESP32.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define WIFI_SSID "Virus"
#define WIFI_PASSWORD "12345678"
const char* mqtt_server = "broker.emqx.io";
#define FIREBASE_HOST "espledonoff-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "ZnIORSB3cOXMVmPFZiQdBkSMsNgwhNhsvaKhM0QU"


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

bool deviceregistration=false;


static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };


WiFiClient espClient;
PubSubClient client(espClient);

String macAddress;
String a="new";
String madd,mac;
char cbuff[18];
String result;
String checkValue = "11:22:12:444:444";
bool matchFound = false;

String jsonData;
DynamicJsonDocument doc(1024);
DeserializationError error = deserializeJson(doc, jsonData);

JsonArray devices_details = doc.to<JsonArray>();
String firebasePath;

FirebaseData firebaseData;
FirebaseJson json;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // WiFiManager wm;
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Connecting to Wifi");
    display.display();
    delay(2000);
    display.clearDisplay();

  }

    // bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    // res = wm.autoConnect("IoTPlant","12345678"); // password protected ap

    // if(!res) {
    //     Serial.println("Failed to connect");
    //     // ESP.restart();
    // } 
    // else {
    //     //if you get here you have connected to the WiFi    
    //     Serial.println("connected...yeey :)");
    // }
    Serial.println("Connected to WiFi");
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println("Connected to Wifi");
    display.display();
    // display.clearDisplay();
    delay(2000);
    display.clearDisplay();


    // display.setTextSize(1);
    // display.setTextColor(WHITE);
    // display.setCursor(0, 5);
    // display.println("Checking Device is registered on the Database");
    // display.display();
    // // display.clearDisplay();
    // delay(2000);
    // // display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 5);
    display.println("Connecting to MQTT");
    display.display();
    // display.clearDisplay();
    delay(2000);
    display.clearDisplay();


  Serial.println(WiFi.macAddress());
  macAddress = WiFi.macAddress();
  result=a+macAddress.substring(0,10);
  Serial.println("Connecting to MQTT");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("MQTT Connected");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  display.println("MQTT Connected");
  display.display();
    // display.clearDisplay();
  delay(2000);
    // display.clearDisplay();
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally



  // display.display();
  // delay(2000);
  // Clear the buffer
  // display.clearDisplay();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.printf("Get string... %s\n", Firebase.getString(firebaseData, F("/Devices/macaddress/")) ? firebaseData.to<const char *>() : firebaseData.errorReason().c_str());
  // jsonData=Firebase.getString(firebaseData, F("/Devices/macaddress")) ? firebaseData.to<const char *>() : firebaseData.errorReason().c_str());
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
    
    Serial.print("Key: ");
    Serial.print(key);
    Serial.print(" | Value: ");
    Serial.println(value);
    if (value == macAddress) {
      Serial.print("Match found! Key: ");
      Serial.println(key);
      matchFound = true;
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 5);
      display.println("Device already registered");
      display.display();
      delay(2000);
      display.clearDisplay();
      break;
    }
  }

    if (!matchFound and deviceregistration==true) {
    
    doc[result] = macAddress;
    Serial.println("No match found. Added value with key 'new_device'.");
    firebasePath = "/Devices/macaddress/" + result;
    Serial.printf("Set string... %s\n", Firebase.setString(firebaseData, firebasePath, macAddress) ? "ok" : firebaseData.errorReason().c_str());
    deviceregistration=false;

  }
  String modifiedJsonData;
  serializeJson(doc, modifiedJsonData);

  Serial.println("Modified JSON data:");
  Serial.println(modifiedJsonData);
}

void loop() {


  if (!client.connected()) {
    reconnect();
  }
  client.loop();



}
void reconnect() {
 
  madd = macAddress;
  char cbuff[madd.length() + 1]; //Finds length of string to make a buffer
  madd.toCharArray(cbuff, madd.length() + 1); 


  while (!client.connected()) {
    if (client.connect(cbuff)) {
      Serial.println("MQTT Connected");
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
  DynamicJsonDocument doc1(1024);
  DeserializationError error = deserializeJson(doc1, receivedMessage);

  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }
if (String(topic) == "DEVICEONBOARDING" ) 
{
   Serial.println("message received on topic: DEVICEONBOARDING ");
   deviceregistration=doc1["registered"]:
  
}
}