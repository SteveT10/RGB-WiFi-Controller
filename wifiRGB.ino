#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <SPIFFS.h>
#include <WiFi.h>

//Global constants
#define OUT_PIN 4
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS 300

#define SSID "yourSSID" //Enter network info
#define WIFI_PASS "yourPassword"
#define WIFI_TIMEOUT 20000 //20 second to connect before timeout.

//Global Variables
AsyncWebServer SERVER(80); //Create Server object on port 80.
CRGB leds[NUM_LEDS];

//For moving LEDs through a strip
int16_t rgbIndex = 1;
uint8_t travelSpd = 0;
bool reversedDir = 0;
CRGB ledStorage1;
CRGB ledStorage2;

void setup() {
  delay(1000); //1 Second delay to start
  Serial.begin(115200); //For debugging
  
  if(!SPIFFS.begin()) {
    Serial.println("Error while mounting SPIFFS");
  }

  //Wifi Server SetUp
  connectToWiFi();
  getServerFiles();
  setUpServer();
  SERVER.begin(); //begin webpage 

  //Set up LEDs
  FastLED.addLeds<LED_TYPE, OUT_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(50);
  fill_rainbow(leds, NUM_LEDS, 0, 7);
  FastLED.show();
  
  //Initial Setup for cycling LEDs
  ledStorage1 = leds[0];
  ledStorage2 = leds[1];
}

void loop() {
  if(travelSpd != 0) { //Disable Cycling if zero
    if(reversedDir) {
      moveLeds((NUM_LEDS - 1), 1, -1); 
    } else { //Direction is normal
      moveLeds(1, (NUM_LEDS - 1), 1);
    }
  } 
}

void moveLeds(uint16_t startIndex, uint16_t endIndex, int8_t increment) {
  if (rgbIndex == endIndex) {
    leds[endIndex] = ledStorage1;
    ledStorage1 = leds[0];
    leds[0] = ledStorage2;
    ledStorage2 = leds[startIndex];
    delay(100 - travelSpd);
    FastLED.show();
    rgbIndex = startIndex;
    //Serial.println("Reached Last LED, looping back around");
  } else {
    leds[rgbIndex] = ledStorage1;
    ledStorage1 = leds[rgbIndex + increment];
    leds[rgbIndex + increment] = ledStorage2;
    ledStorage2 = leds[rgbIndex + (increment * 2)];
    rgbIndex += increment * 2;    
  }
}

void connectToWiFi() {
  Serial.print("\nConnecting to ");
  Serial.println(SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, WIFI_PASS);
  unsigned long elapsedTime = 0;

  while (WiFi.status() != WL_CONNECTED && elapsedTime < WIFI_TIMEOUT) {
    delay(1000);
    Serial.print(".");
    elapsedTime += 1000; 
  }

  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("\nConnection Timeout, WiFi service is inactive");
  } else {
    Serial.print("\nConnected to ");
    Serial.println(SSID);
    Serial.print("Local IP is ");
    Serial.println(WiFi.localIP());
  }
}

void getServerFiles() {
  //Root Route
  SERVER.on("/", HTTP_GET, [] (AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  //Get CSS file when requested by html file
  SERVER.on("/index.css", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.css", "text/css");
  });
  
  //Get JS file when requested by html file
  SERVER.on("/index.js", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.js", "text/javascript");
  });
}

void setUpServer() {
  SERVER.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String outputId;
    uint8_t stateValue;
    if (request->hasParam("output") && request->hasParam("state")) {
      outputId = request->getParam("output")->value();
      stateValue = (request->getParam("state")->value()).toInt();
      if(outputId == "brightness") {
        FastLED.setBrightness(stateValue);
        FastLED.show();
      } else if (outputId == "reverseDir") {
        reversedDir = (stateValue == 1);
      } else if (outputId == "travelSpd") {
        travelSpd = stateValue;
      } else {
        Serial.println("Unrecognized Output ID");
      }
    } else if (request->hasParam("color") && request->hasParam("start")) {
      changeRgb((request->getParam("color")->value()),
                (request->getParam("start")->value()).toInt(),
                (request->getParam("end")->value()).toInt());
    } else {
      Serial.println("Unrecognized Parameters");
    }
    request->send(200, "text/plain", "OK");
  });
}

void changeRgb(String hexColor, uint16_t startLed, uint16_t endLed) {
  byte r, g, b;
  char rgbHolder[7];

  //Convert hex to rgb.
  hexColor.toCharArray(rgbHolder, 7);
  sscanf(rgbHolder, "%02x%02x%02x", &r, &g, &b);

  fill_solid((leds + startLed - 1), (endLed - startLed), CRGB(r, g, b));
  FastLED.show();
 
  //Refresh memory containing old rgb values.
  ledStorage1 = leds[0];
  ledStorage2 = leds[1];
  rgbIndex = 1; //does not matter so long as storage corrolates with index.
}
