String get_MAC_as_ID() {

  uint8_t mac[6];
  // Get MAC address for WiFi station
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  
  char mac_char[18] = {0};
  sprintf(mac_char, "%02X:%02X:%02X:%02X:%02X:%02X", 
  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  
  return String(mac_char);
}

void board_led_blink(int max_time){
  if((millis() - blink_timestamp) >= max_time){
    digitalWrite(ON_BOARD_LED, board_led_state);
    board_led_state = !board_led_state;
    blink_timestamp = millis();
  }
}



void reset_OTA_timestamp(){
  OTA_timestamp = millis();
}

void reset_request_actuator_timestamp(){
  request_actuator_timestamp = millis();
}

void reset_autonomous_timestamp(){
  autonomous_timestamp = millis();
}

void reset_client_timestamp(){
  client_timestamp = millis();
}

void reset_reconnect_tcp_timestamp(){
  reconnect_tcp_timestamp = millis();
}

void reset_post_request_timestamp(){
  post_request_timestamp = millis();
}
