void connect_to_wifi() {

  WiFi.softAPdisconnect(true);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  
  debug("Waiting for WiFi connection...");
  
  while (WiFi.status() != WL_CONNECTED) {
    manual_actuator_state_change_handler();
    board_led_blink(WIFI_BLINK_MAX);
  }
  
  board_led_state = HIGH;
  digitalWrite(ON_BOARD_LED, board_led_state);

  WiFi_Server.begin();

  // WiFi stats:
  debug("WiFi Connected!");
  debug("SSID: " + String(wifi_ssid.c_str()));
  debug("Pass: " + String(wifi_password.c_str()));
  debug("IP address: " + String(WiFi.localIP().toString())); 
  debug("Signal strength: " + String(WiFi.RSSI()) + " dBm\n"); 
  save_network_settings();
  
  main_state = DEVICE_CONNECTION;
}

void init_wifi_setup() {

  //Init WiFi as Station, start SmartConfig
  WiFi.softAP(device_name.c_str());

  debug("AP IP address: ");
  debug(WiFi.softAPIP().toString());

  // Handle POST requests at /credentials
  http_server.on("/credentials", HTTP_POST, [](){

    debug("Received /credentials HTTP Request");
    // Print all form data
    for (int i = 0; i < http_server.args(); i++) {
      debug(http_server.argName(i) + ":");
      debug(http_server.arg(i));
    }

    if (http_server.hasArg("ssid") && http_server.hasArg("password") && http_server.hasArg("userId")
        && http_server.hasArg("mainServerAddress") && http_server.hasArg("telemetryServerAddress")) {

      wifi_ssid = http_server.arg("ssid");
      wifi_password = http_server.arg("password");
      user_id = http_server.arg("userId");
      main_server_address = http_server.arg("mainServerAddress");
      telemetry_server_address = http_server.arg("telemetryServerAddress");

      if(http_server.hasArg("gatewayMac") && http_server.hasArg("gatewayIp")){
        gateway_mac_address = http_server.arg("gatewayMac");
        if(!gateway_address.fromString(http_server.arg("gatewayIp"))){
          http_server.send(400, "text/plain", "Invalid request. The Gateway IP provided is invalid.");
        }
      }

      if(http_server.hasArg("deviceName")){
        device_name = http_server.arg("deviceName");
        esp_bt_dev_set_device_name(device_name.c_str());
      }

      debug("Connecting to " + wifi_ssid + " WiFi Network");
      http_server.send(200, "text/plain", "Connected successfully!");
      main_state = CONNECT_WIFI;
    } else {
      http_server.send(400, "text/plain", "Invalid request. Some mandatory parameters are missing.");
    }
  });

  // Start the server
  http_server.begin();
}
