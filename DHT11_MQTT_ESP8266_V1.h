#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN            2     //////// choose a pin you want to use to read the DHT sensor.
#define DHTTYPE           DHT11     // DHT 11 

DHT dht(DHTPIN, DHTTYPE);

uint32_t delayMS;
          
WiFiClient espClient;
PubSubClient client(espClient);
//long lastMsg = 0;
  //     char msg[50];
    //   char msg1[50];
      // int value = 0;
char* tempTopic = "temp/sensor9";  //// Set to Topic you want to use for MQTT
char* humidityTopic  = "hum/sensor0";   //// Set to Topic you want to use for MQTT
long interval = 200; //(ms) - 200 ms between reports
unsigned long resetPeriod = 86400000; // 1 day - this is the period after which we restart the CPU, to deal with odd memory leak errors
unsigned long prevTime; 
float h, t;  
       
////////// Update these with values suitable for your network.

const char* ssid = "*******************";  ///////////// your ssid.
const char* password = "*********************";  ///////////// Your passwd.
const char* mqtt_server = "192.16*.***.**";     /// IP from your MQTT  Broker

void setup_wifi() {

   Serial.begin(115200);
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
    if (client.connect("ESP8266_Test")) { //set unique client ID

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
  
void setup() {  




     Serial.begin(115200); 
 // Port defaults to 8266
    ArduinoOTA.setPort(8266);




  // Hostname defaults to esp8266-[ChipID]
 ArduinoOTA.setHostname("Bad-unten_DHT11MQTT_ESP8266_V1.0");       ///// Set unique ID if possible with version of software you using




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




  setup_wifi();
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);


}

void loop() {


  
         delay(300000); ////// give a 5min delay///////////
    ////////// it is not needed to reed the temperature so often//////
     


 
  if (!client.connected()) {
    reconnect();


   read_sensors();
  
  }


  client.loop();
 
   read_sensors();
    


  // reset after a day to avoid memory leaks 
  if(millis()>resetPeriod){
    ESP.restart();
  }


}

void read_sensors() {


  prevTime = 0;


  static int counter = 0;
  
  if(prevTime + interval < millis() || prevTime == 0){
    prevTime = millis();
    Serial.println("checking again");
    Serial.println(prevTime);
    
    h = dht.readHumidity();
    t = dht.readTemperature();
    
    h = h*1.23;
    t = t*1.1;
    
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else if(!client.connected()){
      Serial.println("Connection to broker lost; retrying");
    }
    else{
      char* tPayload = f2s(t,0);
      char* hPayload = f2s(h,0);
      
      Serial.println(t);
      Serial.println(h);
      
      Serial.println(tPayload);
      Serial.println(hPayload);


      client.publish(tempTopic, tPayload);
      client.publish(humidityTopic, hPayload);


      Serial.println("published environmental data");
    }
    
  }
  
   client.loop();


}

/* float to string
 * f is the float to turn into a string
 * p is the precision (number of decimals)
 * return a string representation of the float.
 */
char *f2s(float f, int p){
  char * pBuff;                         // use to remember which part of the buffer to use for dtostrf
  const int iSize = 10;                 // number of buffers, one for each float before wrapping around
  static char sBuff[iSize][20];         // space for 20 characters including NULL terminator for each float
  static int iCount = 0;                // keep a tab of next place in sBuff to use
  pBuff = sBuff[iCount];                // use this buffer
  if(iCount >= iSize -1){               // check for wrap
    iCount = 0;                         // if wrapping start again and reset
  }
  else{
    iCount++;                           // advance the counter
  }
  return dtostrf(f, 0, p, pBuff);       // call the library function
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}

/////////////////end read_sensors)///////////////////////////

/////////////////////////END ///////////////////////////////