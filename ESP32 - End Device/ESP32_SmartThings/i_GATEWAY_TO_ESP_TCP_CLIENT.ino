WiFiClient incoming_tcp_client;

int gateway_to_server_request_handler(int code){
    
  switch(code){
    
    case REQUEST_READ_SENSOR:
      debug("GATEWAY_REQUEST: " + String(code) + " REQUEST_READ_SENSOR");
      server_response_code = RESPONSE_SENSOR_DATA;
      return READ_SENSORS;
    
    case REQUEST_ACTUATOR_CHANGE:
      debug("GATEWAY_REQUEST: " + String(code) + " REQUEST_ACTUATOR_CHANGE");
      actuator_number = incoming_tcp_client.readStringUntil('\r').toInt();
      debug("ACTUATOR NUMBER: " + String(actuator_number));
      server_response_code = RESPONSE_ACTUATOR_CHANGE;
      return CHANGE_ACTUATOR_STATE;

    case REQUEST_ACTUATOR_CHANGE_ALL:
      debug("GATEWAY_REQUEST: " + String(code) + " REQUEST_ACTUATOR_CHANGE_ALL");
      change_all = true;
      server_response_code = RESPONSE_ACTUATOR_CHANGE;
      return CHANGE_ACTUATOR_STATE; 

    case REQUEST_MODE_CHANGE:
      debug("GATEWAY_REQUEST: " + String(code) + " REQUEST_MODE_CHANGE");
      server_response_code = RESPONSE_MODE_CHANGE;
      return CHANGE_MODE;

    case REQUEST_DEBUG_CHANGE:
      debug("GATEWAY_REQUEST: " + String(code) + " REQUEST_DEBUG_CHANGE");
      server_response_code = RESPONSE_DEBUG_CHANGE;
      return GATEWAY_RESPONSE;

    case REQUEST_GATEWAY_CONNECTION:
      debug("GATEWAY_REQUEST: " + String(code) + " REQUEST_GATEWAY_CONNECTION");
      server_response_code = RESPONSE_GATEWAY_CONNECTED;
      return GATEWAY_RESPONSE;

    case REQUEST_NAME_CHANGE:
      debug("GATEWAY_REQUEST: " + String(code) + " REQUEST_NAME_CHANGE");
      server_response_code = RESPONSE_OK;
      device_name = incoming_tcp_client.readStringUntil('\r');
      debug("New Device Name - > " + String(device_name));
      esp_bt_dev_set_device_name(device_name.c_str());
      return GATEWAY_RESPONSE;

    case REQUEST_OTA_UPDATE:
      debug("GATEWAY_REQUEST: " + String(code) + " REQUEST_OTA_UPDATE"); 
      if(start_OTA_server()){
        server_response_code = RESPONSE_OK;
        OTA_Update_active = true;
        reset_OTA_timestamp();
      }
      else{
        server_response_code = RESPONSE_ERROR;
      }
      return GATEWAY_RESPONSE;

    case REQUEST_DEVICE_RESET:
      debug("GATEWAY_REQUEST: " + String(code) + " REQUEST_DEVICE_RESET");
      server_response_code = RESPONSE_DEVICE_RESET;
      return GATEWAY_RESPONSE;

    default:
      debug("GATEWAY_REQUEST: " + String(code) + " UNKNOWN_CODE");
      server_response_code = UNKNOWN_CODE;
      return GATEWAY_RESPONSE;
  }
}

void server_to_gateway_response_handler(int code){
  
  switch(code){

    case RESPONSE_OK:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_OK");
      incoming_tcp_client.print(String(code));
      break;
    
    case RESPONSE_SENSOR_DATA:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_SENSOR_DATA");
      incoming_tcp_client.print(String(code) + String(" ") + get_sensor_state_repr());
      break;
    
    case RESPONSE_ACTUATOR_CHANGE:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_ACTUATOR_CHANGE");
      incoming_tcp_client.print(String(code) + String(" ") + String(response_actuator_state));
      break;

    case RESPONSE_MODE_CHANGE:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_MODE_CHANGE");
      server_state = (server_state == MANUAL_MODE) ? AUTONOMOUS_MODE : MANUAL_MODE;
      working_mode = server_state;
      reset_autonomous_timestamp();
      incoming_tcp_client.print(String(code) + String(" ") + String(server_state));
      break;

    case RESPONSE_DEBUG_CHANGE:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_DEBUG_CHANGE");
      debug_active = !debug_active;
      if(debug_active){
        debug("Debug Mode Activated!");
      }
      incoming_tcp_client.print(String(code) + String(" ") + String(debug_active));
      break;
    
    case RESPONSE_GATEWAY_CONNECTED:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_GATEWAY_CONNECTED");
      client_state = READ_SENSORS;
      incoming_tcp_client.print(String(RESPONSE_OK));
      break;

    case RESPONSE_DEVICE_RESET:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_DEVICE_RESET");
      clear_eeprom();
      main_state = WIFI_SETUP;
      incoming_tcp_client.print(String(RESPONSE_OK));
      break;

    case RESPONSE_ERROR:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_ERROR");
      incoming_tcp_client.print(String(code));
      break;
      
    default:
      debug("ESP32_RESPONSE: " + String(code) + " UNKNOWN_CODE");
      incoming_tcp_client.print(UNKNOWN_CODE);
      break;
  }
  incoming_tcp_client.stop();
}

int handle_tcp_client(){
  incoming_tcp_client = WiFi_Server.available();

  if (incoming_tcp_client.connected()) {

    debug("Client Connected!");
    debug("IP: " + String(incoming_tcp_client.remoteIP().toString()) 
           + ":" + String(incoming_tcp_client.remotePort()));
           
    gateway_address = incoming_tcp_client.remoteIP();
 
    char recieved_code[4];
    
    recieved_code[0] = incoming_tcp_client.read();
    recieved_code[1] = incoming_tcp_client.read();
    recieved_code[2] = incoming_tcp_client.read();
    recieved_code[3] = 0;
    
    return String(recieved_code).toInt();
  }

  return NOT_CONNECTED;
}
