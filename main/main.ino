// WiFi library for ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h> // for OTA discovery in Arduino

// MQTT libraries
#include <WiFiClient.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// NTP libraries
#include <NTPClient.h>
#include <WiFiUdp.h>

// DS18B20 sensor library
#include <OneWire.h>
#include <DallasTemperature.h>

// Logging library
#include <ArduinoLog.h>

// OTA library
#include <ArduinoOTA.h>

// Configuration file
#include "config.h"

// Define MQTT client to send data
WiFiClient wifiClient;
Adafruit_MQTT_Client mqttClient(&wifiClient,
                                MQTT_SERVER_ADDRESS, MQTT_SERVER_PORT,
                                MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);

// Topics to publish data
Adafruit_MQTT_Publish telemetryTopic = Adafruit_MQTT_Publish(&mqttClient, MQTT_TELEMETRY_TOPIC, MQTT_QOS_0);
Adafruit_MQTT_Publish attributeTopic = Adafruit_MQTT_Publish(&mqttClient, MQTT_ATTRIBUTE_TOPIC, MQTT_QOS_1);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER);

// DS18B20 objects
OneWire oneWire(ONEWIRE_PIN);
DallasTemperature sensor(&oneWire);
DeviceAddress temp_addr;

// Buffer for MQTT message
char msg[MSG_BUFFER_SIZE];

void WifiSetup() {
  Log.noticeln(NL NL "Connecting to " WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int8_t ret;
  if ((ret = WiFi.waitForConnectResult()) != WL_CONNECTED) {
    Log.errorln("Connection Error, status_code = %d", ret);
    Log.errorln("Resetting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  Log.noticeln(NL "WiFi connected. IP address: ");
  Log.noticeln(WiFi.localIP());
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqttClient.connected()) return;

  Log.noticeln("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqttClient.connect()) != 0) { // connect will return 0 for connected
    Log.errorln(mqttClient.connectErrorString(ret));
    Log.errorln("Retrying MQTT connection in 5 seconds...");
    mqttClient.disconnect();

    for(int i = 0; i < 50; i++) { // waits 5 seconds while checking for OTA updates
      ArduinoOTA.handle();
      delay(100);
    }

    retries--;
    if (retries == 0) ESP.restart();
  }

  Log.noticeln("MQTT Connected!");
}

bool readTempSensor(float& temperature) {
  sensor.requestTemperatures();
  
  if (!sensor.getAddress(temp_addr,0)) { // Find sensor address
    return false;
  }

  temperature = sensor.getTempC(temp_addr);
  return true;
}

bool testTempSensor() {
  float temp;
  return readTempSensor(temp);
}

void sendData() {
  // Save current timestamp
  unsigned long timestamp = timeClient.getEpochTime(); // unix time in seconds

  // Read Temperature with DS18B20 sensor
  float temp;
  if(!readTempSensor(temp)) {
    Log.errorln("DS18B20 Sensor error.");
    sendStatus("DS18B20", "ERROR");
    ESP.restart();
  }
  Log.noticeln("DS18B20 Read OK.");

  // Write to message buffer, timestamp in milliseconds by adding 3 zeros
  snprintf(msg, MSG_BUFFER_SIZE, "{'ts':%lu000,'values':{'temperature':%.1f}}", timestamp, temp);

  // Send MQTT message to telemetryTopic
  Log.noticeln("Publish message: %s", msg);

  while(!telemetryTopic.publish(msg)) {
    Log.errorln("Failed. Disconnect, connect.");
    delay(1000);
    mqttClient.disconnect();
    MQTT_connect();
    Log.noticeln("Trying to send message again now.");
  }

  Log.noticeln("Success, next in 30s.");
  delay(2000);
}

void sendSystemInfo() {
  snprintf(msg, MSG_BUFFER_SIZE, "{'firmwareVersion':'%s','ip':'%s','mac':'%s'}",
                                 FIRMWARE_VERSION,
                                 WiFi.localIP().toString().c_str(),
                                 WiFi.macAddress().c_str());

  // Send MQTT message to attributeTopic
  Log.noticeln("Publish message: %s", msg);

  while(!attributeTopic.publish(msg)) {
    Log.errorln("Failed. Disconnect, connect.");
    delay(1000);
    mqttClient.disconnect();
    MQTT_connect();
    Log.noticeln("Trying to send message again now.");
  }
}

void sendStatus(const char *label, const char *status) {
  snprintf(msg, MSG_BUFFER_SIZE, "{'%s_status':'%s'}", label, status);

  // Send MQTT message to attributeTopic
  Log.noticeln("Publish message: %s", msg);

  while(!attributeTopic.publish(msg)) {
    Log.errorln("Failed. Disconnect, connect.");
    delay(1000);
    mqttClient.disconnect();
    MQTT_connect();
    Log.noticeln("Trying to send message again now.");
  }
}

void setup() {
  // Iniatilize serial interface for logging
  Serial.begin(115200);
  while(!Serial && !Serial.available());
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  // Show MAC
  Log.noticeln(NL NL "MAC address: %s", WiFi.macAddress().c_str());

  // Initialize WiFi connection
  WifiSetup();

  // OTA setup
  ArduinoOTA.setHostname(MQTT_CLIENT_ID);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();

  // Connect with MQTT Broker
  MQTT_connect();

  // Send system info as attributtes
  sendSystemInfo();

  // Initialize NTP client
  timeClient.begin();

  // Get time for the first time, timesout in 15s after start
  // Send to Thingsboard status of NTP time
  while(!timeClient.isTimeSet()) {
    Log.noticeln("Waiting for time to be set by NTP.");
    if(millis() > 15000) {
      Log.noticeln("Taking too long. Reporting error and restarting.");
      sendStatus("NTP", "ERROR");
      ESP.restart();
    }
    timeClient.update();
    delay(1000);
  }
  Log.noticeln("NTP time set successfully.");
  sendStatus("NTP", "OK");

  // Initialize and test DS18B20 sensor
  sensor.begin();

  if(!testTempSensor()) {
    Log.errorln("DS18B20 Sensor not working.");
    sendStatus("DS18B20", "ERROR");
    ESP.restart();
  }
  Log.noticeln("DS18B20 Sensor working.");
  sendStatus("DS18B20", "OK");
}

void loop() {
  // Handle OTA firmware flash request
  ArduinoOTA.handle();

  int seconds = timeClient.getSeconds();
  if(seconds == 0 || seconds == 30) {
    sendData();

    // Update internal time with NTP server
    timeClient.update();
  }

  delay(200);
}
