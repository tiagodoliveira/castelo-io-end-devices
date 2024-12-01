void client_behavior(void * pvParameters){
  
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
          if (!gateway.connect(gateway_address, gateway_port)) {
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
