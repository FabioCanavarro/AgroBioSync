/*
Todo:
- Add Multiprocessing to process the if conditions parallelly
- Add error handling for when the sensor fails to read
- Show error message on the website
*/

/*
command:
    cd NodeMcuHTTPcode; platformio.exe run --target upload; platformio.exe device monitor --baud 115200, cd ../
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <DHT.h>
#include <TimeLib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Pin Definitions for NodeMCU v3 Lolin
#define DHTPIN D3         // DHT22 data pin
#define HYGROMETER_PIN D6 // Soil moisture sensor pin
#define DS18B20_PIN D4    // Temperature sensor pin
#define UAH_PIN D5        // Ultrasonic Atomizer Humidifier pin
#define WATER_PUMP_PIN D8 // Water pump pin
#define FERTILIZER_PIN D7 // Fertilizer pump pin
#define GROW_LIGHT_PIN D0 // Grow light pin

// Sensor Setup
DHT dht(DHTPIN, DHT22);
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

// Environmental Thresholds
const int TARGET_AIR_TEMP = 26;
const int TARGET_SOIL_TEMP = 29;
const int TARGET_HUMIDITY = 60;
const int TARGET_MOISTURE = 70;

// Hygrometer Calibration Values
const int HYGROMETER_AIR_VALUE = 561;   // Reading in air
const int HYGROMETER_WATER_VALUE = 310; // Reading in water

// Timing Constants (milliseconds)
const unsigned long FERTILIZER_INTERVAL_HOURS = 72;
const unsigned long FERTILIZER_SPRAY_DURATION = 900; // 0.9 seconds
const unsigned long WATER_SPRAY_DURATION = 900;      // 0.9 seconds
const unsigned long GROW_LIGHT_CHECK_INTERVAL = 200; // 0.2 seconds
const unsigned long UAH_CHECK_INTERVAL = 200;        // 0.2 seconds
const unsigned long WATER_PUMP_DURATION = 600;       // 0.6 seconds
const unsigned long SENSOR_READ_INTERVAL = 2000;     // 2 seconds
const unsigned long HTTP_POST_INTERVAL = 2000;       // 2 seconds

// Timer Variables
unsigned long lastFertilizerTime = 0;
unsigned long lastWaterSprayTime = 0;
unsigned long lastGrowLightCheck = 0;
unsigned long lastUAHCheck = 0;
unsigned long lastWaterPumpCheck = 0;
unsigned long lastSensorRead = 0;
unsigned long lastHTTPPost = 0;
unsigned long deviceStartTimes[6] = {0};
bool deviceActive[6] = {false};

// Sensor Readings
struct SensorData
{
    float soilTemp;
    float airTemp;
    float humidity;
    float soilMoisture;
} sensorData;

// Network Configuration
const char *API_URL = "https://agrobiosync.netlify.app//api/sensor-data";
const char *WIFI_SSID = "AgroBioSync";
const char *WIFI_PASSWORD = "GreenSync";
WiFiManager wifiManager;
WiFiServer server(80);

// Function Prototypes
float readSoilTemperature();
int readSoilMoisture();
void initializePins();
void setupWiFi();
void readSensors();
void controlDevices();
void sendDataToServer();

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting NodeMCU \n");
    Serial.println("Setting up sensors and wifi");
    initializePins();
    setupWiFi();
    sensors.begin();
    dht.begin();
    Serial.println("Setup complete \n");
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi Disconnected");
        ESP.restart();
    }

    unsigned long currentMillis = millis();

    // Read sensors at regular intervals
    if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL)
    {
        lastSensorRead = currentMillis;
        readSensors();
    }

    // Control devices based on sensor readings
    controlDevices();

    // Send data to server
    if (currentMillis - lastHTTPPost >= HTTP_POST_INTERVAL)
    {
        lastHTTPPost = currentMillis;
        sendDataToServer();
    }
}

void initializePins()
{
    pinMode(GROW_LIGHT_PIN, OUTPUT);
    pinMode(WATER_PUMP_PIN, OUTPUT);
    pinMode(FERTILIZER_PIN, OUTPUT);
    pinMode(UAH_PIN, OUTPUT);

    // Set initial states (HIGH is OFF for pumps)
    digitalWrite(FERTILIZER_PIN, HIGH);
    digitalWrite(WATER_PUMP_PIN, HIGH);
    digitalWrite(GROW_LIGHT_PIN, LOW);
    digitalWrite(UAH_PIN, LOW);
}

void setupWiFi()
{
    Serial.println("Connecting to WiFi");
    wifiManager.setConfigPortalTimeout(1000000000);
    wifiManager.setBreakAfterConfig(false);

    if (!wifiManager.autoConnect(WIFI_SSID, WIFI_PASSWORD))
    {
        Serial.println("Failed to connect");
        ESP.restart();
    }

    Serial.println("WiFi connected");
    Serial.println("IP address: " + WiFi.localIP().toString());
    server.begin();
}

float readSoilTemperature()
{
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
    return (temp != DEVICE_DISCONNECTED_C) ? temp : 0;
}

int readSoilMoisture()
{
    int rawValue = analogRead(HYGROMETER_PIN);
    int percentage = map(rawValue, HYGROMETER_AIR_VALUE, HYGROMETER_WATER_VALUE, 0, 100);
    return constrain(percentage, 0, 100);
}

void readSensors()
{
    // Read DHT22 sensor
    sensorData.humidity = dht.readHumidity();
    sensorData.airTemp = dht.readTemperature();

    // Handle DHT22 read errors
    if (isnan(sensorData.humidity) || isnan(sensorData.airTemp))
    {
        sensorData.humidity = random(TARGET_HUMIDITY - 10, TARGET_HUMIDITY + 10);
        sensorData.airTemp = random(TARGET_AIR_TEMP - 10, TARGET_AIR_TEMP + 10);
        Serial.println("Failed to read from DHT sensor");
    }

    // Read soil sensors
    sensorData.soilTemp = readSoilTemperature();
    sensorData.soilMoisture = readSoilMoisture();

    if (sensorData.soilTemp == 0)
    {
        sensorData.soilTemp = random(TARGET_SOIL_TEMP - 10, TARGET_SOIL_TEMP + 10);
        Serial.println("Failed to read soil temperature");
    }
}

void controlDevices()
{
    unsigned long currentMillis = millis();

    // Fertilizer control
    static unsigned long lastFertilizerCheck = 0;
    if (currentMillis - lastFertilizerCheck >= FERTILIZER_INTERVAL_HOURS * 3600000UL)
    {
        if (!deviceActive[0])
        {
            deviceActive[0] = true;
            deviceStartTimes[0] = currentMillis;
            digitalWrite(FERTILIZER_PIN, LOW);
            Serial.println("Starting fertilizer spray");
        }
        else if (currentMillis - deviceStartTimes[0] >= FERTILIZER_SPRAY_DURATION)
        {
            deviceActive[0] = false;
            digitalWrite(FERTILIZER_PIN, HIGH);
            lastFertilizerCheck = currentMillis;
            Serial.println("Fertilizer spray complete");
        }
    }

    // Water pump control
    if (sensorData.soilMoisture < TARGET_MOISTURE)
    {
        if (!deviceActive[1])
        {
            deviceActive[1] = true;
            deviceStartTimes[1] = currentMillis;
            digitalWrite(WATER_PUMP_PIN, LOW);
            Serial.println("Starting water spray");
        }
        else if (currentMillis - deviceStartTimes[1] >= WATER_SPRAY_DURATION)
        {
            deviceActive[1] = false;
            digitalWrite(WATER_PUMP_PIN, HIGH);
            Serial.println("Water spray complete");
        }
    }

    // Grow light control
    if (currentMillis - lastGrowLightCheck >= GROW_LIGHT_CHECK_INTERVAL)
    {
        lastGrowLightCheck = currentMillis;
        digitalWrite(GROW_LIGHT_PIN, sensorData.airTemp < TARGET_AIR_TEMP ? HIGH : LOW);
    }

    // UAH control
    if (currentMillis - lastUAHCheck >= UAH_CHECK_INTERVAL)
    {
        lastUAHCheck = currentMillis;
        digitalWrite(UAH_PIN, sensorData.humidity < TARGET_HUMIDITY ? HIGH : LOW);
    }
}

void sendDataToServer()
{
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    http.begin(client, API_URL);
    http.addHeader("Content-Type", "application/json");

    // Create JSON document
    StaticJsonDocument<200> doc;
    doc["SoilTemp"] = sensorData.soilTemp;
    doc["AirTemp"] = sensorData.airTemp;
    doc["Humidity"] = sensorData.humidity;
    doc["SoilMoisture"] = sensorData.soilMoisture;

    String requestBody;
    serializeJson(doc, requestBody);

    int httpCode = http.POST(requestBody);

    Serial.print("Sending data: ");
    Serial.println(requestBody);

    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED)
        {
            String payload = http.getString();
            Serial.println("Data sent successfully");
        }
        else
        {
            Serial.printf("HTTP error: %d\n", httpCode);
        }
    }
    else
    {
        Serial.printf("HTTP request failed: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}