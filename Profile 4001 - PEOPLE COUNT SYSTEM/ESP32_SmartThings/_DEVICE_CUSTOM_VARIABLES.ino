/*
 * Define here your device's custom variables,
 * that will depend on the implementation of the device.
 * Define how many actuators and sensors your device has,
 * how long some timestamps will be, and other important variables.
 */
 
String firmware_version = "1.1";
String DEVICE_PROFILE = "4001";

// Indicates if the device will work in deep sleep mode.
boolean deep_sleep_on = false;

//If the device doesn't have any actuators leave the array empty,
//otherwise, define the initial states of your actuators.
boolean actuator_state[] = {};

const int PIN_COUNT_IN = 32;
const int PIN_COUNT_OUT = 33;

//If the device doesn't have any actuators leave the array empty,
//otherwise, define the digital pins of your actuators.
int actuators_pins[] = {};

unsigned long count_in_timestamp = 0;
unsigned long count_out_timestamp = 0;
const int TIME_TO_CONSIDER_COUNT = 250;

unsigned long count_in_sensor = 0;
unsigned long count_out_sensor = 0;

int COUNT_IN_EEPROM_ADDRESS = 140;
int COUNT_OUT_EEPROM_ADDRESS = 144;

void set_actuators_state(){
}
 
void define_custom_device_variables(){

  pinMode(ON_BOARD_LED, OUTPUT);
  pinMode(PIN_COUNT_IN, INPUT);
  pinMode(PIN_COUNT_OUT, INPUT);

  count_in_sensor = read_long_eeprom(COUNT_IN_EEPROM_ADDRESS);
  count_out_sensor = read_long_eeprom(COUNT_OUT_EEPROM_ADDRESS);

  // Time in millis that takes the device in 
  // Autonomous mode to read the sensors again
  autonomous_max_time = 1000; 

  working_mode = MANUAL_MODE;
}
