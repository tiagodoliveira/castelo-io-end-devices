class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() == 1) {
        bt_received_char = value[0];
        debug("Received Char via BLE: ");
        debug(String(bt_received_char));
        bluetooth_request_handler(bt_received_char);
      }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      debug("Client Connected");
    };

    void onDisconnect(BLEServer* pServer) {
      debug("Client Disconnected");
    }
};

void bluetooth_setup(){
  debug("Connecting Bluetooth: " + device_name);
  BLEDevice::init(device_name.c_str());
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  
  BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  //BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);

  // Start advertising
  pServer->getAdvertising()->start();
  debug("Waiting for a BT client connection...");
}

boolean bluetooth_request_handler(char recieved_message){

  int message_code = recieved_message - '0';
  
  switch(message_code){
    
    case BT_REQUEST_MODE_CHANGE:{
      if(debug_active || server_state == AUTONOMOUS_MODE){
        autonomous_mode_state = CHANGE_MODE;
      }
      return true;}

    case BT_REQUEST_DEBUG_CHANGE:{
      if(debug_active){
        debug_active = !debug_active;
      }
      return true;}

    case BT_REQUEST_CLEAR_EEPROM:{
      if(debug_active){
        clear_eeprom();
        ESP.restart();
        has_configurations_saved = false;
        main_state = WIFI_SETUP;
      }
      return true;}

    case BT_REQUEST_OTA_UPDATE:{
      if(debug_active){
        OTA_Update_active = true;
        start_OTA_server();
        reset_OTA_timestamp();
      }
      return true;}

    default:{
      return false;}
  }
}

void debug(String val){
  if(debug_active){
    Serial.println(val);
  }
}

void debug(const char *val){
  if(debug_active){
    Serial.println(val);
  }
}

void debug(int val){
  if(debug_active){
    Serial.println(val);
  }
}
