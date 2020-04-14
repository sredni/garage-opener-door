#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define NETWORK_SSID        "..."
#define NETWORK_PASSWORD    "..."
#define SERVER_HOST         "..."
#define SERVER_PORT         ...
#define STATUS_TOPIC        "home/garage/door/status"
#define SWITCH_TOPIC        "home/garage/door/switch"
#define SWITCH_PIN          D3
#define STATUS_PIN          D4

int switchTime = 0;
int statusTime = 0;

WiFiClient WiFiClient;
PubSubClient client(WiFiClient);

void setup() {
  pinMode(STATUS_PIN, OUTPUT);
  pinMode(SWITCH_PIN, OUTPUT);
  digitalWrite(STATUS_PIN, HIGH);
  digitalWrite(SWITCH_PIN, LOW);
  Serial.begin(115200);
  delay(100);
  Serial.println("HELLO");
  Serial.print("Connecting to ");
  Serial.println(NETWORK_SSID);
  WiFi.begin(NETWORK_SSID, NETWORK_PASSWORD);

  client.setServer(SERVER_HOST, SERVER_PORT);
  client.setCallback(callback);
}

void loop() {
  delay(1000);
  
  if (WiFi.status() == WL_CONNECTED) {
    connectMQTT();
    client.loop();
  } else {
    Serial.println("Trying to connect...");
  }

  if (switchTime > 0) {
    Serial.println("ON");
    digitalWrite(SWITCH_PIN, HIGH);
    digitalWrite(STATUS_PIN, LOW);
    --switchTime;
    
    if (switchTime == 0) {
      byte nil[0];
      client.publish(SWITCH_TOPIC, nil, 0, true);
    }
  } else {
    Serial.println("OFF");
    digitalWrite(SWITCH_PIN, LOW);
    digitalWrite(STATUS_PIN, HIGH);
        
    if (statusTime > 0) {
      --statusTime;

      if (statusTime == 0) {
        client.publish(STATUS_TOPIC, "0", true);
      }
    }
  }
  
  Serial.println(".");
}

void callback(char* topic, byte* data, unsigned int length) {
  char charData[length+1];
  
  for (int i=0;i<length;i++) {
    charData[i] = (char)data[i];
  }
  
  int switchValue = atoi(charData);

  Serial.println(switchValue);

  if (switchValue) {
    client.publish(STATUS_TOPIC, "1", true);
    switchTime = 5;
    statusTime = 10;
  }
}

void connectMQTT() {
  if (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("")) {
      Serial.println("connected");
      client.subscribe(SWITCH_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
  
