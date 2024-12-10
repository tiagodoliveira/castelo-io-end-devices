void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_MAX_SIZE);
  bluetooth_setup();
  define_custom_device_variables();
  mqtt_client.setServer(mqtt_server_url, mqtt_port);  // Set the MQTT broker details
  mqtt_client.setCallback(mqtt_callback);         // Set the message callback function
}

void loop() {
  esp_main();
  server_behavior();
}
