void server_behavior(void* parameter){
    for (;;) {
      switch(server_state){
        
        case AUTONOMOUS_MODE:
          autonomous_mode();
          break;
            
        case MANUAL_MODE:
          manual_mode();
          break;
      
        default:
          server_state = MANUAL_MODE;
          break;
      }
      vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay 5 seconds between messages
    }
}

void start_MDNS_server(const char *name){
  // use multicast DNS to create host name address
  // https://tttapa.github.io/ESP8266/Chap08%20-%20mDNS.html

  debug(name);
  if (!MDNS.begin(name)) { //http_client://device_name.local
    debug("Error setting up MDNS responder!");
  }
  debug("MDNS responder started");
}
