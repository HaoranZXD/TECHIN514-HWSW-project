#include <Arduino.h>
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VEML7700.h>

// BLE settings
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 1000;

// UUIDs for BLE service and characteristic
#define SERVICE_UUID        "d4d8b28b-8928-4044-b3b2-fbed8f587fd0"
#define CHARACTERISTIC_UUID "c8e44563-c8f3-4822-8a41-9f4df10fa9ac"

// Sensor setup
Adafruit_VEML7700 veml = Adafruit_VEML7700();
Adafruit_BME280 bme;

// Filtered values initialization
float alpha = 0.2;
float filteredTemperature = 0.0;
float filteredHumidity = 0.0;
float filteredPressure = 0.0;
float filteredLux = 0.0;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Serial.println("Starting BLE work and sensor initialization!");

    // Initialize BME280
    if (!bme.begin(0x76)) {  
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }

    // Initialize VEML7700
    if (!veml.begin()) {
        Serial.println("No VEML7700 sensor found... check your wiring?");
        while (1);
    }

    // Configure VEML7700 settings
    veml.setGain(VEML7700_GAIN_1);
    veml.setIntegrationTime(VEML7700_IT_100MS);

    // BLE initialization
    BLEDevice::init("SensorServer");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->addDescriptor(new BLE2902());
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Sensors are ready!");
}

void loop() {
    // Read data from sensors
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F; // Convert Pa to hPa for readability
    float lux = veml.readLux();

    // Apply exponential decay filter to sensor readings
    filteredTemperature = alpha * temperature + (1 - alpha) * filteredTemperature;
    filteredHumidity = alpha * humidity + (1 - alpha) * filteredHumidity;
    filteredPressure = alpha * pressure + (1 - alpha) * filteredPressure;
    filteredLux = alpha * lux + (1 - alpha) * filteredLux;

    if (deviceConnected) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            // Prepare and send the BLE message
            char message[100];
            snprintf(message, sizeof(message), "Temp: %.1f C, Hum: %.1f %%, Press: %.1f hPa, Lux: %.1f", filteredTemperature, filteredHumidity, filteredPressure, filteredLux);
            pCharacteristic->setValue(message);
            pCharacteristic->notify();
            Serial.println(message);

            previousMillis = currentMillis;
        }
    }

    // Handle BLE connection status changes
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // Give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // Advertise again
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }

    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }

    delay(1000);
    } //
