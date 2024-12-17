HTTPClient http_client;
/*
 * 
 */
int send_device_info_to_main_server() {

  http_client.begin(main_server_address + "/" + get_MAC_as_ID());
  http_client.addHeader("Content-Type", "application/json");

  int http_response_code = http_client.GET();

  if(http_response_code == HTTP_CODE_OK){
    //TODO set devie variables returned by server
    return http_response_code;
  }

  http_client.setURL(main_server_address);
  http_client.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(256);

  doc["endDeviceMac"] = get_MAC_as_ID();
  doc["endDeviceIp"] = String(WiFi.localIP().toString());

  JsonObject endDeviceModel = doc.createNestedObject("endDeviceModel");
  endDeviceModel["modelId"] = DEVICE_PROFILE;

  JsonObject user = doc.createNestedObject("user");
  user["userId"] = user_id;

  doc["endDeviceName"] = device_name;
  doc["debugMode"] = debug_active;

  JsonObject gateway = doc.createNestedObject("gateway");
  gateway["gatewayMac"] = gateway_mac_address;
  
  doc["firmware"] = firmware_version;
  doc["workingMode"] = get_working_mode_string(working_mode);

  String json_request;
  serializeJson(doc, json_request);

  debug("ESP32 -> Sending POST request to Server on " + main_server_address);
  debug(json_request);

  http_response_code = http_client.POST(json_request);
  debug("Response from Server: " + http_client.getString());

  if (http_response_code > HTTP_CODE_CREATED) {
    if(deep_sleep_on){
      debug("Going to sleep... Wake me up when September ends...");
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1);
      esp_deep_sleep_start();
    }
  }
  else {
    debug("Error on sending POST: ");
    debug(http_client.errorToString(http_response_code));
  }
  http_client.end();

  return http_response_code;
}



boolean start_OTA_server() {
  //return index page which is stored in serverIndex
  // TODO This will be deleted after automatic gateway implementation
  http_server.on("/", HTTP_GET, []() {
    http_server.sendHeader("Connection", "close");
    http_server.send(200, "text/html", serverIndex);
  });

  //handling uploading firmware file
  http_server.on("/update", HTTP_POST, []() {
    http_server.sendHeader("Connection", "close");
    http_server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    
    HTTPUpload& upload = http_server.upload();
    
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
  http_server.begin();

  return true;
}
