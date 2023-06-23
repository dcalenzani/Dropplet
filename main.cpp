/* Main configuration file for ESP32 project named "dropplet", which should evolve as a home appliance for gardening. The organization of the code puts moduluratity first, systems pin definitions and its functions will be together instead of definition > function structure. Stakeholder testing for ESP32 still undone. Future add-ons projected: Battery and battery life ESP32, motor pump, Wifi Server instead of client. */

// Librarys used for the project. I always follow the policy of using the least amount of libraries possible.
#include <Arduino.h> // Main library for arduino
#include <WiFi.h>    // Helps with Wifi handling and has some functions for WiFi events
#include <HTTPClient.h> // HTTP CRUD functionalities and other tools.
#include <ArduinoJson.h> // JSON handling
#include <DHT.h> // Temperature and humidity sensors

/* SYSTEM CONTROL
RedLed indicates Off, GreenLed indicates On. */

// #define Button
#define RedLed 12
#define GreenLed 14
// Boolean states will be used as HIGH and LOW states
volatile bool redLedState = false;
volatile bool greenLedState = false;

hw_timer_t *timer_1 = NULL; // Timer 1 millis

void IRAM_ATTR RGBLeds() {
  redLedState = !redLedState;
  greenLedState = !greenLedState;
  digitalWrite(GreenLed, redLedState);
  digitalWrite(RedLed, greenLedState);
}

/* ANALOG SENSORS
A group of different complementary sensors for home growing, includes an DHT11, a Resistive Soil Sensor and a LDR. All read functions pass as a return a Float primitive, so it can be send as a JSON object. */
#define dht11 04
#define SoilSensor 00
int hum_soil;
int hum_dht;
int tem_dht;
hw_timer_t *timer_2 = NULL; // Timer 2 millis

// Resistive soil sensors are (as their name states) based on resistance, so they are read as any analog value.
void humRead(){
  hum_soil = analogRead(dht11);
  Serial.print("Moisture value: ");
  Serial.println(hum_soil);
}

void humSend(){

}

/* WIFI CONNECTION
The blue led its for communicating phisically the state. A blink each second means that the Wifi its OK, if the Led stays on then it means that the Wifi is disconnected. Fast Blinking (500ms) means it has disconnected and manual connection is required */
#define BlueLed 27
volatile bool blueLedState = false;

const char* ssid = "DANIEL_plus";
const char* password = "10552669";
const char* serverName = "http://192.168.1.34/api/dropplet";

void APHandler(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
  Serial.println("Connected To The WiFi Network");
  
}
 
void IPHandler(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}
 
void DisconnectionHandler(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
  Serial.println("Disconnected From WiFi Network");
  // Attempt Re-Connection
  WiFi.begin(ssid, password);
}

/*
void humPost(){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://192.168.1.34/api/dropplet");
    http.addHeader("Content-Type", "application/json");
    String payload = "{\"humidity\":2.22,\"peristatus\":true}";
    int httpCode = http.POST(payload);
    String response = http.getString();
    Serial.println(response);
    http.end();
  }
}
*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(15200);
  pinMode(RedLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);
  pinMode(BlueLed, OUTPUT);

  timer_1 = timerBegin(0, 80, true); // 80 prescaler
  timerAttachInterrupt(timer_1, &RGBLeds, true);
  timerAlarmWrite(timer_1, 1000000, true); // 1000000 microseconds = 1 second
  timerAlarmEnable(timer_1);

  timer_2 = timerBegin(1, 80, true); // 80 prescaler
  timerAttachInterrupt(timer_2, &humRead, true);
  timerAlarmWrite(timer_2, 10000000, true); // 10000000 microseconds = 10 seconds
  timerAlarmEnable(timer_2);

  WiFi.mode(WIFI_STA);
  WiFi.onEvent(APHandler, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(IPHandler, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(DisconnectionHandler, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi Network ..");

}

void loop() {
  /*
  static unsigned long lastRequestTime = 0;
  const unsigned long requestInterval = 10000000; // milliseconds
  
  // put your main code here, to run repeatedly:
  if (WiFi.status() == WL_CONNECTED) {
    if (millis() - lastRequestTime >= requestInterval) {
      lastRequestTime = millis();
      
      HTTPClient http;
      http.begin("http://192.168.1.34:3000/api/dropplet");
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST("{\"humidity\":2.22,\"peristatus\":true}");
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
    }
  }
  */
}
