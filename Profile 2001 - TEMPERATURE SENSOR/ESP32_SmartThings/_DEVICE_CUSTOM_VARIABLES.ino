
/*
 * Define here your device's custom variables
 * that will depend on the implementation of the device
 * Define how many actuators and sensors your device has
 * How long some timestamps will be, and other important variables
 */

#include <OneWire.h>
#include <DallasTemperature.h>

String firmware_version = "1.1";
String DEVICE_PROFILE = "2001";
String device_name = "Castelo-io-SmartDevice";

// Indicates if the device will work in deep sleep mode
boolean deep_sleep_on = false;

boolean actuator_state[] = {};//NOT USED

const int SENSOR_PIN = 21;
float temperature;
float temp_last_reading;

OneWire oneWire(SENSOR_PIN);
DallasTemperature sensors(&oneWire);

const int CLIENT_WAIT_TIME = 1000;

int actuators_pins[] = {};//NOT USED

void set_actuators_state(){}//NOT USED

void define_custom_device_variables(){

  pinMode(ON_BOARD_LED, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);

  client_timestamp = millis();

  // Time in millis that takes the device in Autonomous mode to read the sensors again
  autonomous_max_time = 1000; 

  // Time in millis that takes the device to repeat the HTTP POST request, 
  // with the device's profile information, to the main server
  post_request_max_time = 0;

  // Time in millis that the device will wait until trying to re-establish the TCP connection
  // with the Gateway. Only used in Client Behavior.
  reconnect_tcp_max_time = 5000;

  reconnect_mqtt_max_time = 5000;
  
  // Maximum time in millis that the Over the Air Update Server will wait for a new
  // firmware update file, sent by the Gateway.
  OTA_max_time = 120000;
    
  main_state = READ_EEPROM;
  client_state = READ_SENSORS;
  autonomous_mode_state = GATEWAY_REQUEST; 
  manual_mode_state = GATEWAY_REQUEST;
  working_mode = MANUAL_MODE;
}
