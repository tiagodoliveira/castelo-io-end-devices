void server_behavior(){

    switch(server_state){
      
      case AUTONOMOUS_MODE:
        autonomous_mode();
        break;
          
      case MANUAL_MODE:
        if (!mqtt_client.connected()) {
          reconnect(); // Reconnect to MQTT broker if disconnected
        }
        mqtt_client.loop(); // Process incoming messages and maintain connection
        manual_mode();
        break;
    
      default:
        server_state = MANUAL_MODE;
        break;
    }
}
