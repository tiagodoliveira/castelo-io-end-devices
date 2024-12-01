void manual_mode(){
  
  switch (manual_mode_state) {

    case GATEWAY_REQUEST:
        gateway_request_code = handle_client();
        if(gateway_request_code != NOT_CONNECTED){
          manual_mode_state = gateway_to_server_request_handler(gateway_request_code);
        }
      break;

    case READ_SENSORS:
      debug("ESP32 -> READ_SENSORS");
      read_sensors(SERVER);
      manual_mode_state = GATEWAY_RESPONSE;
      break;

    case CHANGE_ACTUATOR_STATE:
      debug("ESP32 -> CHANGE_ACTUATOR_STATE");
      change_actuator_state();
      manual_mode_state = GATEWAY_RESPONSE;
      break;

    case CHANGE_MODE:
      debug("ESP32 -> CHANGE_MODE");
      working_mode = AUTONOMOUS_MODE;
      write_mode_eeprom(working_mode);
      manual_mode_state = GATEWAY_RESPONSE;
      debug("Changing do Autonomous Mode...");
      break;

    case GATEWAY_RESPONSE:
      debug("ESP32 -> GATEWAY_RESPONSE");
      manual_mode_state = GATEWAY_REQUEST;
      server_to_gateway_response_handler(server_response_code);
      break;

    default:
      manual_mode_state = GATEWAY_REQUEST;
      break;
  }
}
