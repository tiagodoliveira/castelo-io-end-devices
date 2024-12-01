/* 
 * Used Libraries
 */
#include "WiFi.h"
#include "esp_system.h"
#include "esp_bt_device.h"
#include "BluetoothSerial.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

/* 
 * Automatons States
 */
#define READ_EEPROM 0
#define SMARTCONFIG 1
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
#define REQUEST_RESET_COUNTER 110
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
#define DEVICE_NAME_START_ADDRESS 100
#define DEVICE_NAME_END_ADDRESS 119
#define WORKING_MODE_ADDRESS 120
const int ACTUATOR_STATE_ADDRESS[] = {130, 131, 132, 133, 134, 135, 136, 137, 138, 139};

// Checks if Bluetooth is turned on
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

/* 
 * Program variables
 */ 
int server_state, actuator_number;
int gateway_request_code, client_request_code, gateway_response_code;
int server_response_code, working_mode, autonomous_max_time;
int ssid_length, psk_length, server_http_response;

int main_state = READ_EEPROM;
int client_state = READ_SENSORS;
int autonomous_mode_state = GATEWAY_REQUEST; 
int manual_mode_state = GATEWAY_REQUEST;

// Maximum time in millis that the Over the Air Update Server will wait for a new
// firmware update file, sent by the Gateway.
int OTA_max_time = 120000;

// Time in millis that the device will wait until trying to re-establish the TCP connection
// with the Gateway. Only used in Client Behavior.
int reconnect_tcp_max_time = 5000;

boolean connected_to_gateway, response_actuator_state, change_all;
boolean first_time_connection = true;
boolean has_configurations_saved = false;
boolean OTA_Update_active = false;
boolean debug_active = true;
boolean board_led_state = HIGH;

unsigned long autonomous_timestamp, client_timestamp;
unsigned long reconnect_tcp_timestamp, blink_timestamp;
unsigned long OTA_timestamp, request_actuator_timestamp;

String ssid, password, device_name;

String main_server_address = "http://192.168.43.246:8080/communication";

JsonObject recieved_object;
IPAddress gateway_address;
uint16_t gateway_port;
BluetoothSerial SerialBT;
WiFiServer WiFi_Server(ESP32_SERVER_PORT);
WebServer OTA_server(MDNS_SERVER_PORT);

WiFiClient client;
WiFiClient gateway;

TaskHandle_t Client_Task;

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
