
void read_sensors(int task){
  
  if(task == CLIENT){
    if((millis() - client_timestamp) >= CLIENT_WAIT_TIME){

      ext_temperature = dht.readTemperature();
      air_moisture = dht.readHumidity();

      if (isnan(ext_temperature) || isnan(air_moisture)) {return;}

      soil_moisture;
      water_flow; 
      
      client_state = DEVICE_REQUEST;
      client_request_code = REQUEST_ACTUATOR_STATE_CHANGED;
      client_timestamp = millis();
      
    }
  }
  else{
 
  }
}

void change_actuator_state(){
  if(!change_all){
    if(server_state == MANUAL_MODE){
      actuator_state[actuator_number] = !actuator_state[actuator_number];
      write_actuator_state_eeprom(actuator_state[actuator_number], actuator_number);
      digitalWrite(WATER_VALVE, !actuator_state[actuator_number]);
    }
    else{
      
    }
    response_actuator_state = actuator_state[actuator_number];
  }
  else{
    change_all_actuators();
  }
  
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
  
  boolean button_pressed = digitalRead(MANUAL_WATER_VALVE_SWITCH);
  
  if(button_pressed){
    actuator_number = 0;
    actuator_state[actuator_number] = !actuator_state[actuator_number];    
    write_actuator_state_eeprom(actuator_state[actuator_number], actuator_number);
    digitalWrite(WATER_VALVE, !actuator_state[actuator_number]); 
  }
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

  String sensors = String(ext_temperature) + "," 
                + String(air_moisture) + "," 
                + String(soil_moisture) + "," 
                + String(water_flow); 
  return sensors;
}
