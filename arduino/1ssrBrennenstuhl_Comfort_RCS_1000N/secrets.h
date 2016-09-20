
#ifndef SECRETSFILE
#define SECRETSFILE

// Wifi settings
#define wifi_ssid "FIXME"
#define wifi_password "FIXME"

// Update these with values suitable for your network.
IPAddress ip(10, 10, 10, 20); //Node static IP
IPAddress gateway(10, 10, 10, 10);
IPAddress subnet(255, 255, 0, 0);
IPAddress dns(10, 10, 10, 10);

//MQTT server config
#define mqtt_server "10.10.10.10"
#define mqtt_user "FIXME"
#define mqtt_password "FIXME"
#define mqtt_client_id "FIXME"

#endif // #ifndef SECRETSFILE
