#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <TinyGPS++.h>
#include <WiFiClientSecure.h>

#define REDLED 4
#define YELLOWLED 10
#define INTERVAL 5000

TinyGPSPlus gps;
unsigned long lastsend;
const char* host="example.execute-api.eu-west-1.amazonaws.com";
const int httpsPort=443;

void setup() {
  Serial.begin(9600, SERIAL_8N1); //start the serial interface for the gps board
  Serial.swap(); //GPS is connected with tx only (we're not sending any data TO the gps module) on pin 13, the alternative UART0 rx pin on the ESP8266. Useful so the gps does not interfere with code uploads
  Serial1.begin(9600); //UART1 only has a tx port, all we need to enable some debugging output
  pinMode(REDLED, OUTPUT); 
  pinMode(YELLOWLED, OUTPUT);
  digitalWrite(REDLED,LOW);
  digitalWrite(YELLOWLED,HIGH);
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(false); //not interfere with Serial connection for GPS
  wifiManager.setAPCallback(wifiConfigMode);
  wifiManager.autoConnect("GpsDevice");
  //if we get here we have a wifi connection
  digitalWrite(REDLED,LOW);
  digitalWrite(YELLOWLED,LOW);
}

void loop() {
  
  // put your main code here, to run repeatedly:
  if ((millis()-lastsend)>INTERVAL){
    lastsend=millis();
    digitalWrite(YELLOWLED,HIGH);
    if(gps.location.isValid()){
      doSendLocation();
    }else{
      digitalWrite(REDLED,HIGH);
      Serial1.println(gps.satellites.value());
    }
    delay(200);
    digitalWrite(REDLED,LOW);
    digitalWrite(YELLOWLED,LOW);
  }      
  while(Serial.available()){
    gps.encode(Serial.read());
    yield();
  }
  

}
void doSendLocation(){
  WiFiClientSecure client;
  String request;
  int brackets=0;
  
  request=String(F("GET /Prod?action=put&key=IOT4ALL&deviceId=marcos&lat="));
  request+=String(gps.location.lat(),6);
  request+=String(F("&lng="));
  request+=String(gps.location.lng(),6);
  request+=String(F(" HTTP/1.1\r\nUser-Agent: GPSRowingDevice/0.1\r\nHost: "));
  request+=String(host);
  request+=String(F("\r\nConnection: close\r\n\r\n"));

  Serial1.println(request);
  yield();

  if(client.connect(host,httpsPort)){
    client.print(request);
    while(client.connected()){
      while(client.available()){
        char c=client.read();
//        Serial1.print(c);
        if(c=='{'){
          brackets+=1;
        }else if(c=='}') {
          brackets-=1;
          if (brackets==0){
            client.stop();
          }
        }
        yield();
      }
      yield();
    }
    delay(200);
  }else{
    digitalWrite(YELLOWLED,LOW);
    digitalWrite(REDLED,HIGH);
    delay(1000);
  }
}
void wifiConfigMode (WiFiManager *myWiFiManager){
  digitalWrite(REDLED,HIGH);
}
