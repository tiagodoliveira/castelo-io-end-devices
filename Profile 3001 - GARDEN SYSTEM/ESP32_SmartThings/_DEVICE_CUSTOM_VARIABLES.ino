
/*
 * Define here your device's custom variables
 * that will depend on the implementation of the device
 * Define how many actuators and sensors your device has
 * How long some timestamps will be, and other important variables
 */
#include "DHT.h"
#define DHTTYPE DHT11

String firmware_version = "1.1";
String DEVICE_PROFILE = "3001";

// Indicates if the device will work in deep sleep mode
boolean deep_sleep_on = false;

boolean actuator_state[] = {0};

const int WATER_VALVE = 21;
const int AIR_MOISTURE_AND_TEMP = 36;
const int SOIL_MOISTURE = 34;
const int WATER_FLOW = 35;
const int MANUAL_WATER_VALVE_SWITCH = 33;

DHT dht(AIR_MOISTURE_AND_TEMP, DHTTYPE);

float ext_temperature;
float air_moisture;
int soil_moisture;
int water_flow; 

int actuators_pins[] = {WATER_VALVE};

unsigned long manual_button_timestamp = 0;
const int MANUAL_BUTTON_WAIT_TIME = 300;
const int CLIENT_WAIT_TIME = 1000;

void set_actuators_state(){
  digitalWrite(WATER_VALVE, actuator_state[0]);
}
 
void define_custom_device_variables(){

  pinMode(ON_BOARD_LED, OUTPUT);
  pinMode(WATER_VALVE, OUTPUT);
  pinMode(AIR_MOISTURE_AND_TEMP, INPUT);
  pinMode(SOIL_MOISTURE, INPUT);
  pinMode(WATER_FLOW, INPUT);
  pinMode(MANUAL_WATER_VALVE_SWITCH, INPUT);
  
  dht.begin();
  
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
