#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SwitecX25.h>

// Display settings
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// standard X25.168 ranges from 0 to 315 degrees, at 1/3 degree increments
#define STEPS 945

// For motors connected to D0, D1, D2, D3
SwitecX25 motor1(STEPS, D0, D1, D2, D3);

// BLE settings
static BLEUUID serviceUUID("d4d8b28b-8928-4044-b3b2-fbed8f587fd0");
static BLEUUID charUUID("c8e44563-c8f3-4822-8a41-9f4df10fa9ac");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

String receivedData = "";

void displayMessage(String message) {
  display.clearDisplay();
  display.setTextSize(1); // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0,0); // Start at top-left corner
  display.println(message);
  display.display();
}

void processData(String data) {
  Serial.println("Processed Data: " + data);
  // displayMessage("Data: " + data);
}

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    receivedData = String((char*)pData, length);
    Serial.println("Received Data: " + receivedData);
    processData(receivedData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
    displayMessage("Connected");
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    displayMessage("Disconnected");
  }
};

bool connectToServer() {
    Serial.print("Connecting to ");
    Serial.println(myDevice->getAddress().toString().c_str());

    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    pClient->connect(myDevice);
    Serial.println(" - Connected to server");

    pClient->setMTU(517);

    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("Found BLE Device: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = false;
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();
    // Initialize the stepper motor and move it to zero position
  motor1.zero(); // Make sure your motor library supports this or use motor.setPosition(0) if not
  while (motor1.currentStep != motor1.targetStep) {
    motor1.update();
  }

  // Now move the stepper motor to its maximum position
  motor1.setPosition(STEPS); // Set to the maximum steps the motor can take
  while (motor1.currentStep != motor1.targetStep) {
    motor1.update();
  }

  // Optionally, return the motor to its mid-point or starting position for your application
  motor1.setPosition(STEPS / 2);
  while (motor1.currentStep != motor1.targetStep) {
    motor1.update();
  }


  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void processReceivedData(String data) {
  // Initial indices for parsing the data
  int tempStart = data.indexOf("Temp: ") + 6;
  int humStart = data.indexOf("Hum: ", tempStart) + 5;
  int pressStart = data.indexOf("Press: ", humStart) + 7;
  int luxStart = data.indexOf("Lux: ", pressStart) + 5;

  // Find end indices for each segment based on known separators
  int tempEnd = data.indexOf(" C", tempStart);
  int humEnd = data.indexOf(" %", humStart);
  int pressEnd = data.indexOf(" hPa", pressStart);
  int luxEnd = data.length(); // Assumes lux is the last value in the string

  // Parse each segment to its corresponding float
  float temperature = data.substring(tempStart, tempEnd).toFloat();
  float humidity = data.substring(humStart, humEnd).toFloat();
  float pressure = data.substring(pressStart, pressEnd).toFloat();
  float lux = data.substring(luxStart, luxEnd).toFloat();

  int targetStep = (int)(temperature * (STEPS / 40.0));
  motor1.setPosition(targetStep);

  while (motor1.currentStep != motor1.targetStep) {
    motor1.update();
  }

  // Display parsed values on the OLED
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Temp: ");
  display.print(temperature, 1);
  display.print(" C");
  display.setCursor(0, 15);
  display.print("Hum: ");
  display.print(humidity, 1);
  display.print(" %");
  display.setCursor(0, 30);
  display.print("Pres: ");
  display.print(pressure, 2);
  display.print(" hPa");
  display.setCursor(0, 45); // Adjust as necessary for your display's resolution
  display.print("Lux: ");
  display.print(lux);
  display.display();
}


void loop() {
  if (doConnect) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
      displayMessage("Connected to Server");
    } else {
      Serial.println("Failed to connect to the server; there is nothing more we will do.");
      displayMessage("Failed to Connect");
    }
    doConnect = false;
  }

  if (connected) {
    // This part of the loop runs when the client is connected to the BLE server.
    // Display any other status or operation specific messages if required.
  } else if (doScan) {
    displayMessage("Scanning...");
    BLEDevice::getScan()->start(0); // 0 means continuous scanning
  }

  // Optionally, check for received data and display it.
  if (!receivedData.isEmpty()) {
    processReceivedData(receivedData);
    receivedData = ""; // Clear after displaying to avoid repetitive display
  }

  delay(1000); // Delay a second between loops to manage the workflow and not overload the BLE communication.
}
