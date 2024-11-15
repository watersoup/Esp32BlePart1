#ifndef BLE_SERVER_MANAGER_H
#define BLE_SERVER_MANAGER_H
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <vector>
#include <map>

class BLEServerManager {
public:
    BLEServerManager(const std::string& deviceName, const std::string& serviceUUID, const std::vector<std::string>& charUUIDs);

    void init();

    bool startAdvertising();

    bool isAdvertising();

    bool stopAdvertising();

    void handleClientData(BLECharacteristic *pChar);
    BLECharacteristic* getCharacteristic(int index) { return characteristics[index]; }
    int getNumClients() { if (clients.empty() ) return 0;
                            else  return clients.size(); }

private:
    std::string deviceName;
    std::string serviceUUID;
    std::vector<std::string> charUUIDs;
    BLEServer* pServer;
    std::vector<BLECharacteristic*> characteristics;
    std::vector<uint16_t> clients;
    std::map<uint16_t, BLECharacteristic*> clientCharMap;
    BLEAdvertising* pAdvertising;
    bool advertFlag = false;

};



class MyServerCallbacks : public BLEServerCallbacks {
public:
    MyServerCallbacks(std::vector<uint16_t>& clients);
    MyServerCallbacks(BLEServerManager *bleManager, std::vector<uint16_t>& clients, std::vector<BLECharacteristic*>& characteristics);

    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) override;

    void onDisconnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) override;

private:
    std::vector<uint16_t>& clients;
    std::vector<BLECharacteristic*>& characteristics;
    BLEServerManager *bleManager;

};


class ClientCallback : public BLECharacteristicCallbacks {
public:
    ClientCallback(BLEServerManager& serverManager);

    void onWrite(BLECharacteristic* pChar) override ;

private:
    BLEServerManager& serverManager;
};
#endif
