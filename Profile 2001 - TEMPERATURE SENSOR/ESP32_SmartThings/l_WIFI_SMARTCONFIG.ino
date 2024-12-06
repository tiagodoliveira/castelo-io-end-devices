void connect_to_wifi() {

  WiFi.softAPdisconnect(true);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  debug("Waiting for WiFi connection...");
  
  while (WiFi.status() != WL_CONNECTED) {
    manual_actuator_state_change_handler();
    board_led_blink(WIFI_BLINK_MAX);
  }
  save_network_settings();
  
  board_led_state = LOW;
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
  WiFi.softAP("Castelo-io-SmartDevice");

  debug("AP IP address: ");
  debug(WiFi.softAPIP().toString());

  // Handle POST requests at /credentials
  server.on("/credentials", HTTP_POST, [](){
    debug("Received /credentials HTTP Request");
    if (server.hasArg("ssid") && server.hasArg("password")) {
      ssid = server.arg("ssid");
      password = server.arg("password");
      debug("Connecting to " + ssid + " WiFi Network");
      server.send(200, "text/plain", "Connected successfully!");
      main_state = CONNECT_WIFI;
    } else {
      server.send(400, "text/plain", "Invalid request. SSID and password are required.");
    }
  });

  // Start the server
  server.begin();
}
