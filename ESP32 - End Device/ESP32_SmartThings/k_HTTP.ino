void initiate_http_server(){

  // Setup /connect endpoint for client <-> endDevice connection
  http_server.on("/connect", HTTP_POST, [](){

    debug("Received /connect HTTP Request");
    // Print all form data
    for (int i = 0; i < http_server.args(); i++) {
      debug(http_server.argName(i) + ":");
      debug(http_server.arg(i));
    }

    if (http_server.hasArg("ssid") && http_server.hasArg("password") && http_server.hasArg("userId")
        && http_server.hasArg("mainServerAddress") && http_server.hasArg("telemetryServerAddress")) {

      wifi_ssid = http_server.arg("ssid");
      wifi_password = http_server.arg("password");
      telemetry_server_address = http_server.arg("telemetryServerAddress");

      if(http_server.hasArg("gatewayMac") && http_server.hasArg("gatewayIp")){
        gateway_mac_address = http_server.arg("gatewayMac");
        if(!gateway_address.fromString(http_server.arg("gatewayIp"))){ // Validates the provided IP Address 
          http_server.send(400, "text/plain", "Invalid request. The Gateway IP provided is invalid.");
        }
      }

      debug("Connecting to " + wifi_ssid + " WiFi Network");

      StaticJsonDocument<200> jsonResponse;
      jsonResponse["endDeviceMac"] = get_MAC_as_ID();
      jsonResponse["firmware"] = firmware_version;
      jsonResponse["workingMode"] = get_working_mode_string(working_mode);
      jsonResponse["debugMode"] = debug_active;

      JsonObject endDeviceModel = jsonResponse.createNestedObject("endDeviceModel");
      endDeviceModel["modelId"] = DEVICE_PROFILE;

      String response;
      serializeJson(jsonResponse, response);

      http_server.send(200, "application/json", response);

      main_state = CONNECT_WIFI;
    } else {
      StaticJsonDocument<200> errorJson;
      errorJson["error"] = "Invalid request. Some mandatory parameters are missing.";
      
      String errorResponse;
      serializeJson(errorJson, errorResponse);
      http_server.send(400, "application/json", errorResponse);
    }
  });

  // Start the server
  http_server.begin();
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
