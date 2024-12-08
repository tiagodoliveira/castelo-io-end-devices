
/*
 * 
 */
int send_device_info_to_main_server() {
  
  HTTPClient http;

  http.begin(main_server_address);
  http.addHeader("Content-Type", "application/json");
  
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

  int http_response_code = http.POST(json_request);
  debug("Response from Server: " + http.getString());

  if (http_response_code > HTTP_CODE_CREATED) {
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

void start_MDNS_server(){
  // use multicast DNS to create host name address
  // https://tttapa.github.io/ESP8266/Chap08%20-%20mDNS.html

  debug(device_name);
  if (!MDNS.begin(device_name.c_str())) { //http://device_name.local
    debug("Error setting up MDNS responder!");
  }
  debug("MDNS responder started");
}

boolean start_OTA_server() {
  //return index page which is stored in serverIndex
  // TODO This will be deleted after automatic gateway implementation
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  //handling uploading firmware file
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    
    HTTPUpload& upload = server.upload();
    
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
  server.begin();

  return true;
}
