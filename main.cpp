#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define button 27        // system button control
#define redLed 12        // red led
#define greenLed 14      // green led

volatile bool redLedState = false;
volatile bool greenLedState = false;

hw_timer_t *timer_1 = NULL; // Timer 1 millis
hw_timer_t *timer_2 = NULL; // Timer 2 millis

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
 
void DiscoHandler(WiFiEvent_t wifi_event, WiFiEventInfo_t wifi_info) {
  Serial.println("Disconnected From WiFi Network");
  // Attempt Re-Connection
  WiFi.begin(ssid, password);
}

void IRAM_ATTR RGLeds() {
  {
    redLedState = !redLedState;
    greenLedState = !greenLedState;
    digitalWrite(greenLed, redLedState);
    digitalWrite(redLed, greenLedState);
  }
}
 /*
void IRAM_ATTR humPost(){
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
  Serial.begin(9600);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  timer_1 = timerBegin(0, 80, true); // 80 prescaler
  timerAttachInterrupt(timer_1, &RGLeds, true);
  timerAlarmWrite(timer_1, 1000000, true); // 1000000 microseconds = 1 second
  timerAlarmEnable(timer_1);

/*
  timer_2 = timerBegin(1, 80, true); // 80 prescaler
  timerAttachInterrupt(timer_2, &humPost, true);
  timerAlarmWrite(timer_2, 10000000, true); // 10000000 microseconds = 10 seconds
  timerAlarmEnable(timer_2);
*/

  WiFi.mode(WIFI_STA);
  WiFi.onEvent(APHandler, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(IPHandler, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(DiscoHandler, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi Network ..");

}

void loop() {
  static unsigned long lastRequestTime = 0;
  const unsigned long requestInterval = 10000; // milliseconds
  
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
}
