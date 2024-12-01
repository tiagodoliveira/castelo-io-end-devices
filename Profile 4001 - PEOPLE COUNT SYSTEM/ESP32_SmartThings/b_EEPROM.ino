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
  
  for (int i = 0; i < 32; ++i){
    char eeprom_value = char(EEPROM.read(i));
    ssid += eeprom_value;
    total_ssid_sum += (eeprom_value + 0);
  }

  if(total_ssid_sum > 0){
    debug("Reading EEPROM -> WiFi Password...");
  
    for (int i = 32; i < 96; ++i){
      password += char(EEPROM.read(i));
    }

    debug("SSID: " + String(ssid.c_str()));
    debug("Pass: " + String(password.c_str()));
    read_mode_eeprom();
    read_actuator_state_eeprom();
  
    return true;
  }
  debug("No settings saved on EEPROM!");
  
  return false;
}

/*
 * Saves the connected network's SSID and Password
 * to their correspondent EEPROM address.
 */
void save_network_settings() {

  String curr_ssid = WiFi.SSID();
  String curr_pass = WiFi.psk();
  
  if (curr_ssid.length() > 0 && curr_pass.length() > 0) {
    
    debug("Saving network settings...");
    
    debug("Writing EEPROM ssid...");
    write_str_eeprom(0, curr_ssid);
    
    debug("Writing EEPROM pass..."); 
    write_str_eeprom(SSID_MAX_LENGTH, curr_pass);
  }
  else{
    debug("Invalid network settings recieved!");
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

//This function will write a 4 byte (32bit) long to the eeprom at
//the specified address to adress + 3.
void write_long_eeprom(int address, long value){
  
  //Decomposition from a long to 4 bytes using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

//This function will return a 4 byte (32bit) long from the eeprom
//at the specified address to adress + 3.
long read_long_eeprom(int address){
  
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
