
void read_sensors(int task){
  
  if(task == CLIENT){
    if((millis() - client_timestamp) >= CLIENT_WAIT_TIME){

      sensors.requestTemperatures();
      temperature = sensors.getTempCByIndex(0);
      
      if(temperature != temp_last_reading){
        temp_last_reading = temperature;
        client_state = DEVICE_REQUEST;
        client_request_code = REQUEST_ACTUATOR_STATE_CHANGED;
        client_timestamp = millis();
      }
    }
  }
  else{
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);
  }
}

void change_actuator_state(){
  //NOT USED
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
  //NOT USED
}

String get_actuators_state_repr(){
  
  return NO_SENSORS_OR_ACTUATORS;
}

String get_sensor_state_repr(){

  return String(temperature);
}
