void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_MAX_SIZE);  
  SerialBT.begin(device_name);
  define_custom_device_variables();
}

void loop() {
  bluetooth_handle();
  esp_main();
  server_behavior();
}
