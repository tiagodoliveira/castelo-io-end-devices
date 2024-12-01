
/*
 * 
 */
int send_device_info_to_main_server() {
  
  HTTPClient http;

  http.begin(main_server_address);
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(1024);
  doc["ip"] = String(WiFi.localIP().toString());
  doc["code"] = String(REQUEST_DEVICE_CONNECTION);
  doc["mac"] = get_MAC_as_ID();
  doc["profile"] = DEVICE_PROFILE;
  doc["version"] = firmware_version;
  doc["mode"] = String(working_mode);
  doc["sensors_states"] = get_sensor_state_repr();
  doc["actuators_states"] = get_actuators_state_repr();
  String json_request;
  serializeJson(doc, json_request);

  debug("ESP32 -> Sending POST request to Server");

  int http_response_code = http.POST(json_request);

  if (http_response_code > 0) {
    
    device_name = http.getString(); //Get the name given to the device
    
    debug("Response from Server: " + String(http_response_code));
    debug("Device name: " + device_name);
       
    esp_bt_dev_set_device_name(device_name.c_str());
    
    if(deep_sleep_on){
      debug("Going to sleep... Wake me up when September ends...");
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);
      esp_deep_sleep_start();
    }
  }
  else {
    debug("Error on sending POST: ");
    debug(http.errorToString(http_response_code));
  }
  http.end();

  return http_response_code;
}

boolean start_MDNS_server() {

  // use multicast DNS to create host name address
  // https://tttapa.github.io/ESP8266/Chap08%20-%20mDNS.html

  debug(device_name);
  if (!MDNS.begin(device_name.c_str())) { //http://device_name.local
    debug("Error setting up MDNS responder!");
    return false;
  }
  debug("MDNS responder started");

  //return index page which is stored in serverIndex
  // TODO This will be deleted after automatic gateway implementation
  OTA_server.on("/", HTTP_GET, []() {
    OTA_server.sendHeader("Connection", "close");
    OTA_server.send(200, "text/html", serverIndex);
  });

  //handling uploading firmware file
  OTA_server.on("/update", HTTP_POST, []() {
    OTA_server.sendHeader("Connection", "close");
    OTA_server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    
    HTTPUpload& upload = OTA_server.upload();
    
    switch (upload.status) {
      
      case UPLOAD_FILE_START:
        debug("Update: " + String(upload.filename.c_str()));
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
          Update.printError(Serial);
        }
        break;

      case UPLOAD_FILE_WRITE:
        // Flashing firmware to ESP
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
        break;

      case UPLOAD_FILE_END:
        if (Update.end(true)) { //true to set the size to the current progress
          debug("File Size: " + String(upload.totalSize));
          debug("Update Success: \nRebooting...");
        } else {
          Update.printError(Serial);
        }
        break;
    }
  });
  OTA_server.begin();

  return true;
}
