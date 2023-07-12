#include <WiFi.h>  
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <Keypad.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DS18B20_PIN  5 // ESP32 pin GIOP5 connected to DS18B20 sensor's DATA pin. address: 0x28, 0x68, 0xB9, 0x48, 0xF6, 0x12, 0x3C, 0x00
#define samp_siz 4 
#define rise_threshold 5

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

OneWire oneWire(DS18B20_PIN);
DallasTemperature DS18B20(&oneWire);

float tempC; // temperature in Celsius
float tempF; // temperature in Fahrenheit

String inputString;
long inputInt;
int length;

byte rowPins[ROWS] = {3, 18, 10, 11}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {12, 13, 14}; //connect to the column pinouts of the keypad


Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
// ***ADD VARIABLE BELOW INITIAL CONFIG***
//---- WiFi settings
const char* ssid = "";
const char* password = "";
//---- MQTT Broker settings
const char* mqtt_server = ""; // replace with your broker url
const char* mqtt_username = ""; // replace with your Credential
const char* mqtt_password = "";
const int mqtt_port = 8883;
//===========================
WiFiClientSecure espClient;  
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
//============= Configure OUTPUT Here============================

const char* Temp_topic= "Temp";
const char* Keypad_topic= "Keypad";

//============= Config Command Here==============================
//const char* command1_topic="command1";
//const char* command1_topic="command2";
static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";
String topic;
String messageTemp;
String message;
void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];
  Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);
  messageTemp = topic;
  Serial.print(messageTemp);
  //==============MESSAGE RECEPTION HERE (Uncomment to edit)====================
  /* if (messageTemp == "MESSAGE INPUT"){
    YOUR VARIABLE = incommingMessage.to**WHAT YOU NEED**();
  }*/
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

      //SUBSCRIBE TO TOPIC HERE

     // client.subscribe("SUBSCRIBE NAME");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//============================================ADD USER VARIABLE HERE====================================



void setup() {
//=========================NO TOUCHY==============================================================
  Serial.begin(9600);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCallback(callback);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
  while (!Serial) delay(1);
  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  delay(1000);
//==================================ADD USER SETUP HERE===================================================


}

void loop() {
//============== NO TOUCHY=======================
   if (!client.connected()) reconnect();
      client.loop();
//============== ADD LOOP CODE HERE==============
  char key = customKeypad.getKey();

  if (key) 
  {
    Serial.println(key);

    if (key >= '0' && key <= '9') 
    {     // only act on numeric keys
      inputString += key;              // append new character to input string
      Serial.println(inputString);
    } 
    else if (key == '*') 
    {
      if (inputString.length() > 0) 
      {
        inputInt = inputString.toInt(); // YOU GOT AN INTEGER NUMBER
        length = inputString.length();
        inputString[length-1] = '\0';  // delete the last character
        Serial.println(inputString);

      }
    } 
    else if (key == '#') 
    {
      Serial.println(inputString);           //Enter
      publishMessage(Keypad_topic,inputString,true);  // send string to HiveMQ
      
    }
  }

 
  DS18B20.requestTemperatures();       // send the command to get temperatures
  tempC = DS18B20.getTempCByIndex(0);  // read temperature in °C
  
  String temp_str;
  temp_str = String(tempC);
  //tempF = tempC * 9 / 5 + 32; // convert °C to °F
  //Serial.print("Temperature: ");
  //Serial.print(tempC);    // print the temperature in °C
  //Serial.print("°C");
  //Serial.println();
  //Serial.print("  ~  ");  // separator between °C and °F
  //Serial.print(tempF);    // print the temperature in °F
  //Serial.println("°F");
  delay(500);


//===============Publish MQTT MESSAGE=======================

publishMessage(Temp_topic,String(temp_str),true);

}


//======================================= publising as string
void publishMessage(const char* topic, String payload , boolean retained){
  if (client.publish(topic, payload.c_str(), true)){
      //Serial.println("Message publised ["+String(topic)+"]: "+payload);
}
}
