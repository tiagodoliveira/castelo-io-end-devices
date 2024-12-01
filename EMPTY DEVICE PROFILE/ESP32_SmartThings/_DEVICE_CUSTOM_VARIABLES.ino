
/*
 * Define here your device's custom variables
 * that will depend on the implementation of the device
 * Define how many actuators and sensors your device has
 * How long some timestamps will be, and other important variables
 */

String firmware_version = "1.1";
String DEVICE_PROFILE = "1001";

// Indicates if the device will work in deep sleep mode
boolean deep_sleep_on = false;

boolean actuator_state[] = {0};

//Set the default state of every actuator
void set_actuators_state(){
  //digitalWrite(RELAY_DIGITAL_OUT, actuator_state[0]);
}
 
void define_custom_device_variables(){

  pinMode(ON_BOARD_LED, OUTPUT);

  main_server_address = "http://192.168.43.246:8080/communication";

  // Time in millis that takes the device in Autonomous mode to read the sensors again
  autonomous_max_time = 1000; 

  // Time in millis that takes the device to repeat the HTTP POST request, 
  // with the device's profile information, to the main server
  post_request_max_time = 0;

  // Time in millis that the device will wait until trying to re-establish the TCP connection
  // with the Gateway. Only used in Client Behavior.
  reconnect_tcp_max_time = 5000;

  // Maximum time in millis that the Over the Air Update Server will wait for a new
  // firmware update file, sent by the Gateway.
  OTA_max_time = 120000;
    
  main_state = READ_EEPROM;
  client_state = READ_SENSORS;
  autonomous_mode_state = GATEWAY_REQUEST; 
  manual_mode_state = GATEWAY_REQUEST;
  working_mode = MANUAL_MODE;
}
