// Function to handle incoming messages
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  debug("Message arrived on topic: ");
  debug(topic);

  debug("Message: ");
  String testMessage = (char*)payload;
  debug(testMessage);
}

void reconnect() {
  while (!mqtt_client.connected()) {
    debug("Attempting MQTT connection...");
    if (mqtt_client.connect(client_id)) {
      debug("connected!");
      mqtt_client.subscribe(topic); // Subscribe to the topic
      debug("Subscribed to topic: ");
      debug(topic);
    } else {
      debug("failed, rc=");
      debug(String(mqtt_client.state()));
      debug(" Trying again in 5 seconds...");
      delay(5000);
    }
  }
}