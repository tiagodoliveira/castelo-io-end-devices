void connect_to_wifi() {

  WiFi.softAPdisconnect(true);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  debug("Waiting for WiFi connection...");
  
  while (WiFi.status() != WL_CONNECTED) {
    manual_actuator_state_change_handler();
    board_led_blink(WIFI_BLINK_MAX);
  }
  save_network_settings();
  
  board_led_state = HIGH;
  digitalWrite(ON_BOARD_LED, board_led_state);

  WiFi_Server.begin();

  // WiFi stats:
  debug("WiFi Connected!");
  debug("SSID: " + String(ssid.c_str()));
  debug("Pass: " + String(password.c_str()));
  debug("IP address: " + String(WiFi.localIP().toString())); 
  debug("Signal strength: " + String(WiFi.RSSI()) + " dBm\n"); 

  main_state = DEVICE_CONNECTION;
}

void init_wifi_setup() {

  //Init WiFi as Station, start SmartConfig
  WiFi.softAP(device_name.c_str());

  debug("AP IP address: ");
  debug(WiFi.softAPIP().toString());

  // Handle POST requests at /credentials
  server.on("/credentials", HTTP_POST, [](){

    debug("Received /credentials HTTP Request");
    // Print all form data
    for (int i = 0; i < server.args(); i++) {
      debug(server.argName(i) + ":");
      debug(server.arg(i));
    }

    if (server.hasArg("ssid") && server.hasArg("password") && server.hasArg("userId")
        && server.hasArg("mainServerAddress") && server.hasArg("telemetryServerAddress")) {

      ssid = server.arg("ssid");
      password = server.arg("password");
      user_id = server.arg("userId");
      main_server_address = server.arg("mainServerAddress");
      telemetry_server_address = server.arg("telemetryServerAddress");

      if(server.hasArg("gatewayMac") && server.hasArg("gatewayIp")){
        gateway_mac_address = server.arg("gatewayMac");
        if(!gateway_address.fromString(server.arg("gatewayIp"))){
          server.send(400, "text/plain", "Invalid request. The Gateway IP provided is invalid.");
        }
      }

      if(server.hasArg("deviceName")){
        device_name = server.arg("deviceName");
        esp_bt_dev_set_device_name(device_name.c_str());
      }

      debug("Connecting to " + ssid + " WiFi Network");
      server.send(200, "text/plain", "Connected successfully!");
      main_state = CONNECT_WIFI;
    } else {
      server.send(400, "text/plain", "Invalid request. Some mandatory parameters are missing.");
    }
  });

  // Start the server
  server.begin();
}