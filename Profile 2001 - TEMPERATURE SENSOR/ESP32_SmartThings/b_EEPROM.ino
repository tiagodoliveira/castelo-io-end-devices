/*
 * Reads the first 32 addresses of the EEPROM
 * where the WiFi SSID is stored. If the sum of
 * the read values is greater than zero, 
 * the function continues to read the rest of
 * the saved settings. If the sum of the read
 * values is zero it means that the EEPROM was
 * cleared, or its the first time turning on the device.
 * 
 * returns boolean -> The returned value is used to 
 * verify whether there are saved settings on the EEPROM or not.
 */
boolean load_settings_eeprom() {
  
  int total_ssid_sum = 0;
  
  debug("Reading EEPROM -> WiFi SSID...");
  
  for (int i = WIFI_SSID_ADDRESS_START; i <= WIFI_SSID_ADDRESS_END; ++i){
    char eeprom_value = char(EEPROM.read(i));
    wifi_ssid += eeprom_value;
    total_ssid_sum += (eeprom_value + 0);
  }

  if(total_ssid_sum > 0){
    debug("Reading EEPROM -> WiFi Password...");
  
    for (int i = WIFI_PASWORD_ADDRESS_START; i <= WIFI_PASWORD_ADDRESS_END; ++i){
      wifi_password += char(EEPROM.read(i));
    }
    read_mode_eeprom();
    read_actuator_state_eeprom();
  
    return true;
  }
  debug("No settings saved on EEPROM!");
  
  return false;
}

/*
 * Saves the connected network's SSID and Password
 * to their correspondent EEPROM address if the current one is different from the already saved
 * We do this to avoid rewirting on the EEPROM everytime the device restarts because the EEPROM
 * as a write life of 100,000 times
 */
void save_network_settings() {

  String curr_ssid = WiFi.SSID();
  String curr_pass = WiFi.psk();

  if(strcmp(curr_ssid.c_str(), wifi_ssid.c_str()) != 0 && strcmp(curr_pass.c_str(), wifi_password.c_str()) != 0){
    if (curr_ssid.length() > 0 && curr_pass.length() > 0) {
        
        debug("Saving network settings...");
        
        debug("Writing EEPROM wifi_ssid...");
        write_str_eeprom(WIFI_SSID_ADDRESS_START, curr_ssid);
        
        debug("Writing EEPROM pass..."); 
        write_str_eeprom(WIFI_PASWORD_ADDRESS_START, curr_pass);
      }
      else{
        debug("Invalid network settings recieved!");
      }
  } else{
    debug("Valid credentials already saved!"); 
  }
}

/*
 * Fills the EEPROM addresses with zeros resetting it's
 * values. After resetting the addresses, restores the 
 * Working Mode and Behavior addresses to their default value
 */
void clear_eeprom(){
  
  debug("Clearing EEPROM...");
  
  for (int i = 0; i < EEPROM_MAX_SIZE; ++i) { 
    EEPROM.write(i, 0); 
  }
  EEPROM.write(WORKING_MODE_ADDRESS, MANUAL_MODE);
  EEPROM.commit();
  
  has_configurations_saved = false;
}

/*
 * Writes the input String in the EEPROM at the given
 * address.
 */
void write_str_eeprom(int init_address, String value){
  
  for (int i = 0; i < value.length(); ++i){
    if(i > EEPROM_MAX_SIZE){
      debug("Warning: Not enough space on EEPROM!");
      debug("Value: " + value + " -> Not written!");
      return;
    }
    EEPROM.write(init_address + i, value[i]);
  }   
  EEPROM.commit();
}

void write_actuator_state_eeprom(int state, int actuator_num){
  debug("Writing actuator state " + String(state) + " on EEPROM...");
  EEPROM.write(ACTUATOR_STATE_ADDRESS[actuator_num], state);
  EEPROM.commit(); 
}

void write_mode_eeprom(int current_mode){
  debug("Writing working mode " + String(current_mode) + " on EEPROM...");
  EEPROM.write(WORKING_MODE_ADDRESS, current_mode); 
  EEPROM.commit();
}

void read_actuator_state_eeprom(){
  debug("Reading actuator state of EEPROM...");
  for(int i = 0; i < sizeof(actuator_state) ; i++){
    actuator_state[i] = EEPROM.read(ACTUATOR_STATE_ADDRESS[i]);
  }
  set_actuators_state();
}

void read_mode_eeprom(){
  debug("Reading working mode of EEPROM...");
  working_mode = EEPROM.read(WORKING_MODE_ADDRESS); 
}
