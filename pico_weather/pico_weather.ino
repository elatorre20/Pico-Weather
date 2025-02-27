#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "secret.h"

//scheduling
uint32_t task_millis[3]; //used to keep track of execution times of tasks [remote update, check mode]
#define DOWNLOAD_UPDATE_PERIOD 60000 //download temperatures every 60 seconds
#define MODE_UPDATE_PERIOD 500 //check every half second what metric is wanted and write it

//wifi 
char ssid[] = SSID_SECRET; //SSID and password are imported from ignored secret.h for security
char pass[] = PASSWORD_SECRET;
int status = WL_IDLE_STATUS; //status of Wifi connection
char server[] = "forecast.weather.gov";  //National Weather Service Data
WiFiClientSecure client; 

//data
char site_data[65535]; //buffer to hold the downloaded web page
uint8_t temperature, humidity, indoor_temperature, indoor_humidity, data_temp; //weather stats to be displayed
char data_buf[3]; //to hold the strings to be parsed for values

void updateRemoteTemps(){
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println((String)"Wifi status: " + (status == WL_CONNECTED));
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
      delay(3000); //wait for data to load
      memset(site_data, 0, sizeof(site_data));
      uint16_t count = 0;
      while(client.available() && count < sizeof(site_data)){//read site data into buffer
        site_data[count] = (char)client.read();
        //Serial.print(site_data[count]);
        count++;
      }
      Serial.println("\n");
      char* data_start = strstr(site_data, "\nBOSTON"); //start point of the data we want
      memset(data_buf, 0, 3); //clear data buffer
      strncpy(data_buf, data_start+33, 3); //copy the data we want into the buffer
      if(atoi(data_buf)){ //nonzero humidity means good data
        humidity = (atoi(data_buf)); //get the int value for humidity
        memset(data_buf, 0, 3); //clear data buffer
        strncpy(data_buf, data_start+25, 3); //copy the data we want into the buffer
        temperature = (atoi(data_buf)); //get the int value for temperature
      }
      else{
        Serial.println("Bad data, not updating values");
      }
      
      Serial.println((String)"The temperature in Boston is now: " + temperature + "*F");
      Serial.println((String)"The relative humidity in Boston is now: " + humidity + "%");
      Serial.println();
    }
  }
  else{
    Serial.println("Connection Failed!");
  }
  digitalWrite(LED_BUILTIN, LOW);
}

//display
void writeToMeter(uint8_t temp){
  uint16_t meter_val = temp * 257;
  Serial.println((String)temp + "," + meter_val);
  analogWrite(METER_PIN, meter_val);
  delayMicroseconds(500);
}

void setup() {
  Serial.begin(9600); //begin USB serial
  pinMode(METER_PIN, OUTPUT); //setup the meter pin
  analogWriteResolution(16);
  status = WiFi.begin(ssid, pass); //attempt wifi connection
  delay(3000); //wait for wifi connection
}

void loop() {
  // put your main code here, to run repeatedly:
  if(millis() - task_millis[0] > DOWNLOAD_UPDATE_PERIOD){
    updateRemoteTemps();
  }
  if(millis() - task_millis[1] > MODE_UPDATE_PERIOD){
    indoor_temperature++;
    writeToMeter(indoor_temperature);
  }
}
  
