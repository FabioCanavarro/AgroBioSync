/*
Todo:
- Add Multiprocessing to proccess the if conditions parallelly
- Add error handling for when the sensor fails to read
- Show error message on the website
- fix the circuit not responding after restarting microcontroller
*/

#include <Arduino.h> //done
#include <ESP8266WiFi.h> //done
#include <ESP8266HTTPClient.h> //done
#include <ArduinoJson.h> //done
#include <WiFiManager.h> //done
#include "DHT.h" //done
#include <TimeLib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/* 
*Commands 
Upload:
      C:\Users\ASUS\.platformio\penv\Scripts\platformio.exe run --target upload
Serial Monitor:
      C:\Users\ASUS\.platformio\penv\Scripts\platformio.exe device monitor --baud=115200 

Combined command:
  cd NodeMcuHTTPcode; C:\Users\ASUS\.platformio\penv\Scripts\platformio.exe run --target upload; C:\Users\ASUS\.platformio\penv\Scripts\platformio.exe device monitor --baud 115200
*/



//*Pins and Setup
// Initiating the Pins
// Receiving Pins
const int DHTPIN = D3;
const int HYGROGENATOR_PIN = D6; 
const int DB18S20 = D4; 

// Output Pins
const int UAH = D5; 
const int water_pump = D8;
const int fertilizer_pump = D7;
const int growlight = D0; 

// DHT22 setup
DHT dht(DHTPIN, DHT22);

// DS18B20 setup
OneWire oneWire(DB18S20);
DallasTemperature sensors(&oneWire);

//*Variable
// Needed config for HYGROGENATOR
const int needed_air_temp= 26; //adjustable
const int needed_soil_temp = 29; //adjustable
const int needed_humidity_percentage = 60; //adjustable
const int needed_moisture_percentage = 70; //adjustable
const int AirValue = 561;   //replace the value with value when placed in air using calibration code 
const int WaterValue = 310; //replace the value with value when placed in water using calibration code 

// needed config for Fertilizer
const int duration_before_fertlizer_in_hours=72;

// Device operation durations (in milliseconds)
const unsigned long FERTILIZER_SPRAY_DURATION = 900;  // 0.9 seconds
const unsigned long WATER_SPRAY_DURATION = 900;       // 0.9 seconds
const unsigned long GROW_LIGHT_CHECK_INTERVAL = 200;  // 0.2 seconds
const unsigned long UAH_CHECK_INTERVAL = 200;         // 0.2 seconds
const unsigned long WATER_PUMP_DURATION = 600;        // 0.6 seconds
const unsigned long SENSOR_READ_INTERVAL = 2000;      // 2 seconds
const unsigned long HTTP_POST_INTERVAL = 2000;        // 2 seconds

// Timer variables
unsigned long lastFertilizerTime = 0;
unsigned long lastWaterSprayTime = 0;
unsigned long lastGrowLightCheck = 0;
unsigned long lastUAHCheck = 0;
unsigned long lastWaterPumpCheck = 0;
unsigned long lastSensorRead = 0;
unsigned long lastHTTPPost = 0;
unsigned long deviceStartTimes[6] = {0, 0, 0, 0, 0, 0}; // Store start times for each device
bool deviceActive[6] = {false, false, false, false, false, false}; // Track if devices are active

// Misc
int soilMoistureValue = 0;
int soilmoisturepercent= 0;
int currentsecond;
int tempsecond = 1800;//some delay before spraying

// Initiating variable
int hum;
int tempC;
int moisturelevel;
int soiltempc;
int soilTemp;
int airTemp;
int humidity;
int soilMoisture;
int timer = millis();

// Needed config for wifi
String url = "http://localhost:3001/api/nodeMCU-data";
String username = "AgroBioSync";
String password = "GreenSync";
WiFiServer server(80);
WiFiManager wm;


//*Functions
int readDS18B20(){
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if(tempC != DEVICE_DISCONNECTED_C) 
  {
    return tempC;
  } 
  else
  {
    Serial.println("Error: Could not read temperature data");
    return 0;
  }
  
}

int read_hygrometer(){
  soilMoistureValue = analogRead(HYGROGENATOR_PIN);
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  if(soilmoisturepercent >= 100)
  {
    return 100;
  }
  else if(soilmoisturepercent <= 0)
  {
    return 0;
  }
  else
  {
    return soilmoisturepercent;
  }
}


void setup() {
  // Initialize output pins
  pinMode(growlight, OUTPUT);
  pinMode(water_pump, OUTPUT);
  pinMode(fertilizer_pump, OUTPUT);
  pinMode(UAH, OUTPUT);
  
  // Set initial state for all outputs (HIGH is OFF for pumps)
  digitalWrite(fertilizer_pump, HIGH);
  digitalWrite(water_pump, HIGH);
  digitalWrite(growlight, LOW);
  digitalWrite(UAH, LOW);

  // Initialize sensors
  sensors.begin();
  dht.begin();
  
  // Initialize Serial communication
  Serial.begin(115200);
  Serial.println("Ready");

  // WiFi Setup
  wm.setConfigPortalTimeout(1000000000);
  wm.setBreakAfterConfig(false);
  bool res = wm.autoConnect(username.c_str(), password.c_str());

  if(!res) {
    Serial.println("Failed to connect");
    ESP.restart();
  } else {
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
  }

  // Initialize all timers
  unsigned long currentTime = millis();
  lastFertilizerTime = currentTime;
  lastWaterSprayTime = currentTime;
  lastGrowLightCheck = currentTime;
  lastUAHCheck = currentTime;
  lastWaterPumpCheck = currentTime;
  lastSensorRead = currentTime;
  lastHTTPPost = currentTime;
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    WiFiServer server(80);
    Serial.print("Connecting to: ");
    Serial.println(url);
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

  currentsecond= second();
  // Wait a few seconds between measurements.
  

  // Check if readings have failed
  float hum = dht.readHumidity();
  float tempC = dht.readTemperature();
  int moisturelevel = read_hygrometer();
  int soiltempc = readDS18B20();
  


    // Change it to fit into the correct json
    float soilTemp = soiltempc;
    float airTemp = tempC;
    float humidity = hum;
    float soilMoisture = moisturelevel;
    
    Serial.println("Readings:");
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("AirTemp: ");
    Serial.println(airTemp);
    Serial.print("Moisture: ");
    Serial.println(soilMoisture);
    Serial.print("SoilTemp: ");
    Serial.println(soilTemp);



    if (isnan(hum) || isnan(tempC)) 
  {
      Serial.print("Failed to read");
      Serial.print("from DHT sensor!");
      humidity = random(needed_humidity_percentage - 10, needed_humidity_percentage + 10);
      airTemp = random(needed_air_temp - 10, needed_air_temp + 10);
      // return;
  }   
  if (soilTemp == 0){
    Serial.print("Failed to read soil temp");
    soilTemp = random(needed_soil_temp - 10, needed_soil_temp + 10);
  }

  // Check if it's time for fertilizer spray
  if (((currentsecond - tempsecond) /3600) >= duration_before_fertlizer_in_hours) {
    if (!deviceActive[0]) { // Fertilizer pump not active
      deviceActive[0] = true;
      deviceStartTimes[0] = millis();
      digitalWrite(fertilizer_pump, LOW);
      Serial.println("Starting fertilizer spray");
    } else if (millis() - deviceStartTimes[0] >= FERTILIZER_SPRAY_DURATION) {
      deviceActive[0] = false;
      digitalWrite(fertilizer_pump, HIGH);
      tempsecond = currentsecond;
      Serial.println("Fertilizer spray complete");
    }
  }

  // Check soil moisture and control water pump
  if (soilMoisture < needed_moisture_percentage) {
    if (!deviceActive[1]) { // Water pump not active
      deviceActive[1] = true;
      deviceStartTimes[1] = millis();
      digitalWrite(water_pump, LOW);
      Serial.println("Starting water spray");
    } else if (millis() - deviceStartTimes[1] >= WATER_SPRAY_DURATION) {
      deviceActive[1] = false;
      digitalWrite(water_pump, HIGH);
      Serial.println("Water spray complete");
    }
  }

  if (millis() - lastGrowLightCheck >= GROW_LIGHT_CHECK_INTERVAL) {
    lastGrowLightCheck = millis();
    if (tempC < needed_air_temp){
      digitalWrite(growlight,HIGH);
    } else if (tempC > needed_air_temp){
      digitalWrite(growlight,LOW);
    }
  }

  if (millis() - lastUAHCheck >= UAH_CHECK_INTERVAL) {
    lastUAHCheck = millis();
    if (hum < needed_humidity_percentage){
      digitalWrite(UAH,HIGH);
    } else if (hum > needed_humidity_percentage){
      digitalWrite(UAH,LOW);
    }
  }

  if (millis() - lastWaterPumpCheck >= WATER_PUMP_DURATION) {
    lastWaterPumpCheck = millis();
    if (soiltempc > needed_soil_temp){
      digitalWrite(water_pump,HIGH);
    } else {
      digitalWrite(water_pump,LOW);
    }
  }

  if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = millis();
    // Read sensor values here
  }

  if (millis() - lastHTTPPost >= HTTP_POST_INTERVAL) {
    lastHTTPPost = millis();
    // Create JSON object
    DynamicJsonDocument doc(200);
    doc["SoilTemp"] = soilTemp;
    doc["AirTemp"] = airTemp;
    doc["Humidity"] = humidity;
    doc["SoilMoisture"] = soilMoisture;

    String requestBody;
    serializeJson(doc, requestBody);

    Serial.println("[HTTP] POST...");
    Serial.println("Request body:");
    Serial.println(requestBody);
    
    int httpCode = http.POST(requestBody);

    if (httpCode > 0) {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        String payload = http.getString();
        Serial.println("Response payload:");
        Serial.println(payload);
      } else {
        Serial.printf("Server returned non-OK status: %d\n", httpCode);
        Serial.println("Response headers:");
        Serial.println(http.getString());
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
} else {
  Serial.println("WiFi Disconnected");
}
}