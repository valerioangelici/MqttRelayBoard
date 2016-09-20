/*
  Mqtt ESP8266 relay

  sketch to manage 2 relays via mqtt and physical buttons

  this sketch is built to manage two relays with openhab
  items using the mqtt binding and two physical buttons
  every time a relay is switched on/off the new state is
  sento to a mqtt topic so that openhab is updated even
  when the relay is switched with the physical buttons

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//#include <EEPROM.h>
#include "mysecrets.h"
#include "config.h"
#include "ardprintf.h"

int relay1Status = 0;

//switch related variables
int switch1LastReading = HIGH;
long lastSwitchTime = 0;
long switchDebounce = 200;

unsigned long lastTickTime = 0;
unsigned long tickCounter = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(SWITCH_1_PIN, INPUT);
  Serial.begin(115200);

  Serial.println("Hello world!!");

  ardprintf("Tick counter: %d", tickCounter);
  ardprintf("Last temp: %d", lastTempMultiplied);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("subscribing...");
  client.subscribe(relay_1_command_topic);
  client.loop();
  Serial.println("Subscribed!!");

}

// Set SDA and SDL ports
// Wire.begin(2, 14);


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  unsigned long start = millis();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);
  WiFi.config(ip, gateway, subnet, dns);

  while (WiFi.status() != WL_CONNECTED) {
    delay(10);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Connected in: ");
  Serial.println(millis() - start);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("helloTopic", "hello world");
      // ... and resubscribe
      client.subscribe(relay_1_command_topic);
      client.loop();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 second");
      // Wait 1 second before retrying
      delay(1000);
    }
  }
}

bool checkBound(int newValue, int prevValue, int maxDiff) {
  return newValue < prevValue - maxDiff || newValue > prevValue + maxDiff;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Callback");
  Serial.print("Topic:");
  Serial.println(topic);
  Serial.print("Length:");
  Serial.println(length);
  Serial.print("Payload:");
  Serial.write(payload,length);
  Serial.println();

  if (strcmp(topic,relay_1_command_topic)==0) {
    if (payload[0] == '0'){
      switchRelay(RELAY_1_PIN, LOW);
    } else if(payload[0] == '1') {
      switchRelay(RELAY_1_PIN, HIGH);
    }
  }

}

void loop() {
  long loopStartMillis = millis();

  //firstly check for millis overflow (every about 50 days)  
  if(lastTickTime > millis()){
    Serial.println("millis overflow, resetting counters");
    //millis overflow , reset the other counters too
    lastTickTime = millis();
    tickCounter = 0;
  }
  //client loop to allow wifi and mqtt packets handling
  client.loop();
  //Check for a switch press
  checkSwitches();

  //check if is passed enough time before last tick to generate a new one
  if(millis() - lastTickTime > tickInterval){
    ardprintf("Tick!! counter: %d", tickCounter);
    //check for client disconnection
    if (!client.connected()) {
      reconnect();
    }
    //handle temp sensor actions
    //update counters
    tickCounter++;
    lastTickTime = millis();
    ardprintf("Tick Loop completed in millis: %d", millis() - loopStartMillis);
  }
}

void checkSwitches() {
  //read and process switch 1
  int switch1Reading = digitalRead(SWITCH_1_PIN);
    if(switch1Reading == LOW && switch1LastReading == HIGH && millis() - lastSwitchTime > switchDebounce){
      //testing the button again in 50 msec to try to avoid noise
      delay(50);
      switch1Reading = digitalRead(SWITCH_1_PIN);
      if(switch1Reading == LOW){
        Serial.println("Switching relay1 by connected switch");
        if(relay1Status == 1){
          switchRelay(RELAY_1_PIN, LOW);
        } else {
          switchRelay(RELAY_1_PIN, HIGH);
        }
        lastSwitchTime = millis();
      } 
    }
  switch1LastReading = switch1Reading;

}

void switchRelay(int relayPin, int status){
  ardprintf("switchRelay() start relayPin: %d status %d", relayPin, status);
  //check that pin is valid
  if(relayPin == RELAY_1_PIN){
    //check that status is valid
    if(status == HIGH || status == LOW){
      //if pin and status are valid digital write the new value
      digitalWrite(relayPin, status);
      int statusInt = 0;
      if(status == HIGH){
        statusInt = 1;
      }
      if(relayPin == RELAY_1_PIN){
        client.publish(relay_1_status_topic, String(statusInt).c_str());
        relay1Status = statusInt;
      }
    }//status pin check
  }//relay pin check
}
