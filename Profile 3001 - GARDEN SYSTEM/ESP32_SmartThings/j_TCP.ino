int gateway_to_server_request_handler(int code){
    
  switch(code){
    
    case REQUEST_READ_SENSOR:
      debug("GATEWAY_REQUEST: " + String(code) + " REQUEST_READ_SENSOR");
      server_response_code = RESPONSE_SENSOR_DATA;
      return READ_SENSORS;
    
    case REQUEST_ACTUATOR_CHANGE:
      debug("GATEWAY_REQUEST: " + String(code) + " REQUEST_ACTUATOR_CHANGE");
      actuator_number = client.readStringUntil('\r').toInt();
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
      device_name = client.readStringUntil('\r');
      debug("New Device Name - > " + String(device_name));
      esp_bt_dev_set_device_name(device_name.c_str());
      return GATEWAY_RESPONSE;

    case REQUEST_OTA_UPDATE:
      debug("GATEWAY_REQUEST: " + String(code) + " REQUEST_OTA_UPDATE"); 
      if(start_MDNS_server()){
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
      client.print(String(code));
      break;
    
    case RESPONSE_SENSOR_DATA:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_SENSOR_DATA");
      client.print(String(code) + String(" ") + get_sensor_state_repr());
      break;
    
    case RESPONSE_ACTUATOR_CHANGE:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_ACTUATOR_CHANGE");
      client.print(String(code) + String(" ") + String(response_actuator_state));
      break;

    case RESPONSE_MODE_CHANGE:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_MODE_CHANGE");
      server_state = (server_state == MANUAL_MODE) ? AUTONOMOUS_MODE : MANUAL_MODE;
      working_mode = server_state;
      reset_autonomous_timestamp();
      client.print(String(code) + String(" ") + String(server_state));
      break;

    case RESPONSE_DEBUG_CHANGE:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_DEBUG_CHANGE");
      debug_active = !debug_active;
      if(debug_active){
        debug("Debug Mode Activated!");
      }
      client.print(String(code) + String(" ") + String(debug_active));
      break;
    
    case RESPONSE_GATEWAY_CONNECTED:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_GATEWAY_CONNECTED");
      client_state = READ_SENSORS;
      client.print(String(RESPONSE_OK));
      break;

    case RESPONSE_DEVICE_RESET:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_DEVICE_RESET");
      clear_eeprom();
      main_state = SMARTCONFIG;
      client.print(String(RESPONSE_OK));
      break;

    case RESPONSE_ERROR:
      debug("ESP32_RESPONSE: " + String(code) + " RESPONSE_ERROR");
      client.print(String(code));
      break;
      
    default:
      debug("ESP32_RESPONSE: " + String(code) + " UNKNOWN_CODE");
      client.print(UNKNOWN_CODE);
      break;
  }
  client.stop();
}

int client_to_gateway_request_handler(int code){
 
  switch(code){
    
    case REQUEST_SENSOR_DATA:      
      if(gateway.connect(gateway_address, GATEWAY_PORT)){
        debug("ESP32_REQUEST: " + String(code) + " REQUEST_SENSOR_DATA");
        gateway.print(String(code) + String(" ") + get_MAC_as_ID() + 
                     String(" ") + get_sensor_state_repr());
        String line = gateway.readStringUntil('\r');
        gateway_response_code = line.toInt(); 
      }
      else{
        gateway_response_code = NOT_CONNECTED;
      }
      return DEVICE_RESPONSE;

    case REQUEST_ACTUATOR_STATE_CHANGED:
      
      if(gateway.connect(gateway_address, GATEWAY_PORT)){
        debug("ESP32_REQUEST: " + String(code) + " REQUEST_ACTUATOR_STATE_CHANGED");
        gateway.print(String(code) + String(" ") + get_MAC_as_ID() + String(" ") + String(actuator_number) + 
                     String(" ")  + String(actuator_state[actuator_number]));
        String line = gateway.readStringUntil('\r');
        gateway_response_code = line.toInt();
        return DEVICE_RESPONSE;
      }
      else{
        return GATEWAY_REQUEST;
      }

    default:
      return READ_SENSORS;
  }
}

int gateway_to_client_response_handler(int code){
 
  switch(code){
    
    case RESPONSE_OK:
      debug("GATEWAY_RESPONSE: " + String(code) + " RESPONSE_OK");
      return READ_SENSORS;

    case RESPONSE_ACTUATOR_STATE_CHANGED:
      debug("GATEWAY_RESPONSE: " + String(code) + " RESPONSE_ACTUATOR_STATE_CHANGED");
      return READ_SENSORS;
    
    case NOT_CONNECTED:
      debug("GATEWAY_RESPONSE: " + String(code) + " NOT_CONNECTED");
      return RECONNECT_TCP;
      
    default:
      debug("GATEWAY_RESPONSE: " + String(code) + " UNKNOWN_CODE");
      return READ_SENSORS;
  }
}

int handle_tcp_client(){
  delay(1000);
  client = WiFi_Server.available();

  if (client.connected()) {

    debug("Client Connected!");
    debug("IP: " + String(client.remoteIP().toString()) 
           + ":" + String(client.remotePort()));
           
    gateway_address = client.remoteIP();
 
    char recieved_code[4];
    
    recieved_code[0] = client.read();
    recieved_code[1] = client.read();
    recieved_code[2] = client.read();
    recieved_code[3] = 0;
    
    return String(recieved_code).toInt();
  }

  return NOT_CONNECTED;
}
