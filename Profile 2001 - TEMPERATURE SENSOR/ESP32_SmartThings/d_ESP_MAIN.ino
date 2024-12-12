// Task Handles
TaskHandle_t mqtt_task_handle;
TaskHandle_t gateway_to_esp_tcp_task_handle;
TaskHandle_t esp_to_gateway_tcp_task_handle;

void esp_main() {
  
  switch (main_state) {

    case READ_EEPROM:
      has_configurations_saved = load_settings_eeprom();
      if(has_configurations_saved){
        main_state = CONNECT_WIFI;
      }
      else{
        main_state = WIFI_SETUP;
      }
      break;
      
    case WIFI_SETUP:
      init_wifi_setup();
      main_state = SETTING_UP_WIFI;
      break;

    case SETTING_UP_WIFI:
      http_server.handleClient();
      break;

    case CONNECT_WIFI:
      if(!first_time_connection){
        vTaskDelete(mqtt_task_handle);
        vTaskDelete(gateway_to_esp_tcp_task_handle);
        vTaskDelete(esp_to_gateway_tcp_task_handle);
        debug("MQTT Task -> DELETED");
        debug("GATEWAY to ESP Server Task -> DELETED");
        debug("ESP to GATEWAY Client Task -> DELETED");
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

            /*
              All http requests either inbound or outbound will be dealt in the esp_main task running
              in the ESP32 loop which runs on core 0. All other TCP connections will be running in 
              core 1. MQTT will have the highest priority because it will be the main form of communication
              All other tasks will should have lower priority as they are not as often called or maybe not at all
              if for example the device doesn't need a Gateway connection.
            */

            //This will handle all incoming and outgoing mqtt messages
            xTaskCreatePinnedToCore(mqttTask, "MQTT Task", 4096, NULL, 3, &mqtt_task_handle, 0);  // MQTT - High priority (3)

            //This task will handle all TCP client connection from the Gateway to the ESP32
            xTaskCreatePinnedToCore(server_behavior, "GATEWAY to ESP Server Task", 4096, NULL, 2, &gateway_to_esp_tcp_task_handle, 0); // Gateway - Low priority (1)
            
            //This task will manage all TCP connections from the ESP32 to the Gateway 
            xTaskCreatePinnedToCore(
                    gateway_outgoing_tcp_client_handler,   /* Task function. */
                    "ESP to GATEWAY Client Task",     /* name of task. */
                    4096,             /* Stack size of task */
                    NULL,              /* parameter of the task */
                    1,                 /* priority of the task */
                    &esp_to_gateway_tcp_task_handle,      /* Task handle to keep track of created task */
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
        http_server.handleClient();
        if((millis() - OTA_timestamp) >= OTA_max_time){
          debug("OTA Server shutting down...");
          OTA_Update_active = false;
        }
      }
      break;

    default:
      break;
  }
}
