#include <Arduino.h>
/*
  Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
  Ported to Arduino ESP32 by Evandro Copercini
  updated by chegewara and MoThunderz
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include<BLEserverManager.h>

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-CBA987654321"
// Define the UUIDs for the characteristics
std::vector<std::string> charUUIDs = {
    "7fac1651-859b-4860-96b9-da21b4205ad9",
    "7fac1651-859b-4861-96b9-da21b4205ad9",
    "7fac1651-859b-4862-96b9-da21b4205ad9",
    "7fac1651-859b-4863-96b9-da21b4205ad9",
    "7fac1651-859b-4864-96b9-da21b4205ad9",
    "7fac1651-859b-4865-96b9-da21b4205ad9"
};
const int BUILTINPIN = 8;
void BLE_advert();

void blink(const int PIN, int times=1, long int freq=500){
  for(int i = 0; i < times; i++){
    digitalWrite(PIN, LOW);
    delay(freq);
    digitalWrite(PIN, HIGH);
    delay(freq);
  }
}

// Global instance of BLEServerManager
BLEServerManager bleServerManager("JRBL", SERVICE_UUID, charUUIDs);


void setup() {

  pinMode(BUILTINPIN,OUTPUT);
  blink(BUILTINPIN,2, 1000);
  Serial.begin(115200);

  bleServerManager.init();

}

void loop() {
    // notify changed value
    if (bleServerManager.getNumClients()!=0) {
        blink(BUILTINPIN, 1, 500);
        int randValue = random(100);
        String txValue = "";
        Serial.println("---> " + String(randValue));

        for (size_t i = 0; i < bleServerManager.getNumClients(); ++i) {
            txValue = "OK/" + String(i + 1) + ":" + String(randValue);
            bleServerManager.getCharacteristic(i)->setValue(txValue.c_str());
            bleServerManager.getCharacteristic(i)->notify();
        }
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        // don't restart advertisizing here, just notify through email or app about the 
        // loss of all client connections;
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    else if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    delay(30000);
}
