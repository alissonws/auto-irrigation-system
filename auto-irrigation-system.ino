#include "Arduino.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "thermistor.h"

#define ESP_BUILTIN_LED 2 //ESP8266-12E has led pinned to VCC, so it will light on LOW
#define DEVICE_OUTPUT_PIN 14
#define DEVICE_INFO_LED_PIN 5
#define DEVICE_CURRENT_DETECTOR_INPUT_PIN 12



// Analog pin used to read the NTC
#define NTC_PIN               A0

// Thermistor object
THERMISTOR thermistor(NTC_PIN,        // Analog pin
                      100000,          // Nominal resistance at 25 ºC
                      3950,           // thermistor's beta coefficient
                      2000);         // Value of the series resistor

// Global temperature reading
uint16_t temp;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up device");
  
  pinMode(ESP_BUILTIN_LED, OUTPUT);
  pinMode(DEVICE_OUTPUT_PIN, OUTPUT);
  pinMode(DEVICE_INFO_LED_PIN, OUTPUT);
  pinMode(DEVICE_CURRENT_DETECTOR_INPUT_PIN, INPUT_PULLUP);

  
  digitalWrite(ESP_BUILTIN_LED, LOW);
  digitalWrite(DEVICE_OUTPUT_PIN, LOW);
  digitalWrite(DEVICE_INFO_LED_PIN, HIGH);


  setupWiFi();
  setupOTAUpdateServer();
  
  digitalWrite(DEVICE_INFO_LED_PIN, LOW);
  digitalWrite(ESP_BUILTIN_LED, HIGH);
  delay(200);
  digitalWrite(DEVICE_INFO_LED_PIN, HIGH);
  digitalWrite(ESP_BUILTIN_LED, LOW);
  delay(200);
  digitalWrite(DEVICE_INFO_LED_PIN, LOW);
  digitalWrite(ESP_BUILTIN_LED, HIGH);
}

void loop() {
  ArduinoOTA.handle(); //Checking for OTA updates

  //digitalWrite(DEVICE_INFO_LED_PIN, LOW);
  //delay(1000);
  //digitalWrite(DEVICE_INFO_LED_PIN, HIGH);
  //delay(1000);
  digitalWrite(DEVICE_OUTPUT_PIN, HIGH);

  temp = thermistor.read();   // Read temperature

  //Serial.println(analogRead(A0));

  delay(20000);


}

void setupOTAUpdateServer() {
    // OTA server set up //
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
}

void setupWiFi() {
  // Set web server port number to 80
  WiFiServer server(80);
  
  // WiFi setup //
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  // put your setup code here, to run once:

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  //reset settings - wipe credentials for testing
  //wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
}
