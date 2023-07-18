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

char devices;
int i= 0;
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

bool deviceregistration=false,registration_status=false,display_screen=false;
String comment;
int xaxis=0,yaxis=0;

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

String firebasePath;
FirebaseData firebaseData;
FirebaseJson json;

void displayonscreen(String comment,int xaxis,int yaxis);
void callback(char* topic, byte* message, unsigned int length) ;
void reconnect();

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
    displayonscreen("Connecting to Wifi",0,1);
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
    client.publish("test","message");


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
}

void loop() {
   Serial.print("registration_status:  ");
   Serial.println(registration_status);
   Serial.print("deviceregistration:   ");
   Serial.println(deviceregistration);
   Serial.print("matchFound:   ");
   Serial.println(matchFound);

   if (!registration_status)
   {
    Serial.println("In registration_status condition");

    if (!display_screen)
    {
    Serial.println("In display screen condition");
    displayonscreen("mac"+macAddress,0,1);
    display_screen=true;
    }
   
    Serial.println("In registration if condition @@@@@@@@");
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




  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(2000);

}
void reconnect() {
 
  madd = macAddress;
  char cbuff[madd.length() + 1]; //Finds length of string to make a buffer
  madd.toCharArray(cbuff, madd.length() + 1); 


  while (!client.connected()) {
    if (client.connect(cbuff)) {
      Serial.println("MQTT Connected");
      client.subscribe("DEVICEONBOARDING");
      String topic1;
      topic1="DEVICEONBOARDING/"+macAddress;
      client.subscribe(topic1.c_str());

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
  DynamicJsonDocument doc1(1024);
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
}

void displayonscreen(String comment,int xaxis,int yaxis)
{
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(xaxis, yaxis);
      display.println(comment);
      display.display();
      delay(2000);
      display.clearDisplay();


}
