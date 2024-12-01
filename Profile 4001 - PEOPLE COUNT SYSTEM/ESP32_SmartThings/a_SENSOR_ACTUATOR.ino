
void read_sensors(int task){

  boolean count_in = digitalRead(PIN_COUNT_IN);
  boolean count_out = digitalRead(PIN_COUNT_OUT);
  
  if(task == CLIENT){
    if(count_in){
      if((millis() - count_in_timestamp) >= TIME_TO_CONSIDER_COUNT){
        count_in_timestamp = millis();
        count_in_sensor ++;
        write_long_eeprom(COUNT_IN_EEPROM_ADDRESS, count_in_sensor);
        client_request_code = REQUEST_SENSOR_DATA;
        client_state = DEVICE_REQUEST;
      }
    }
    else{
   
    }
    if(count_out){
      if((millis() - count_out_timestamp) >= TIME_TO_CONSIDER_COUNT){
        count_out_timestamp = millis();
        count_out_sensor ++;
        write_long_eeprom(COUNT_OUT_EEPROM_ADDRESS, count_out_sensor);
        client_request_code = REQUEST_SENSOR_DATA;
        client_state = DEVICE_REQUEST;
      }
    }
    else{
      
    }
  }
}

void reset_counters(){
  count_in_sensor = 0;
  count_out_sensor = 0;
}

void change_actuator_state(){
  
}

void change_all_actuators(){
  boolean turnON = 1;
  int num_actuators = sizeof(actuator_state);
  
  for(int i = 0; i < num_actuators; i++){
    if(actuator_state[i]){
      turnON = 0;
      break;
    }
  }

  for(int i = 0; i < num_actuators; i++){
    actuator_state[i] = turnON;
    write_actuator_state_eeprom(turnON, i);
    digitalWrite(actuators_pins[i], turnON);
  }
  response_actuator_state = turnON;
  change_all = false;
}

void manual_actuator_state_change_handler(){
  
}

String get_actuators_state_repr(){

  return NO_SENSORS_OR_ACTUATORS;
}

String get_sensor_state_repr(){

  return String(count_in_sensor) + "," + String(count_out_sensor);
}
