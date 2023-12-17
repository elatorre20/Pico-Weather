#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "secret.h"


//wifi 
char ssid[] = SSID_SECRET; //SSID and password are imported from ignored secret.h for security
char pass[] = PASSWORD_SECRET;
int status = WL_IDLE_STATUS; //status of Wifi connection
char server[] = "forecast.weather.gov";  //National Weather Service Data
WiFiClientSecure client; 


void setup() {
  Serial.begin(9600); //begin USB serial
  status = WiFi.begin(ssid, pass); //attempt wifi connection
  Serial.print("Wifi status: ");
  if(status == WL_CONNECTED){ //if wifi connects successfully
    client.setInsecure(); //connect to SSL server but ignore certs
    Serial.println("Connected to WiFi!");
    if(client.connect(server, 443)){ //for ssl requests
      Serial.println("Connected to Website!");
      client.println("GET https://forecast.weather.gov/product.php?site=BOX&issuedby=BOX&product=RWR&format=txt&version=1&glossary=0 HTTP/1.0"); //latest weather reports for boston region, updated hourly
      client.println("Host:  forecast.weather.gov");
      client.println("User-Agent: Pico_Weather/v0.1");
      client.println("Connection: close");
      client.println();
      delay(1000);
      int count = 0;
      while(client.available()){ 
        count++;
        Serial.print((char)client.read());//print data from client to serial
        // if(count == 64){//line break every 64 characters
        //   count = 0;
        //   Serial.println();
        // }
      }
      Serial.println("Nothing more to read!");
    }
  }
  else{
    Serial.println("Connection Failed!");
  }
}

void loop() { //to know that the setup has finished runnign
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
  
