#include "DHT.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <WiFi.h>
#include <WebServer.h>

#define DHTPIN 13      // Pin which is connected to the DHT11 sensor
#define DHTTYPE DHT11  // Define the type of DHT sensor
#define KNOCKPIN 16    // Pin on which is connected knocking sensor

DHT dht(DHTPIN, DHTTYPE);

// WiFi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
WebServer server(80);

// Global variables for sensor values
String temperature = "N/A";
String humidity = "N/A";
String knocking = "Nikdo neklepe";
unsigned long lastKnockTime = 0;
unsigned long lastDataReceived = 0;

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;

// The remote service we wish to connect to.
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Device connected");
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Device disconnected");
      BLEDevice::startAdvertising();
    }
};

String getTimeAgo() {
    if (lastDataReceived == 0) return "No data received yet";
    
    unsigned long diff = millis() - lastDataReceived;
    if (diff < 1000) return "teď";
    if (diff < 60000) return String(diff/1000) + "před pár sekundami";
    if (diff < 3600000) return String(diff/60000) + "před pár minutami";
    return String(diff/3600000) + "před pár hodinami";
}

void handleRoot() {
    // Reset knock status after 3 seconds
    if (lastKnockTime > 0 && millis() - lastKnockTime > 3000) {
        knocking = "Nikdo neklepe";
        lastKnockTime = 0;
    }
    
    String html = "<html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f0f0f0; }";
    html += ".grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; }";
    html += ".tile { background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }";
    html += ".value { font-size: 36px; margin: 10px 0; word-break: break-all; text-align: center; }";
    html += ".label { color: #666; font-size: 16px; font-weight: bold; text-align: center; }";
    html += ".temp { color: #FF5722; }";
    html += ".humid { color: #2196F3; }";
    html += ".knock { color: #4CAF50; }";
    html += ".time { font-size: 12px; color: #666; text-align: center; margin-top: 10px; }";
    html += "</style></head><body>";
    html += "<div class='grid'>";
    
    html += "<div class='tile'>";
    html += "<div class='label'>Teplota</div>";
    html += "<div class='value temp'>" + temperature + " °C</div>";
    html += "<div class='time'>Poslední aktualizace: " + getTimeAgo() + "</div>";
    html += "</div>";
    
    html += "<div class='tile'>";
    html += "<div class='label'>Vlhkost</div>";
    html += "<div class='value humid'>" + humidity + " %</div>";
    html += "<div class='time'>Poslední aktualizace: " + getTimeAgo() + "</div>";
    html += "</div>";
    
    html += "<div class='tile'>";
    html += "<div class='label'>Klepání</div>";
    html += "<div class='value knock'>" + knocking + "</div>";
    html += "<div class='time'>Poslední aktualizace: " + (lastKnockTime > 0 ? getTimeAgo() : "N/A") + "</div>";
    html += "</div>";
    
    html += "</div>";
    html += "<script>setTimeout(function(){location.reload()},1000);</script>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void setup() {
    Serial.begin(115200);
    dht.begin();
    pinMode(KNOCKPIN, INPUT);

    // Initialize WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi. IP: ");
    Serial.println(WiFi.localIP());

    // Setup web server
    server.on("/", handleRoot);
    server.begin();

    // Create the BLE Device
    BLEDevice::init("ESP32 Sensor");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(serviceUUID);
    pCharacteristic = pService->createCharacteristic(
        charUUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(serviceUUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();
}

void loop() {
    // Read sensors
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    bool isKnocking = !digitalRead(KNOCKPIN);

    // Update knocking status regardless of other sensor readings
    if (isKnocking) {
        knocking = "Klepání!";
        lastKnockTime = millis();
        lastDataReceived = millis();
    } else if (lastKnockTime > 0 && millis() - lastKnockTime > 3000) {
        knocking = "Nikdo neklepe";
        lastKnockTime = 0;
    }

    // Update temperature and humidity if readings are valid
    if (!isnan(h) && !isnan(t)) {
        temperature = String(t);
        humidity = String(h);
        lastDataReceived = millis();
    }
    
    // Update BLE characteristic with all values
    String newValue = "T:" + temperature + ",H:" + humidity + ",K:" + String(isKnocking);
    pCharacteristic->setValue(newValue.c_str());
    pCharacteristic->notify();

    server.handleClient();
    delay(1000);
}
