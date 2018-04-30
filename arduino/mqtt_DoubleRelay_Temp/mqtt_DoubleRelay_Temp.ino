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
  #include <OneWire.h>
  #include <PubSubClient.h>
  #include <DallasTemperature.h>
//#include <EEPROM.h>
#include "mysecrets.h"
#include "config.h"
#include "ardprintf.h"


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

int lastTempMultiplied = 0;

String ON = "ON";
String OFF = "OFF";

String relay1Status = OFF;
String relay2Status = OFF;

//switch related variables
int switch1LastReading = HIGH;
int switch2LastReading = HIGH;
long lastSwitchTime = 0;
long switchDebounce = 200;

unsigned long lastConnectionCheckTime = 0;
unsigned long lastTickTime = 0;
unsigned long tickCounter = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(SWITCH_1_PIN, INPUT);
  pinMode(SWITCH_2_PIN, INPUT);
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
  client.subscribe(relay_2_command_topic);
  Serial.println("Subscribed!!");

}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  unsigned long start = millis();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.setPhyMode(WIFI_PHY_MODE_11G);
  WiFi.mode(WIFI_STA);

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
 // while (!client.connected()) {
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
      client.subscribe(relay_2_command_topic);
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        //Serial.println(" try again in 1 second");
      // Wait 1 second before retrying
      //delay(1000);
    }
  //}
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
  String payloadString = String((char*)payload);
  if (strcmp(topic,relay_1_command_topic)==0) {
    if (OFF.equalsIgnoreCase(payloadString)){
      switchRelay(RELAY_1_PIN, OFF);
    } else if(ON.equalsIgnoreCase(payloadString)) {
      switchRelay(RELAY_1_PIN, ON);
    }
  } 
  if (strcmp(topic,relay_2_command_topic)==0) {
    if (OFF.equalsIgnoreCase(payloadString)){
      switchRelay(RELAY_2_PIN, OFF);
    } else if(ON.equalsIgnoreCase(payloadString)) {
      switchRelay(RELAY_2_PIN, ON);
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
    lastConnectionCheckTime = millis();
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
    verifyAndSendTemp();
    //update counters
    tickCounter++;
    lastTickTime = millis();
    ardprintf("Tick Loop completed in millis: %d", millis() - loopStartMillis);
  }
}

void verifyAndSendTemp() {
  //Read Temp
  DS18B20.requestTemperatures();
  float newTemp = DS18B20.getTempCByIndex(0);
  int newTempMultiplied = (int)newTemp * 10;
  //send temp every x ticks or if temp changed
  if ((tickCounter % ticksToForceTempUpdate) == 0 || checkBound(newTempMultiplied, lastTempMultiplied, tempDiffToSendUpdate)) {
    char buffer[10];
    String tempC = dtostrf(newTemp, 4, 1, buffer);//handled in sendTemp()
    ardprintf("publishing new temperature: %d", newTemp);
    client.publish(temperature_topic, String(tempC).c_str(), true);
    lastTempMultiplied = newTempMultiplied;
  }
}

void checkSwitches() {
    //Using long press on switch 1 to activate relay 2
    int switch1Reading = digitalRead(SWITCH_1_PIN);
    if(switch1Reading == LOW && switch1LastReading == HIGH && millis() - lastSwitchTime > switchDebounce){
      long startMillis = millis();
      while(millis() - startMillis < 5000 && switch1Reading == LOW){
        client.loop(); //client loop as this loop could be long
        //testing the button again in 50 msec to try to avoid noise
        delay(50);
        switch1Reading = digitalRead(SWITCH_1_PIN);
        if(switch1Reading == HIGH && millis() - startMillis < LONG_PRESS_MILLIS) {
          Serial.println("Switching relay1 by connected switch");
          if(ON.equalsIgnoreCase(relay1Status)) {
            switchRelay(RELAY_1_PIN, OFF);
          } else {
            switchRelay(RELAY_1_PIN, ON);
          }
          lastSwitchTime = millis();
          break;
        } else if (switch1Reading == LOW && millis() - startMillis > LONG_PRESS_MILLIS){
          Serial.println("Switching relay1 by long press on connected switch");
          if(ON.equalsIgnoreCase(relay2Status)){
            switchRelay(RELAY_2_PIN, OFF);
          } else {
            switchRelay(RELAY_2_PIN, ON);
          }
          lastSwitchTime = millis();
          break;
        }
      }
    }
      switch1LastReading = switch1Reading;

  //read and process switch 2
  int switch2Reading = digitalRead(SWITCH_2_PIN);
  if(switch2Reading == LOW && switch2LastReading == HIGH && millis() - lastSwitchTime > switchDebounce){
      //testing the button again in 50 msec to try to avoid noise
      delay(50);
      switch2Reading = digitalRead(SWITCH_1_PIN);
      if(switch2Reading == LOW){
        Serial.println("Switching relay2 by connected switch");
        if(ON.equalsIgnoreCase(relay2Status)){
          switchRelay(RELAY_2_PIN, OFF);
          } else {
            switchRelay(RELAY_2_PIN, ON);
          }
          lastSwitchTime = millis();
        } 
      }
      switch2LastReading = switch2Reading;
    }

void switchRelay(int relayPin, String status){
	ardprintf("switchRelay() start relayPin: %d status %s", relayPin, status);
	if(relayPin == RELAY_1_PIN && ON.equalsIgnoreCase(status)){
		digitalWrite(relayPin, LOW);
		client.publish(relay_1_status_topic, ON.c_str());
		relay1Status = ON;
	} else if(relayPin == RELAY_1_PIN && OFF.equalsIgnoreCase(status)) {
		digitalWrite(relayPin, HIGH);
		client.publish(relay_1_status_topic, OFF.c_str());
		relay1Status = OFF;
	} else if(relayPin == RELAY_2_PIN && ON.equalsIgnoreCase(status)){
		digitalWrite(relayPin, LOW);
		client.publish(relay_2_status_topic, ON.c_str());
		relay2Status = ON;
	} else if(relayPin == RELAY_2_PIN && OFF.equalsIgnoreCase(status)) {
		digitalWrite(relayPin, HIGH);
		client.publish(relay_2_status_topic, OFF.c_str());
		relay2Status = OFF;
	}
	
}

