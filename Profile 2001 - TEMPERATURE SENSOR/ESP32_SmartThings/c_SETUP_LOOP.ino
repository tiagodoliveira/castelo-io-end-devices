void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_MAX_SIZE);
  bluetooth_setup();
  define_custom_device_variables();
}

void loop() {
  esp_main();
  server_behavior();
}
