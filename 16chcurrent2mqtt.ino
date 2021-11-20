// 16 Channel Current Meter to MQTT
// Repo: https://github.com/nygma2004/16chcurrent2mqtt
// author: Csongor Varga, csongor.varga@gmail.com

// Libraries:
// - FastLED by Daniel Garcia
// Hardware:



#include <SoftwareSerial.h>       // Leave the main serial line (USB) for debugging and flashing
#include <ModbusMaster.h>         // Modbus master library for ESP8266
#include <ESP8266WiFi.h>          // Wifi connection
#include <ESP8266WebServer.h>     // Web server for general HTTP response
#include <PubSubClient.h>         // MQTT support
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include "globals.h"
#include "settings.h"



os_timer_t myTimer;
ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient mqtt(mqtt_server, 1883, 0, espClient);
// SoftwareSerial modbus(MAX485_RX, MAX485_TX, false, 256); //RX, TX
SoftwareSerial modbus(MAX485_RX, MAX485_TX, false); //RX, TX
ModbusMaster current;
CRGB leds[NUM_LEDS];

void preTransmission() {
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission() {
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void sendModbusError(uint8_t result) {
  String message = "";
  if (result==current.ku8MBIllegalFunction) {
    message = "Illegal function";
  }
  if (result==current.ku8MBIllegalDataAddress) {
    message = "Illegal data address";
  }
  if (result==current.ku8MBIllegalDataValue) {
    message = "Illegal data value";
  }
  if (result==current.ku8MBSlaveDeviceFailure) {
    message = "Slave device failure";
  }
  if (result==current.ku8MBInvalidSlaveID) {
    message = "Invalid slave ID";
  }
  if (result==current.ku8MBInvalidFunction) {
    message = "Invalid function";
  }
  if (result==current.ku8MBResponseTimedOut) {
    message = "Response timed out";
  }
  if (result==current.ku8MBInvalidCRC) {
    message = "Invalid CRC";
  }
  if (message=="") {
    message = result;
  }
  Serial.println(message);    
  char topic[80];
  char value[30];
  sprintf(topic,"%s/error",topicRoot);
  mqtt.publish(topic, message.c_str());
  delay(5);
}

void ReadHoldingRegisters() {
  char json[2048];
  char topic[80];
  char value[10]; 

  // Query the modbus device 
  if (seconds % UPDATE_MODBUS==0) {
    leds[0] = CRGB::Yellow;
    FastLED.show();
    uint8_t result;
    //uint16_t data[6];

    digitalWrite(STATUS_LED, 0);
    ESP.wdtDisable();
    result = current.readHoldingRegisters(0,56);
    ESP.wdtEnable(1);
    if (result == current.ku8MBSuccess)   {

      leds[0] = CRGB::Green;
      FastLED.show();
      lastRGB = millis();
      ledoff = true;
      
      #ifdef DEBUG_SERIAL
        Serial.println("Raw: ");
      #endif
      sprintf(json,"{");
      for(int i=0;i<56;i++) {

        #ifdef DEBUG_SERIAL
          sprintf(value,"%d|",current.getResponseBuffer(i));
          Serial.print(value);
        #endif
        #ifdef DEBUG_MQTT
          sprintf(json,"%s \"r%d\":%d,",json,i,current.getResponseBuffer(i));
        #endif
        
      }
      sprintf(json,"%s \"end\":1 }",json);

      #ifdef DEBUG_SERIAL
        Serial.println();
      #endif
      #ifdef DEBUG_MQTT
        sprintf(topic,"%s/raw",topicRoot);
        mqtt.publish(topic, json);
      #endif


        // modbusdata.status = growatt.getResponseBuffer(0);

      sprintf(json,"{",json);
      sprintf(json,"%s \"version\":\"%d.%d.%d\",",json,current.getResponseBuffer(0)/100,current.getResponseBuffer(0)/10%10,current.getResponseBuffer(0)%10);
      sprintf(json,"%s \"rangeah\":%d,",json,current.getResponseBuffer(1));
      sprintf(json,"%s \"rangeip\":%d,",json,current.getResponseBuffer(2));
      
      sprintf(json,"%s \"c01\":%.2f,",json,current.getResponseBuffer(8)*0.01);
      sprintf(json,"%s \"c02\":%.2f,",json,current.getResponseBuffer(9)*0.01);
      sprintf(json,"%s \"c03\":%.2f,",json,current.getResponseBuffer(10)*0.01);
      sprintf(json,"%s \"c04\":%.2f,",json,current.getResponseBuffer(11)*0.01);
      sprintf(json,"%s \"c05\":%.2f,",json,current.getResponseBuffer(12)*0.01);
      sprintf(json,"%s \"c06\":%.2f,",json,current.getResponseBuffer(13)*0.01);
      sprintf(json,"%s \"c07\":%.2f,",json,current.getResponseBuffer(14)*0.01);
      sprintf(json,"%s \"c08\":%.2f,",json,current.getResponseBuffer(15)*0.01);
      sprintf(json,"%s \"c09\":%.2f,",json,current.getResponseBuffer(16)*0.01);
      sprintf(json,"%s \"c10\":%.2f,",json,current.getResponseBuffer(17)*0.01);
      sprintf(json,"%s \"c11\":%.2f,",json,current.getResponseBuffer(18)*0.01);
      sprintf(json,"%s \"c12\":%.2f,",json,current.getResponseBuffer(19)*0.01);
      sprintf(json,"%s \"c13\":%.2f,",json,current.getResponseBuffer(20)*0.01);
      sprintf(json,"%s \"c14\":%.2f,",json,current.getResponseBuffer(21)*0.01);
      sprintf(json,"%s \"c15\":%.2f,",json,current.getResponseBuffer(22)*0.01);
      sprintf(json,"%s \"c16\":%.2f }",json,current.getResponseBuffer(23)*0.01);

/*
      sprintf(json,"%s \"f01\":%.1f,",json,current.getResponseBuffer(24)*0.1);
      sprintf(json,"%s \"f02\":%.1f,",json,current.getResponseBuffer(25)*0.1);
      sprintf(json,"%s \"f03\":%.1f,",json,current.getResponseBuffer(26)*0.1);
      sprintf(json,"%s \"f04\":%.1f,",json,current.getResponseBuffer(27)*0.1);
      sprintf(json,"%s \"f05\":%.1f,",json,current.getResponseBuffer(28)*0.1);
      sprintf(json,"%s \"f06\":%.1f,",json,current.getResponseBuffer(29)*0.1);
      sprintf(json,"%s \"f07\":%.1f,",json,current.getResponseBuffer(30)*0.1);
      sprintf(json,"%s \"f08\":%.1f,",json,current.getResponseBuffer(31)*0.1);
      sprintf(json,"%s \"f09\":%.1f,",json,current.getResponseBuffer(32)*0.1);
      sprintf(json,"%s \"f10\":%.1f,",json,current.getResponseBuffer(33)*0.1);
      sprintf(json,"%s \"f11\":%.1f,",json,current.getResponseBuffer(34)*0.1);
      sprintf(json,"%s \"f12\":%.1f,",json,current.getResponseBuffer(35)*0.1);
      sprintf(json,"%s \"f13\":%.1f,",json,current.getResponseBuffer(36)*0.1);
      sprintf(json,"%s \"f14\":%.1f,",json,current.getResponseBuffer(37)*0.1);
      sprintf(json,"%s \"f15\":%.1f,",json,current.getResponseBuffer(38)*0.1);
      sprintf(json,"%s \"f16\":%.1f }",json,current.getResponseBuffer(39)*0.1);
*/

        
        Serial.println(json);
        sprintf(topic,"%s/data",topicRoot);
        mqtt.publish(topic,json);      
        
      //sprintf(topic,"%s/error",topicRoot);
      //mqtt.publish(topic,"OK");

    } else {
      leds[0] = CRGB::Red;
      FastLED.show();
      lastRGB = millis();
      ledoff = true;

      Serial.print(F("Error: "));
      sendModbusError(result);
    }
    digitalWrite(STATUS_LED, 1);


  }

    
}





// This is the 1 second timer callback function
void timerCallback(void *pArg) {

  
  seconds++;

  ReadHoldingRegisters();

  // Send RSSI and uptime status
  if (seconds % UPDATE_STATUS==0) {
    // Send MQTT update
    if (mqtt_server!="") {
      char topic[80];
      char value[300];
      sprintf(value,"{\"rssi\": %d, \"uptime\": %d, \"ssid\": \"%s\", \"ip\": \"%d.%d.%d.%d\", \"clientid\":\"%s\", \"version\":\"%s\"}",WiFi.RSSI(),uptime,WiFi.SSID().c_str(),WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3],newclientid,buildversion);
      sprintf(topic,"%s/%s",topicRoot,"status");
      mqtt.publish(topic, value);
      Serial.println(F("MQTT status sent"));
    }
  }


}

// MQTT reconnect logic
void reconnect() {
  //String mytopic;
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    byte mac[6];                     // the MAC address of your Wifi shield
    WiFi.macAddress(mac);
    sprintf(newclientid,"%s-%02x%02x%02x",clientID,mac[2],mac[1],mac[0]);
    Serial.print(F("Client ID: "));
    Serial.println(newclientid);
    // Attempt to connect
    if (mqtt.connect(newclientid, mqtt_user, mqtt_password)) {
      Serial.println(F("connected"));
      // ... and resubscribe
      char topic[80];
      sprintf(topic,"%swrite/#",topicRoot);
      mqtt.subscribe(topic);
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(mqtt.state());
      Serial.println(F(" try again in 5 seconds"));
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup() {

  FastLED.addLeds<LED_TYPE, RGBLED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness( BRIGHTNESS );
  leds[0] = CRGB::Pink;
  FastLED.show();

  Serial.begin(SERIAL_RATE);
  Serial.println(F("\n16 Channel Current Meter to MQTT Gateway"));
  // Init outputs, RS485 in receive mode
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  // Initialize some variables
  uptime = 0;
  seconds = 0;
  leds[0] = CRGB::Pink;
  FastLED.show();

  // Connect to Wifi
  Serial.print(F("Connecting to Wifi"));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
    seconds++;
    if (seconds>180) {
      // reboot the ESP if cannot connect to wifi
      ESP.restart();
    }
  }
  seconds = 0;
  Serial.println("");
  Serial.println(F("Connected to wifi network"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("Signal [RSSI]: "));
  Serial.println(WiFi.RSSI());

  // Set up the Modbus line
  current.begin(SLAVE_ID , modbus);
  // Callbacks allow us to configure the RS485 transceiver correctly
  current.preTransmission(preTransmission);
  current.postTransmission(postTransmission);
  Serial.println("Modbus connection is set up");

  // Create the 1 second timer interrupt
  os_timer_setfn(&myTimer, timerCallback, NULL);
  os_timer_arm(&myTimer, 1000, true);

  server.on("/", [](){                        // Dummy page
    server.send(200, "text/plain", "16 Channel Current Meter to MQTT Gateway");
  });
  server.begin();
  Serial.println(F("HTTP server started"));

  // Set up the MQTT server connection
  if (mqtt_server!="") {
    mqtt.setServer(mqtt_server, 1883);
    mqtt.setBufferSize(1024);
    mqtt.setCallback(callback);
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  byte mac[6];                     // the MAC address of your Wifi shield
  WiFi.macAddress(mac);
  char value[80];
  sprintf(value,"%s-%02x%02x%02x",clientID,mac[2],mac[1],mac[0]);
  ArduinoOTA.setHostname(value);

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

  modbus.begin(MODBUS_RATE);
  
  leds[0] = CRGB::Black;
  FastLED.show();
  
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Convert the incoming byte array to a string
  String mytopic = (char*)topic;
  payload[length] = '\0'; // Null terminator used to terminate the char array
  String message = (char*)payload;

  Serial.print(F("Message arrived on topic: ["));
  Serial.print(topic);
  Serial.print(F("], "));
  Serial.println(message);


}

void loop() {
  // Handle HTTP server requests
  server.handleClient();
  ArduinoOTA.handle();

  // Handle MQTT connection/reconnection
  if (mqtt_server!="") {
    if (!mqtt.connected()) {
      reconnect();
    }
    mqtt.loop();
  }

  // Uptime calculation
  if (millis() - lastTick >= 60000) {            
    lastTick = millis();            
    uptime++;            
  }    

  if (millis() - lastWifiCheck >= WIFICHECK) {
    // reconnect to the wifi network if connection is lost
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting to wifi...");
      WiFi.reconnect();
    }
    lastWifiCheck = millis();
  }

  if (ledoff && (millis() - lastRGB >= RGBSTATUSDELAY)) {
    ledoff = false;
    leds[0] = CRGB::Black;
    FastLED.show();
  }


}
