/* Main configuration file for the ESP32 module of the project "dropplet", which is a free online open sourced full stacked software for gardening sharing (thats a lot of words but I swear they mean work). 

OBJECTIVE: Program the ESP32 microcontroller to use peripherals to measure humidity conditions and water plants accordingly, passing data to a web API.

The organization of the code puts moduluratity first, systems pin definitions and its functions will be together. Stakeholder testing for ESP32 still undone. Future add-ons projected: Motor pump, Wifi Server instead of client, Battery and battery life indicator on ESP32
*/

// Librarys used for the project. I always follow the policy of using the least amount of libraries possible.
#include <Arduino.h> // Main library for arduino
#include <WiFi.h>    // Helps with Wifi handling and has some functions for WiFi events
#include <HTTPClient.h> // HTTP CRUD functionalities and other tools.
#include <ArduinoJson.h> // JSON handling
#include <DHT.h> // Temperature and humidity sensors

/* SYSTEM CONTROL
RedLed indicates Off, GreenLed indicates On.
*/

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
  digitalWrite(GreenLed, greenLedState);
  digitalWrite(RedLed, redLedState);
}

/* ANALOG SENSORS
A group of different complementary sensors for home growing, includes an DHT11, a Resistive Soil Sensor and a LDR. All read functions pass as a return a Float primitive, so it can be send as a JSON object.
*/
#define dht11 32
#define SoilSensor 33
int hum_soil;
int hum_dht;
int tem_dht;
hw_timer_t *timer_2 = NULL; // Timer 2 millis

// Resistive soil sensors are (as their name states) based on resistance, so they are read as any analog value.
void humRead(){
  unsigned long previousMillis = 0;
  unsigned long currentMillis = millis(); // Get the current time
  hum_dht = analogRead(dht11);
  Serial.print("DHT value: ");
  Serial.println(hum_dht);
  hum_soil = analogRead(SoilSensor);
  Serial.print("RESISTIVE SOIL value: ");
  Serial.println(hum_soil);
}

void humPost(){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://192.168.1.34/api/dropplet");
    http.addHeader("Content-Type", "application/json");
    String payload = "{\"humidity\":2.22,\"peristatus\":true}";
    int httpCode = http.POST(payload);
    static String response = http.getString();
    Serial.println(response);
    http.end();
  }
}

/* WIFI CONNECTION
The blue led its for communicating phisically the state. A blink each second means that the Wifi its OK, if the Led stays on then it means that the Wifi is disconnected. Fast Blinking (500ms) means it has disconnected and manual connection is required 
*/

#define BlueLed 27
volatile bool blueLedState = false;

#define WIFI_NETWORK "DANIEL"
#define WIFI_PASSWORD "10552669"
#define WIFI_TIMEOUT_MS 20000

void keepWiFiAlive(void * parameters){
  for(;;){
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("WiFi still connected");
      vTaskDelay(10000 / portTICK_PERIOD_MS);
      digitalWrite(BlueLed, HIGH);
      continue;
    }

    Serial.println("WiFi Connecting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

    unsigned long startAttemptTime = millis();

    // Keep looping while we're not connected and haven't reached the timeout
    while (WiFi.status() != WL_CONNECTED &&
          millis() - startAttemptTime < WIFI_TIMEOUT_MS){}
    // When we couldn't make a WiFi connection
      if(WiFi.status() != WL_CONNECTED){
        Serial.println("[WIFI] FAILED");
        vTaskDelay(20000/ portTICK_PERIOD_MS);
        continue;
      }

    Serial.println("[WIFI] Connected: " + WiFi.localIP());
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  xTaskCreatePinnedToCore(
    keepWiFiAlive,
    "KEEP WIFI ALIVE",
    5000,
    NULL,
    1,
    NULL,
    CONFIG_ARDUINO_RUNNING_CORE
  );
  
  pinMode(RedLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);
  pinMode(BlueLed, OUTPUT);

  timer_1 = timerBegin(0, 80, true); // 80 prescaler
  timerAttachInterrupt(timer_1, &RGBLeds, true);
  timerAlarmWrite(timer_1, 1000000, true); // 1000000 microseconds = 1 second
  timerAlarmEnable(timer_1);
}

void loop() {
  unsigned long previousMillis = 0;
  unsigned long currentMillis = millis(); // Get the current time
  if (currentMillis - previousMillis < 200000) {
    humRead();
    humPost();
  previousMillis = currentMillis;
  };
}