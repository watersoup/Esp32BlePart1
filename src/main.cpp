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

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic_1 = NULL;
BLECharacteristic* pCharacteristic_2 = NULL;
BLEDescriptor *pDescr_1;
BLEDescriptor *pDescr_2;
BLE2902 *pBLE2902_1;
BLE2902 *pBLE2902_2;
BLEAdvertising *pAdvertising = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-CBA987654321"
#define CHARACTERISTIC_UUID_1 "7fac1651-859b-4865-96b9-da21b4205ad9"
#define CHARACTERISTIC_UUID_2 "7fac1651-859b-4861-96b9-da21b4205ad9"

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

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.printf( "Device is connected %f\n", pServer->getConnId());
      deviceConnected = true;
      blink(BUILTINPIN,5, 250);
      
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.printf(" Device is disconnected %f\n", pServer->getConnId());
      deviceConnected = false;
      blink(BUILTINPIN,2, 250);
      // start advertising here
      BLE_advert();
    }
};

class clientCallback : public BLECharacteristicCallbacks{
    // receive any information received from the client here
    void onWrite(BLECharacteristic * pChar) override{
        // in case its just a digits
        // uint8_t* received_data = pChar->getData();
        std::string pChar_value_stdstr = pChar->getValue();
        String pChar_value_string = String(pChar_value_stdstr.c_str());
        Serial.println(pChar_value_string);
        
    }
};

void init_BLE_server(int numchar =2){
  // Create the BLE Device
  BLEDevice::init("JRBL");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic_1 = pService->createCharacteristic(
      CHARACTERISTIC_UUID_1,
      BLECharacteristic::PROPERTY_NOTIFY);

  pCharacteristic_2 = pService->createCharacteristic(
      CHARACTERISTIC_UUID_2,
      BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY);
  // Create a BLE Descriptor
  pDescr_1 = new BLEDescriptor((uint16_t)0x2901);
  pDescr_1->setValue("Notify-1");
  pCharacteristic_1->addDescriptor(pDescr_1);

  pDescr_2 = new BLEDescriptor((uint16_t)0x2901);
  pDescr_2->setValue("Notify-2");
  pCharacteristic_2->addDescriptor(pDescr_2);

  pBLE2902_1 = new BLE2902();
  pBLE2902_1->setNotifications(true);
  pCharacteristic_1->addDescriptor(pBLE2902_1);

  pBLE2902_2 = new BLE2902();
  pBLE2902_2->setNotifications(true);
  pCharacteristic_2->addDescriptor(pBLE2902_2);

  pCharacteristic_2->setCallbacks(new clientCallback);

  pCharacteristic_1->setCallbacks(new clientCallback);

  // Start the service
  pService->start();
}

void BLE_advert(){
  // Start advertising
  // BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  if (pAdvertising == nullptr){
      Serial.println(" initiating Advertising class");
      pAdvertising = pServer->getAdvertising();
      pAdvertising->addServiceUUID(SERVICE_UUID);
      pAdvertising->setScanResponse(false);
      pAdvertising->setMinPreferred((uint16_t)5);  // set value to 0x00 to not advertise this parameter
  }
  pAdvertising->start();
  Serial.println("started advertising");
  // BLEDevice::startAdvertising();
}
void setup() {

  pinMode(BUILTINPIN,OUTPUT);
  blink(BUILTINPIN,2, 1000);
  Serial.begin(115200);

  init_BLE_server( 2);

  BLE_advert();

}

void loop() {
    // notify changed value
    if(deviceConnected) {
      // currently do nothing;
      blink(BUILTINPIN,1,500);
      /// send information back w/OK
      int randValue = random(100);
      String txValue = "";
      Serial.println("---> " + String(randValue));
      // if (pChar->getUUID().equals(BLEUUID(CHARACTERISTIC_UUID_1))){
          txValue = "OK/1:"+ String(randValue);
          pCharacteristic_1->setValue(txValue.c_str());
          pCharacteristic_1->notify();
      // } else if (pChar->getUUID().equals(BLEUUID(CHARACTERISTIC_UUID_2))){
          txValue = "OK/2:"+ String(randValue);
          pCharacteristic_2->setValue(txValue.c_str());
          pCharacteristic_2->notify();
      // }
      

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
