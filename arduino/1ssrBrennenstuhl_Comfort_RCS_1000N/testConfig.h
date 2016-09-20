
#ifndef CONFIGFILE
#define CONFIGFILE

/*
	MQTT Topics:
	temperature_topic: the topic to send the temperature updates
	relay_1_command_topic: the topic to subscribe to get the relay 1 commands
	relay_1_status_topic: the topic where the relay 1 status updates must be published
	relay_2_command_topic: the topic to subscribe to get the relay 2 commands
	relay_2_status_topic: the topic where the relay 2 status updates must be published
*/
#define relay_1_command_topic "/test/sockethab/s01/rele1/command"
#define relay_1_status_topic "/test/sockethab/s01/rele1/status"

//led pin is 13
#define RELAY_1_PIN 4
#define SWITCH_1_PIN 12


unsigned long tickInterval = 10000; // in millis
unsigned long forceRelayStatusUpdateTicks = 4; // in ticks

//EEPROM config is not used actually
// #define EEPROM_SWITCH          0
// #define EEPROM_TICK_INTERVAL       4
// #define EEPROM_SEND_INTERVAL_TICKS       8
// #define EEPROM_LAST_TEMP      12
// #define EEPROM_TEMP_DIFF_UPDATE      16
// #define EEPROM_RELAY_1_STATUS      20


#endif // #ifndef CONFIGFILE