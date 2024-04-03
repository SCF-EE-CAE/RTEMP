// Code version
#define FIRMWARE_VERSION "v2"

// WiFi Credentials
#define WIFI_SSID     ""
#define WIFI_PASSWORD ""

// MQTT Broker address/port
#define MQTT_SERVER_ADDRESS ""
#define MQTT_SERVER_PORT    1883

// MQTT Broker credentials
#define MQTT_CLIENT_ID ""
#define MQTT_USERNAME  ""
#define MQTT_PASSWORD  ""

// OTA credentials
#define OTA_PASSWORD ""

// DS18B20 pin
#define ONEWIRE_PIN 2

// NTP server address
#define NTP_SERVER ""

// Size of JSON message buffer, for MQTT
#define MSG_BUFFER_SIZE	100

// MQTT Topics
#define MQTT_TELEMETRY_TOPIC "v1/devices/me/telemetry"
#define MQTT_ATTRIBUTE_TOPIC "v1/devices/me/attributes"
