void esp_main() {
  
  switch (main_state) {

    case READ_EEPROM:
      has_configurations_saved = load_settings_eeprom();
      set_actuators_state();
      if(has_configurations_saved){
        main_state = CONNECT_WIFI;
      }
      else{
        main_state = SMARTCONFIG;
      }
      break;
      
    case SMARTCONFIG:
      init_smartconfig();
      main_state = CONNECT_WIFI;
      break;

    case CONNECT_WIFI:
      if(!first_time_connection){
        vTaskDelete(Client_Task);
        debug("CLIENT TASK -> DELETED");
      }
      
      manual_actuator_state_change_handler();
      connect_to_wifi();
      break;

    case DEVICE_CONNECTION:
      manual_actuator_state_change_handler();
      if(WiFi.status() != WL_CONNECTED){
        main_state = CONNECT_WIFI;
      }
      else{
        if((millis() - post_request_timestamp) >= post_request_max_time){
          reset_post_request_timestamp();
          server_http_response = send_device_info_to_main_server();
          
          if(server_http_response == RESPONSE_OK){
            
            if(working_mode == AUTONOMOUS_MODE){
              debug("ESP32 -> SERVER AUTO MODE");
              server_state = AUTONOMOUS_MODE;
              reset_autonomous_timestamp();
            }
            else{
              debug("ESP32 -> SERVER MANUAL MODE");
              debug("Waiting for TCP request...");
              server_state = MANUAL_MODE;
              
            }
            
            xTaskCreatePinnedToCore(
                    client_behavior,   /* Task function. */
                    "Client_Task",     /* name of task. */
                    10000,             /* Stack size of task */
                    NULL,              /* parameter of the task */
                    1,                 /* priority of the task */
                    &Client_Task,      /* Task handle to keep track of created task */
                    0);                /* pin task to core 0 */   
                          
            first_time_connection = false;
            main_state = OTA_HANDLER;

            debug("-> SERVER AND CLIENT TASKS CREATED");
          }
        }
      }
      break;
  
    case OTA_HANDLER:
      if(WiFi.status() != WL_CONNECTED){
          main_state = CONNECT_WIFI;
        }
      if(OTA_Update_active){
        OTA_server.handleClient();
        if((millis() - OTA_timestamp) >= OTA_max_time){
          debug("OTA Server shutting down...");
          OTA_server.close();
          OTA_Update_active = false;
        }
      }
      break;

    default:
      break;
  }
}
