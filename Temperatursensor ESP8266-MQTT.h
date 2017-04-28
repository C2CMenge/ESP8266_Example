#include <DallasTemperature.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
//OneWire  ds(2);
#define SENSOR_PIN 2           //////// choose a pin you want to use /*** on NodeMCU 2 = D4//
OneWire ourBus(SENSOR_PIN);
DallasTemperature sensors(&ourBus);


/*-----( Declare Variables )-----*/
// Assign the addresses of your 1-Wire temp sensors.

    DeviceAddress Probe01 = { 0x28,0xFF,0x18,0x59,0xB3,0x16,0x04,0x3E }; //// you can read the adress in serial monitor
    DeviceAddress Probe02 = { 0x28,0xFF,0x92,0x16,0x72,0x16,0x03,0x49 };  //// you can read the adress in serial monitor
    DeviceAddress Probe03 = { 0x28,0xFF,0x47,0xF2,0xB2,0x16,0x05,0x0C };  //// you can read the adress in serial monitor
    DeviceAddress Probe04 = { 0x28, 0x9A, 0x80, 0x40, 0x04, 0x00, 0x00, 0xD5 };  //// you can read the adress in serial monitor
    DeviceAddress Probe05 = { 0x28, 0xE1, 0xC7, 0x40, 0x04, 0x00, 0x00, 0x0D };  //// you can read the adress in serial monitor






////////// Update these with values suitable for your network.


const char* ssid = "***************";  ///////////// your ssid.
const char* password = "*************";  ///////////// Your passwd.
const char* mqtt_server = "192.16*.***.**";     /// IP from your MQTT  Broker




WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
const char* outTopic1 = "temp/sensor0";  //// Set to Topic you want to use for MQTT
const char* outTopic  = "temp/sensor";   //// Set to Topic you want to use for MQTT
const char* outTopic6 = "temp/sensor6";  //// Set to Topic you want to use for MQTT
const char* outTopic7 = "temp/sensor7";  //// Set to Topic you want to use for MQTT
const char* outTopic8 = "temp/sensor8";  //// Set to Topic you want to use for MQTT


  




void setup() { 


  // Port defaults to 8266
  ArduinoOTA.setPort(8266);


  // Hostname defaults to esp8266-[ChipID]
 ArduinoOTA.setHostname("myesp8266");


  // No authentication by default
 // ArduinoOTA.setPassword((const char *)"123");


  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  


  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);


  dicoverOneWireDevices();   /////// is searchin after reset for new sensors
                             ///////you can see the address in serial monitor 


                             
    sensors.begin();
    sensors.setResolution(Probe01,11);
    sensors.setResolution(Probe02,11);
    sensors.setResolution(Probe03,11);
    sensors.setResolution(Probe04,11);
    sensors.setResolution(Probe05,11);


    ////// 11 fit for ESP8266




}


void setup_wifi() {


  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);


  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }


  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();


}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client01")) { //set unique client ID


      Serial.println("connected");
      // client.subscribe("event");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {


  
         delay(300000); ////// give a 5min delay///////////
    ////////// it is not needed to reed the temperature so often//////
  
 
   // dicoverOneWireDevices();  ///not needed in loop but usefull for figure out the address
 
  if (!client.connected()) {
    reconnect();


    publish_to_MQTT();   //// MQTT function is called here


    
  }


  
  client.loop();
  


 
  Serial.println();
  Serial.print("Number of Devices found on bus = ");  
  Serial.println(sensors.getDeviceCount());   
  Serial.print("Getting temperatures... ");  
  Serial.println();   
  
 //////// Command all devices on bus to read temperature  
  sensors.requestTemperatures();  
  /////// switch back on what you need
  Serial.print("Probe 01 temperature is:   ");
  printTemperature(Probe01);
  Serial.println();


  Serial.print("Probe 02 temperature is:   ");
  printTemperature(Probe02);
  Serial.println();
  /*
  Serial.print("Probe 03 temperature is:   ");
  printTemperature(Probe03);
  Serial.println();
 /*  
  Serial.print("Probe 04 temperature is:   ");
  printTemperature(Probe04);
  Serial.println();
  /*
  Serial.print("Probe 05 temperature is:   ");
  printTemperature(Probe05);
  Serial.println();
 */ 
}






//--(end main loop )---


 //////// we start to publish some messages to MQTT Broker///
 ////////////Functionis called in void loop///////////
 void publish_to_MQTT()    {
  
 
  
   float tempC1 = sensors.getTempC(Probe01);
    unsigned long previousMillis =0;
    const long interval =2000;
    char Probe01 [10];
   dtostrf(tempC1,7, 3,msg);  //// convert float to char 
   client.publish(outTopic1,msg);   /// send char,set unique ID
   Serial.print("\n\rPROBE 1 Gesendet\n\r");
    Serial.print(msg);
    delay(10);                         // delay for pushing the data to the broker 


    
  float tempC2 = sensors.getTempC(Probe02);
   char Probe02 [10];
   dtostrf(tempC2,7, 3, Probe02);  //// convert float to char 
   client.publish(outTopic,Probe02);   /// send char,set unique ID
   
   Serial.print("\n\rPROBE 2 Gesendet\n\r");
    Serial.print(Probe02);
    delay(10);                         // delay for pushing the data to the broker 


/*


   float tempC3 = sensors.getTempC(Probe03);
   char Probe03 [10];
   dtostrf(tempC3,7, 3, Probe03);  //// convert float to char 
   client.publish(outTopic6,Probe03);   /// send char,set unique ID
   
   Serial.print("\n\rPROBE 3 Gesendet\n\r");
    Serial.print(Probe03);
    delay(10);                         // delay for pushing the data to the broker




/*
      float tempC4 = sensors.getTempC(Probe04);
   char Probe04 [10];
   dtostrf(tempC4,7, 3, Probe04);  //// convert float to char 
   client.publish(outTopic7,Probe04);   /// send char,set unique ID
   
   Serial.print("\n\rPROBE 4 Gesendet\n\r");
    Serial.print(Probe04);
    delay(10);                         // delay for pushing the data to the broker


/*
      float tempC5 = sensors.getTempC(Probe05);
   char Probe05 [10];
   dtostrf(tempC5,7, 3, Probe05);  //// convert float to char 
   client.publish(outTopic8,Probe05);   /// send char,set unique ID
   
   Serial.print("\n\rPROBE 5 Gesendet\n\r");
    Serial.print(Probe05);
    delay(10);                         // delay for pushing the data to the broker


*/
 }
      
  ///////////// END MQTT Function ///////////////////////////////


/*-----( Declare User-written Functions )-----*/
void printTemperature(DeviceAddress deviceAddress)
{


float tempC = sensors.getTempC(deviceAddress);


  if (tempC == -127.00) 
  {
   Serial.print("Error getting temperature  ");
  } 
   else
   {
   Serial.print("C: ");
   Serial.print(tempC);
  // Serial.print(" F: ");
   Serial.print(DallasTemperature::toFahrenheit(tempC));
  }
}


/////////////// End printTemperature //////////////////////////


/////////This Function is looking for new devices and write the adress to serial Monitor/////////
void dicoverOneWireDevices(void){
  
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];


  Serial.print("Look for Devices...\n\r");
  while(ourBus.search(addr)){
       Serial.print("\n\r\n\rFound \`1-Wire\` device with adress:\n\r");
       for(i = 0; i < 8; i++){
        Serial.print("0x");
        if (addr[i] < 16){
          Serial.print("0");
          
        }
        Serial.print(addr[i],HEX);
        if (i < 7) {
          Serial.print(",");
          }
       }


       if (OneWire::crc8( addr,7) != addr[7]){
        Serial.print("CRC is not valid\n\r");
        return;
       }
  }
  Serial.println();
  Serial.print("Done");
  ourBus.reset_search();
  return;
  }


/////////////END ///////////////////////////////
