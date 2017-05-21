/*
  Combination of my own written code and:
  - https://github.com/bportaluri/WiFiEsp/blob/master/examples/WebClient/WebClient.ino
  - https://github.com/knolleary/pubsubclient/tree/master/examples/mqtt_esp8266
  When data is received from broker (CloudMQTT), generate vibration motor (ROB-08449).
*/

#include "WiFiEsp.h"
#include "PubSubClient.h"

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

const char* ssid = "YourSSID";
const char* password = "YourPassword";
const char* mqtt_server = "YourMqttServer";
const int mqtt_port = 12622;
const char *mqtt_user = "YourUsername";
const char *mqtt_pass = "YourPassword";
// Client connections cant have the same connection name
const char *mqtt_client_name = "generateVibrationClient";

WiFiEspClient espClient;
PubSubClient client(mqtt_server, mqtt_port, espClient);

// Digital pin vibration motor
const int motorPin = 3;

void setup() {
  // initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);
  setup_wifi();
  client.setServer(mqtt_server, 12622);

  pinMode(motorPin, OUTPUT);
}

void setup_wifi() {

  delay(10);
  // Connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connection failed.");
  } else {
    Serial.print("Connection successfull!");
    Serial.println();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Incoming data: ");
  Serial.println(topic);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Generate vibration motor if an 1 received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(motorPin, HIGH);
    Serial.print("Receive payload!");
    delay(2000);
    digitalWrite(motorPin, LOW);
  } else {
    digitalWrite(motorPin, LOW);
    Serial.print("Dont receive payload!");
  }
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connection failed. Make connection again.");
    Serial.println();
    WiFi.begin(ssid, password);
  }

  if (!client.connected()) {
    reconnect();
  } else { // Set callback when connection with broker is successfull
    client.setCallback(callback);
    Serial.println();
    Serial.print("Set callback.");
  }
  client.loop();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("generateVibrationClient", mqtt_user, mqtt_pass)) {
      Serial.println("Connected to CloudMQTT server");
      client.subscribe("vibration");
    } else { 
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" Try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
