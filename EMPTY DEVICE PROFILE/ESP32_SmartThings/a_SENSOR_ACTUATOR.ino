
void read_sensors(int task){
  
  if(task == CLIENT){
   
  }
  else{
    
  }
}

void change_actuator_state(){
  
  if(server_state == MANUAL_MODE){
    actuator_state[actuator_number] = !actuator_state[actuator_number];
    write_actuator_state_eeprom(actuator_state[actuator_number], actuator_number);
    // TODO digitalWrite();
  }
  else{
    actuator_state[actuator_number] = !actuator_state[actuator_number];
    write_actuator_state_eeprom(actuator_state[actuator_number], actuator_number);
    // TODO digitalWrite();
  }
  response_actuator_state = actuator_state[actuator_number];
}

void manual_actuator_state_change_handler(){
  /*
  boolean actuator_0_button_pressed = digitalRead(ACTUATOR_0_MANUAL_SWITCH);
  
  if(actuator_0_button_pressed){
    actuator_number = 0;
    actuator_state[actuator_number] = !actuator_state[actuator_number];    
    write_actuator_state_eeprom(actuator_state[actuator_number], actuator_number);
    digitalWrite(RELAY_DIGITAL_OUT, !actuator_state[actuator_number]); 
  }*/
}

String get_actuators_state_repr(){
  String actuators_state_repr = "";
  if(sizeof(actuator_state) > 1){
    for(int i = 0; i < sizeof(actuator_state) - 1; i++){
      actuators_state_repr = actuators_state_repr + String(actuator_state[i]) + ",";
    }
    actuators_state_repr = actuators_state_repr + String(actuator_state[sizeof(actuator_state)-1]);
  }
  else{
    actuators_state_repr = String(actuator_state[0]);
  }

  return actuators_state_repr;
}

String get_sensor_state_repr(){

  return String("");
}
