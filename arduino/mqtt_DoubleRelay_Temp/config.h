
#ifndef CONFIGFILE
#define CONFIGFILE

/*
	MQTT Topics:
	temperature_topic: the topic to send the temperature updates
	relay_1_command_topic: the topic to subscribe to get the relay 1 commands
	rele1_status_topic: the topic where the relay 1 status updates must be published
	rele2_command_topic: the topic to subscribe to get the relay 2 commands
	rele2_status_topic: the topic where the relay 2 status updates must be published
*/
#define temperature_topic "/home/cuci/rb01/temp"
#define relay_1_command_topic "/openHAB/out/CuciRb01Rele1/command"
#define relay_1_status_topic "/openHAB/in/CuciRb01Rele1/state"
#define relay_2_command_topic "/openHAB/out/CuciRb01Rele2/command"
#define relay_2_status_topic "/openHAB/in/CuciRb01Rele2/state"

//led pin is 13
#define RELAY_1_PIN 4
#define RELAY_2_PIN 5
#define SWITCH_1_PIN 12
#define SWITCH_2_PIN 13
#define ONE_WIRE_BUS 2 // DS18B20 on arduino pin2 corresponds to D4 on physical board

int tempDiffToSendUpdate = 1; //this is in tenths of C (1 = 0.1C)
unsigned long tickInterval = 15000; // in millis
unsigned long ticksToForceTempUpdate = 900; // in ticks
unsigned long forceRelayStatusUpdateTicks = 900; // in ticks
unsigned long LONG_PRESS_MILLIS = 2000;

//EEPROM config is not used actually
// #define EEPROM_SWITCH          0
// #define EEPROM_TICK_INTERVAL       4
// #define EEPROM_SEND_INTERVAL_TICKS       8
// #define EEPROM_LAST_TEMP      12
// #define EEPROM_TEMP_DIFF_UPDATE      16
// #define EEPROM_RELAY_1_STATUS      20
// #define EEPROM_RELAY_2_STATUS      24


#endif // #ifndef CONFIGFILE
