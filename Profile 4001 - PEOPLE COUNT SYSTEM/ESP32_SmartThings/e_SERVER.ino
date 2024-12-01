void server_behavior(){

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
}
