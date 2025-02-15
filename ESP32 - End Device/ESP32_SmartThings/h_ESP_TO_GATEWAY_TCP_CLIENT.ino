WiFiClient esp_to_gateway_tcp_client;

void gateway_outgoing_tcp_client_handler(void * pvParameters){
  
  // Delay of 2 seconds to make sure the Client task only starts
  // after it recieves a connection from the gateway, otherwise
  // it might not have the gateway's correct IP
  delay(2000);

  // Setting Operating System Watchdog standard configurations
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;
  
  for(;;){

    // Delay necessary to be consumed by the watchdog
    // https://github.com/espressif/esp-idf/issues/1646
    delay(10);
    
    switch(client_state){
        
      case READ_SENSORS:
        read_sensors(CLIENT);
        break;
    
      case DEVICE_REQUEST:
        debug("CLIENT -> DEVICE_REQUEST");
        client_state = client_to_gateway_request_handler(client_request_code);        
        break;
  
      case DEVICE_RESPONSE:
        debug("CLIENT -> DEVICE_RESPONSE");        
        client_state = gateway_to_client_response_handler(gateway_response_code);
        break;
  
      case RECONNECT_TCP:
        // Because the Client task is independent of the Gateway's requests
        // it needs to verify if the Gateway IP address is still functional.
        // If not, it will wait in this state until the Server task recieves
        // a request from the Gateway, prompting the device to save it's 
        // new IP address
        if((millis() - reconnect_tcp_timestamp) >= reconnect_tcp_max_time){
          if (!esp_to_gateway_tcp_client.connect(gateway_address, gateway_port)) {
            debug("Connection failed!");
            debug("Waiting " + String(reconnect_tcp_max_time/1000) 
                             + " seconds before retrying...");
            reset_reconnect_tcp_timestamp();
          }
          else{
            client_state = READ_SENSORS;
          }
        }
        break;
              
      default:
        client_state = READ_SENSORS;
        break;
    }
  }
}

int client_to_gateway_request_handler(int code){
 
  switch(code){
    
    case REQUEST_SENSOR_DATA:      
      if(esp_to_gateway_tcp_client.connect(gateway_address, GATEWAY_PORT)){
        debug("ESP32_REQUEST: " + String(code) + " REQUEST_SENSOR_DATA");
        esp_to_gateway_tcp_client.print(String(code) + String(" ") + get_MAC_as_ID() + 
                     String(" ") + get_sensor_state_repr());
        String line = esp_to_gateway_tcp_client.readStringUntil('\r');
        gateway_response_code = line.toInt(); 
      }
      else{
        gateway_response_code = NOT_CONNECTED;
      }
      return DEVICE_RESPONSE;

    case REQUEST_ACTUATOR_STATE_CHANGED:
      
      if(esp_to_gateway_tcp_client.connect(gateway_address, GATEWAY_PORT)){
        debug("ESP32_REQUEST: " + String(code) + " REQUEST_ACTUATOR_STATE_CHANGED");
        esp_to_gateway_tcp_client.print(String(code) + String(" ") + get_MAC_as_ID() + String(" ") + String(actuator_number) + 
                     String(" ")  + String(actuator_state[actuator_number]));
        String line = esp_to_gateway_tcp_client.readStringUntil('\r');
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
