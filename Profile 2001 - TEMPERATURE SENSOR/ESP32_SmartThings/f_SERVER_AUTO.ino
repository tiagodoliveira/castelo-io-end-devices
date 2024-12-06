void autonomous_mode(){
  
  switch (autonomous_mode_state) {

    case GATEWAY_REQUEST:  
      
      if(WiFi.status() != WL_CONNECTED){
        autonomous_mode_state = BLUETOOTH_REQUEST;
      }
      else{
        if((millis() - autonomous_timestamp) >= autonomous_max_time){
          autonomous_mode_state = READ_SENSORS;
          reset_autonomous_timestamp();
        }
        else{
          gateway_request_code = handle_tcp_client();
          if(gateway_request_code != NOT_CONNECTED){
            autonomous_mode_state = gateway_to_server_request_handler(gateway_request_code);
          }
        }
      }   
      break;

    case READ_SENSORS:
      read_sensors(SERVER);
      autonomous_mode_state = CHANGE_ACTUATOR_STATE;
      break;

    case CHANGE_ACTUATOR_STATE:
      change_actuator_state();
      autonomous_mode_state = GATEWAY_REQUEST;
      if(WiFi.status() != WL_CONNECTED){
        autonomous_mode_state = BLUETOOTH_REQUEST;
      }
      break;

    case CHANGE_MODE:
      working_mode = MANUAL_MODE;
      write_mode_eeprom(working_mode);
      autonomous_mode_state = GATEWAY_RESPONSE;
      debug("Changing to Manual Mode...");
      break;

    case GATEWAY_RESPONSE:
      autonomous_mode_state = GATEWAY_REQUEST;
      if(WiFi.status() == WL_CONNECTED){
        debug("ESP32 -> GATEWAY_RESPONSE");
        server_to_gateway_response_handler(server_response_code);
      }
      break;

    case BLUETOOTH_REQUEST:
      if((millis() - autonomous_timestamp) >= autonomous_max_time){
        autonomous_mode_state = READ_SENSORS;
        reset_autonomous_timestamp();
      }
      break;
      
    default:
      autonomous_mode_state = GATEWAY_REQUEST;
      break;
  }
}
