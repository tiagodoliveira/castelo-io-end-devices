boolean bluetooth_handle(){
  if (SerialBT.available()) {
    return bluetooth_request_handler(SerialBT.read());
  }
  return false;
}

boolean bluetooth_request_handler(char recieved_message){

  int message_code = recieved_message - '0';
  
  switch(message_code){
    
    case BT_REQUEST_MODE_CHANGE:{
      if(debug_active || server_state == AUTONOMOUS_MODE){
        autonomous_mode_state = CHANGE_MODE;
        SerialBT.write(BT_RESPONSE_OK);
        SerialBT.write('\n');
      }
      return true;}

    case BT_REQUEST_DEBUG_CHANGE:{
      if(debug_active){
        debug_active = !debug_active;
        SerialBT.write(BT_RESPONSE_OK);
        SerialBT.write('\n');
      }
      return true;}

    case BT_REQUEST_CLEAR_EEPROM:{
      if(debug_active){
        clear_eeprom();
        has_configurations_saved = false;
        main_state = SMARTCONFIG;
        SerialBT.write(BT_RESPONSE_OK);
        SerialBT.write('\n');
      }
      return true;}

    case BT_REQUEST_OTA_UPDATE:{
      if(debug_active){
        OTA_Update_active = true;
        start_MDNS_server();
        reset_OTA_timestamp();
        SerialBT.write(BT_RESPONSE_OK);
        SerialBT.write('\n');
      }
      return true;}

    default:{
      SerialBT.write(BT_RESPONSE_ERROR);
      SerialBT.write('\n');
      return false;}
  }
}

void debug(String val){
  if(debug_active){
    Serial.println(val);
    delay(30);
    int c = val.length();
    for (int i = 0; i < c; i++){
        SerialBT.write(val.charAt(i));
    }
    SerialBT.write('\n');
  }
}
