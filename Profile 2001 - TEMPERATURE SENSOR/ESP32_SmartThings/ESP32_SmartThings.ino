/* 
 * Used Libraries
 */
#include "WiFi.h"
#include "esp_system.h"
#include "esp_bt_device.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include <WiFiManager.h>
#include <PubSubClient.h>

/* 
 * Automatons States
 */
#define READ_EEPROM 0
#define WIFI_SETUP 1
#define CONNECT_WIFI 2
#define DEVICE_CONNECTION 3
#define CLIENT 4
#define SERVER 5
#define AUTONOMOUS_MODE 6
#define MANUAL_MODE 7
#define GATEWAY_REQUEST 8
#define READ_SENSORS 9
#define CHANGE_ACTUATOR_STATE 10
#define CHANGE_MODE 11
#define GATEWAY_RESPONSE 13
#define BLUETOOTH_REQUEST 14
#define DEVICE_REQUEST 15
#define DEVICE_RESPONSE 16
#define RECONNECT_TCP 17
#define OTA_HANDLER 18
#define SETTING_UP_WIFI 19


/* 
 * HTTP/TCP Communication Protocol
 */ 
// Requests
#define REQUEST_READ_SENSOR 101
#define REQUEST_ACTUATOR_CHANGE 102
#define REQUEST_MODE_CHANGE 103
#define REQUEST_DEBUG_CHANGE 105
#define REQUEST_GATEWAY_CONNECTION 106
#define REQUEST_NAME_CHANGE 107
#define REQUEST_OTA_UPDATE 108
#define REQUEST_ACTUATOR_CHANGE_ALL 109
#define REQUEST_DEVICE_RESET 129
#define REQUEST_SENSOR_DATA 130
#define REQUEST_ACTUATOR_STATE_CHANGED 131
#define REQUEST_DEVICE_CONNECTION 199

// Responses
#define RESPONSE_OK 200
#define RESPONSE_SENSOR_DATA 201
#define RESPONSE_ACTUATOR_CHANGE 202
#define RESPONSE_MODE_CHANGE 203
#define RESPONSE_DEBUG_CHANGE 205
#define RESPONSE_GATEWAY_CONNECTED 206
#define RESPONSE_DEVICE_RESET 229
#define RESPONSE_ACTUATOR_STATE_CHANGED 231
#define RESPONSE_ERROR 400
#define UNKNOWN_CODE 401

/* 
 * Bluetooth Communication Protocol
 */ 
//Requests
#define BT_REQUEST_MODE_CHANGE 1
#define BT_REQUEST_DEBUG_CHANGE 2
#define BT_REQUEST_CLEAR_EEPROM 3
#define BT_REQUEST_OTA_UPDATE 4

//Responses
#define BT_RESPONSE_OK '0'
#define BT_RESPONSE_ERROR '9'

/* 
 * Comunication Ports
 */ 
#define ESP32_SERVER_PORT 90
#define MDNS_SERVER_PORT 80
#define GATEWAY_PORT 8082

/* 
 * Program constants
 */ 
#define ON_BOARD_LED 2
#define WIFI_BLINK_MAX 200
#define SMARTCONFIG_BLINK_MAX 500
#define NOT_CONNECTED 500
#define MAX_ACTUATORS 10
#define EEPROM_MAX_SIZE 512
#define SSID_MAX_LENGTH 32
#define PSK_MAX_LENGTH 64
#define NO_SENSORS_OR_ACTUATORS "empty"

/* 
 * EEPROM addresses
 */ 
#define WIFI_SSID_ADDRESS_START 0
#define WIFI_SSID_ADDRESS_END 31
#define WIFI_PASWORD_ADDRESS_START 32
#define WIFI_PASWORD_ADDRESS_END 95
#define DEVICE_NAME_ADDRESS_START 96
#define DEVICE_NAME_ADDRESS_END 195
#define WORKING_MODE_ADDRESS 196
const int ACTUATOR_STATE_ADDRESS[] = {197, 198, 199, 200};
// addresses 201 to 255 are empty
#define SERVER_HOST_ADDRESS_START 256
#define SERVER_HOST_ADDRESS_END 512



/*
 * Bluetooth (BLE) Configuration
 */
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLECharacteristic *pCharacteristic;
bool bt_device_connected = false;
char bt_received_char = '\0';


/* 
 * Program variables
 */ 
int main_state, server_state, client_state, actuator_number;
int autonomous_mode_state, manual_mode_state;
int gateway_request_code, client_request_code, gateway_response_code;
int server_response_code, working_mode;
int ssid_length, psk_length, server_http_response;
int autonomous_max_time, OTA_max_time;
int post_request_max_time, reconnect_tcp_max_time, reconnect_mqtt_max_time;

boolean connected_to_gateway, response_actuator_state, change_all;
boolean first_time_connection = true;
boolean has_configurations_saved = false;
boolean OTA_Update_active = false;
boolean debug_active = true;
boolean board_led_state = HIGH;

unsigned long autonomous_timestamp, client_timestamp;
unsigned long post_request_timestamp, reconnect_tcp_timestamp, reconnect_mqtt_timestamp;
unsigned long blink_timestamp, OTA_timestamp, request_actuator_timestamp;

String wifi_ssid, wifi_password, user_id, main_server_address, telemetry_server_address, gateway_mac_address;

const char* mqtt_server_url = "192.168.1.179";
const int mqtt_port = 1883;   
// MQTT client details
const char* mqtt_client_id;    // Unique client ID for the ESP32 device
const char* mqtt_topic = "device/#";            // Subscribes to all subtopics under 'device/'

JsonObject recieved_object;
IPAddress gateway_address;
uint16_t gateway_port;

WiFiServer WiFi_Server(ESP32_SERVER_PORT);
WebServer http_server(MDNS_SERVER_PORT);

/*
 * Server Index Page
 */
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";
