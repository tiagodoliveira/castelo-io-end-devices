void connect_to_wifi() {
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  debug("Waiting for WiFi connection...");
  
  while (WiFi.status() != WL_CONNECTED) {
    manual_actuator_state_change_handler();
    board_led_blink(WIFI_BLINK_MAX);
    if(bluetooth_handle()){
      return;
    }
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

void init_smartconfig() {
  
  //Init WiFi as Station, start SmartConfig
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();

  //Wait for SmartConfig packet from App
  debug("Waiting for SmartConfig...");
  while (!WiFi.smartConfigDone()) {
    board_led_blink(SMARTCONFIG_BLINK_MAX);
    manual_actuator_state_change_handler();
    bluetooth_handle();
  }
  debug("SmartConfig received!");
}
