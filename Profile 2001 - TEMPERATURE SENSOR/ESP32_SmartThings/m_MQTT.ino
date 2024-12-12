WiFiClient mqtt_wifi_client;
PubSubClient mqtt_client(mqtt_wifi_client);

// Task to handle MQTT connections and message loop
void mqttTask(void* parameter) {
  mqtt_client.setServer(mqtt_server_url, mqtt_port);  // Set the MQTT broker details
  mqtt_client.setCallback(mqttCallback);         // Set the message callback function

  for (;;) {
    if (!mqtt_client.connected()) {
      connectMQTT(); // Reconnect if disconnected
    }
    mqtt_client.loop(); // Process MQTT messages
    vTaskDelay(10 / portTICK_PERIOD_MS); // Let other tasks execute briefly
  }
}

// Connect to MQTT Broker
void connectMQTT() {
  while (!mqtt_client.connected()) {
    Serial.print("Connecting to MQTT broker...");
    if (mqtt_client.connect(get_MAC_as_ID().c_str())) { // Set your unique client ID
      Serial.println("MQTT connected!");
      mqtt_client.subscribe("device/#"); // Subscribe to desired topics
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" Trying again in 5 seconds...");
      vTaskDelay(5000 / portTICK_PERIOD_MS); // Use FreeRTOS delay (non-blocking)
    }
  }
}

// Function to handle incoming messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  debug("Message arrived on topic: ");
  debug(topic);

  debug("Message: ");
  String testMessage = (char*)payload;
  // TODO handle MQTT message
  debug(testMessage);
}

void reconnect() {
  if((millis() - reconnect_mqtt_timestamp) >= reconnect_mqtt_max_time){
    if (!mqtt_client.connected()) {
      debug("Attempting MQTT connection...");
      if (mqtt_client.connect(mqtt_client_id)) {
        debug("connected!");
        mqtt_client.subscribe(mqtt_topic); // Subscribe to the topic
        debug("Subscribed to topic: ");
        debug(mqtt_topic);
      } else {
        debug("failed, rc=");
        debug(String(mqtt_client.state()));
        debug(" Trying again in 5 seconds...");
        reset_reconnect_mqtt_timestamp();
      }
    }
  }
}