#include <BLEServerManager.h>
#include <Arduino.h>


MyServerCallbacks::MyServerCallbacks(BLEServerManager *bleManager, std::vector<uint16_t>& clients, 
                std::vector<BLECharacteristic*>& characteristics)
                        : bleManager(bleManager),clients(clients), characteristics(characteristics) {}

void MyServerCallbacks::onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param)  {
    
    
    uint16_t connId = param->connect.conn_id;        
    char clientAddress[18];
    sprintf(clientAddress, "%02X:%02X:%02X:%02X:%02X:%02X",
        param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
        param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
    
    clients.push_back(connId);
    Serial.print("Client connected: ");
    Serial.println(clientAddress);

    // Assign a characteristic to the client
    if (clients.size() <= characteristics.size()) {
        BLECharacteristic* assignedChar = characteristics[clients.size() - 1];
        std::string value = "Connected: " + std::string(clientAddress);
        assignedChar->setValue(value);
        assignedChar->notify();
    }

    // if not advertising then restart advertising
    bleManager->startAdvertising();

}

void MyServerCallbacks::onDisconnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param)  {
    uint16_t connId = param->disconnect.conn_id;
    clients.erase(std::remove(clients.begin(), clients.end(), connId), clients.end());
    Serial.print("Client disconnected: ");
    Serial.println(connId);
}


BLEServerManager::BLEServerManager(const std::string& deviceName, const std::string& serviceUUID, 
            const std::vector<std::string>& charUUIDs):deviceName(deviceName),
                serviceUUID(serviceUUID), charUUIDs(charUUIDs){}

void BLEServerManager::init() {
    BLEDevice::init(deviceName);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks(this ,clients, characteristics));

    BLEService* pService = pServer->createService(serviceUUID);

    for (const auto& charUUID : charUUIDs) {
        BLECharacteristic* pCharacteristic = pService->createCharacteristic(
            charUUID,
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_READ
        );

        BLEDescriptor* pDescr = new BLEDescriptor((uint16_t)0x2901);
        pDescr->setValue("Notify");
        pCharacteristic->addDescriptor(pDescr);

        BLE2902* pBLE2902 = new BLE2902();
        pBLE2902->setNotifications(true);
        pCharacteristic->addDescriptor(pBLE2902);

        characteristics.push_back(pCharacteristic);
    }

    pService->start();
    pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(serviceUUID);
    pAdvertising->start();
    advertFlag = true;
    Serial.println("Advertising ... ");
}

bool BLEServerManager::startAdvertising() {
    if(pAdvertising){
        pAdvertising->start();
        advertFlag = true;
        return true;
    }
    return false;
}
bool BLEServerManager::isAdvertising() {
    return advertFlag;
}

bool BLEServerManager::stopAdvertising() {
    if(pAdvertising){
        pAdvertising->stop();
        advertFlag = false;
        return true;
    }
    return false;
}   
void BLEServerManager::handleClientData(BLECharacteristic* pChar) {
    std::string pChar_value_stdstr = pChar->getValue();
    String pChar_value_string = String(pChar_value_stdstr.c_str());
    Serial.println(pChar_value_string);
}

ClientCallback::ClientCallback(BLEServerManager& serverManager) : serverManager(serverManager) {}

void ClientCallback::onWrite(BLECharacteristic* pChar) {
    // Handle the data sent by the client
    Serial.println("Data received from client");
    serverManager.handleClientData(pChar);
}

