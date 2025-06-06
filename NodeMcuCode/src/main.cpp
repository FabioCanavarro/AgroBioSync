/*
Todo:
- Add Multiprocessing to process the if conditions parallelly
- Add error handling for when the sensor fails to read
- Show error message on the website
*/

/*
command:
    cd NodeMcuCode; platformio.exe run --target upload; platformio.exe device monitor --baud 115200; cd ../
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
#define DHTPIN D3        // DHT22 data pin
#define HYGROMETER_PIN A0 // Soil moisture sensor pin
#define DS18B20_PIN D4    // Temperature sensor pin
#define UAH_PIN D5        //:w Ultrasonic Atomizer Humidifier pin
#define WATER_PUMP_PIN D8 // Water pump pin
#define FERTILIZER_PIN D7 // Fertilizer pump pin
#define GROW_LIGHT_PIN D0 // Grow light pin

// Sensor Setup
DHT dht(DHTPIN, DHT22);
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

// Environmental Thresholds
const uint16_t TARGET_AIR_TEMP = 21;
const uint16_t TARGET_SOIL_TEMP = 23;
const uint8_t TARGET_HUMIDITY = 65; // Should result in a percentage so doing this for reduced size
const uint8_t TARGET_MOISTURE = 20; // Should result in a percentage so doing this for reduced size
const uint8_t WIFI_RETRY_LIMIT = 5; // Should result in a percentage so doing this for reduced size

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
int8_t wifiRetryCount = 0;
bool wifiConnected = false;
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
bool isKillSwitchActive();

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
        setupWiFi();
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

/**
 * @brief Initializes all pins and sets initial states
 *
 * This function is called once in the setup() function to
 * initialize all pins and set initial states.
 *
 * - FERTILIZER_PIN: HIGH (off)
 * - WATER_PUMP_PIN: HIGH (off)
 * - GROW_LIGHT_PIN: LOW (on)
 * - UAH_PIN: LOW (on)
 */
void initializePins()
{
    // Set pin modes
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

/**
 * @brief Sets up WiFi using WiFiManager
 *
 * This function will attempt to connect to the specified WiFi network using the
 * provided password. If the connection fails, it will restart the ESP and reset
 * the WiFi settings.
 *
 * The WiFiManager library is used to manage the WiFi connection. It provides a
 * configuration portal that can be accessed by visiting the IP address of the
 * ESP in a web browser. The portal allows the user to configure the WiFi network
 * settings and set a password.
 *
 * The function will print a message to the serial console indicating whether the
 * connection was successful or not.
 *
 * @return None
 */
void setupWiFi()
{
    Serial.println("Event: Connecting to WiFi");
    wifiManager.setConfigPortalTimeout(1000000000);
    wifiManager.setBreakAfterConfig(false);

    while (!wifiConnected) {
        if (!wifiManager.autoConnect(WIFI_SSID, WIFI_PASSWORD))
        {
            if (wifiRetryCount < WIFI_RETRY_LIMIT)
            {
                Serial.println("Warning: Failed to connect to WiFi, retrying...");
                wifiConnected = false;
                wifiRetryCount +=1;
                continue;
            }
            else
            {
                Serial.println("Warning: Failed to connect");
                Serial.println("Important Event: Resetting WiFi settings and restarting ESP...");
                wifiManager.resetSettings();
                ESP.restart();
            }
        }
        else 
        {
            wifiConnected = true;
        }
    }
    Serial.println("Event: WiFi connected");
    Serial.println("\tIP address: " + WiFi.localIP().toString());
    server.begin();
}

/**
 * @brief Reads soil temperature using DallasTemperature library
 *
 * This function reads the temperature from the DS18B20 sensor connected to the
 * specified pin. It returns the temperature in Celsius.
 *
 * @return float: Temperature in Celsius
 */
float readSoilTemperature()
{
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
    return (temp != DEVICE_DISCONNECTED_C) ? temp : 0;
}

/**
 * @brief Reads soil moisture using analogRead
 *
 * This function reads the soil moisture level from the specified pin. It maps
 * the raw value to a percentage between 0 and 100.
 *
 * @return int: Soil moisture percentage
 */
int readSoilMoisture()
{
    float rawValue = analogRead(HYGROMETER_PIN);
    float valx = rawValue/ 1024.0;
    float percentage = (1 - valx) * 100;
    return percentage;
}
/*
 * @brief Checks if the kill switch is active
 *
 * This function reads the state of the kill switch pin and returns true if the
 * switch is active (HIGH), false otherwise.
 *
 * @return bool: true if kill switch is active, false otherwise
bool isKillSwitchActive()
{
    return digitalRead(KILL_SWITCH_PIN) == HIGH;
}
*/

/**
 * @brief Reads temperature and humidity using DHT22 sensor
 *
 * This function reads the temperature and humidity from the DHT22 sensor and
 * stores the values in the sensorData struct.
 */
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
        Serial.println("Warning: Failed to read from DHT sensor");
    }
    

    // Read soil sensors
    sensorData.soilTemp = readSoilTemperature();
    sensorData.soilMoisture = readSoilMoisture();

    if (sensorData.soilTemp == 0)
    {
        sensorData.soilTemp = random(TARGET_SOIL_TEMP - 10, TARGET_SOIL_TEMP + 10);
        Serial.println("Warning: Failed to read soil temperature");
    }
}

/**
 * @brief Controls devices based on sensor readings and kill switch state
 *
 * This function checks the state of the kill switch and controls the devices
 * accordingly. It also manages the timing for fertilizer, water pump, grow light,
 * and UAH based on the sensor readings.
 */
void controlDevices()
{
    // bool killSwitch = isKillSwitchActive();
    bool killSwitch = false;

    if (killSwitch) {
        Serial.println("Important Event: Kill switch activated");
        digitalWrite(FERTILIZER_PIN, HIGH);
        digitalWrite(WATER_PUMP_PIN, HIGH);
        digitalWrite(GROW_LIGHT_PIN, LOW);
        digitalWrite(UAH_PIN, LOW);
        return;
    }

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
            Serial.println("Important Event: Starting fertilizer spray");
        }
        else if (currentMillis - deviceStartTimes[0] >= FERTILIZER_SPRAY_DURATION)
        {
            deviceActive[0] = false;
            digitalWrite(FERTILIZER_PIN, HIGH);
            lastFertilizerCheck = currentMillis;
            Serial.println("Important Event: Fertilizer spray complete");
        }
    }

    // Water pump control
    if (sensorData.soilMoisture < TARGET_MOISTURE)
    {
        if (!deviceActive[1])
        {
            deviceActive[1] = true;
            deviceStartTimes[1] = currentMillis;
            digitalWrite(WATER_PUMP_PIN, HIGH);
            Serial.println("Important Event: Starting water spray");
        }
        else if (currentMillis - deviceStartTimes[1] >= WATER_SPRAY_DURATION)
        {
            deviceActive[1] = false;
            digitalWrite(WATER_PUMP_PIN, LOW);
            Serial.println("Important Event: Water spray complete");
        }
    }

    // Grow light control
    if (currentMillis - lastGrowLightCheck >= GROW_LIGHT_CHECK_INTERVAL)
    {
        lastGrowLightCheck = currentMillis;
        digitalWrite(GROW_LIGHT_PIN, sensorData.airTemp < TARGET_AIR_TEMP ? HIGH : LOW);

        if (sensorData.airTemp < TARGET_AIR_TEMP && !deviceActive[2])
        {
            deviceActive[2] = true;
            Serial.println("Important Event: Activating grow light");
        }
        else if (sensorData.airTemp >= TARGET_AIR_TEMP && deviceActive[2])
        {

            deviceActive[2] = false;
            Serial.println("Important Event: Deactivating grow light");
        }
    }

    // UAH control
    if (currentMillis - lastUAHCheck >= UAH_CHECK_INTERVAL)
    {
        lastUAHCheck = currentMillis;
        digitalWrite(UAH_PIN, sensorData.humidity < TARGET_HUMIDITY ? HIGH : LOW);

        if (sensorData.humidity < TARGET_HUMIDITY && !deviceActive[3])
        {

            deviceActive[3] = true;
            Serial.println("Important Event: Activating UAH");
        }
        else if (sensorData.humidity >= TARGET_HUMIDITY && deviceActive[3])
        {
            deviceActive[3] = false;
            Serial.println("Important Event: Deactivating UAH");
        }
    }

}

/**
 * @brief Sends sensor data to the server using HTTP POST
 *
 * This function sends the sensor data to the specified server URL using an
 * HTTP POST request. It uses the ESP8266HTTPClient library to handle the
 * HTTP communication.
 */
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

    Serial.print("Event: Sending data: ");
    Serial.println(requestBody);

    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED)
        {
            String payload = http.getString();
            Serial.println("Event: Data sent successfully");
        }
        else
        {
            Serial.printf("Warning: HTTP error: %d\n", httpCode);
        }
    }
    else
    {
        Serial.printf("Warning: HTTP request failed: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}
