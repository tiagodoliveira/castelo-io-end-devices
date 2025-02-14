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

  device_name = get_MAC_as_mDNS();
  esp_bt_dev_set_device_name(device_name.c_str());
  start_MDNS_server(device_name.c_str());
  
  main_state = DEVICE_CONNECTION;
}

void init_wifi_setup() {

  //Init WiFi as Station, start SmartConfig
  WiFi.softAP(device_name.c_str());

  debug("AP IP address: ");
  debug(WiFi.softAPIP().toString());

  initiate_http_server();
}


