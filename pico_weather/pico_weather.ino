#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include "secret.h"

//scheduling
uint32_t task_millis[3]; //used to keep track of execution times of tasks [remote update, local update, check mode]
#define DOWNLOAD_UPDATE_PERIOD 60000 
#define LOCAL_UPDATE_PERIOD 500
#define MODE_UPDATE_PERIOD 100 

//wifi 
char ssid[] = SSID_SECRET; //SSID and password are imported from ignored secret.h for security
char pass[] = PASSWORD_SECRET;
int status = WL_IDLE_STATUS; //status of Wifi connection
char server[] = "forecast.weather.gov";  //National Weather Service Data
WiFiClientSecure client; 

//data
char site_data[65535]; //buffer to hold the downloaded web page
uint8_t outdoor_temperature, outdoor_humidity, indoor_temperature, indoor_humidity, data_temp; //weather stats to be displayed
char data_buf[3]; //to hold the strings to be parsed for values

//local sensor
#define DHTTYPE DHT11
#define SENSOR_PIN 15
DHT dht(SENSOR_PIN, DHTTYPE);
float local_temp = 0;
float local_humidity = 0;

//leds
#define BLUE_LED_PIN 14
#define WHITE_LED_PIN 15

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
        outdoor_humidity = (atoi(data_buf)); //get the int value for humidity
        memset(data_buf, 0, 3); //clear data buffer
        strncpy(data_buf, data_start+25, 3); //copy the data we want into the buffer
        outdoor_temperature = (atoi(data_buf)); //get the int value for temperature
      }
      else{
        Serial.println("Bad data, not updating values");
      }
      
      Serial.println((String)"The temperature in Boston is now: " + outdoor_temperature + "*F");
      Serial.println((String)"The relative humidity in Boston is now: " + outdoor_humidity + "%");
      Serial.println();
    }
  }
  else{
    Serial.println("Connection Failed!");
  }
  digitalWrite(LED_BUILTIN, LOW);
}

//display
#define METER_PIN 21
#define KNOB_PIN 28
uint8_t disp_mode = 0; //counter for which stat is to be displayed
void writeToMeter(uint8_t temp){
  uint16_t meter_val = temp * 257; //update this with the conversion from temp to analogWrite value to display correctly
  Serial.println((String)temp + "," + meter_val);
  analogWrite(METER_PIN, meter_val);
  delayMicroseconds(500);
}

void updateMode(){
  disp_mode = analogRead(KNOB_PIN) >> 8; //turn the meter reading to a selection out of 4 modes
}

void setup() {
  Serial.begin(9600); //begin USB serial
  pinMode(KNOB_PIN, INPUT); //setup the mode knob pin
  pinMode(METER_PIN, OUTPUT); //setup the meter pin
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(WHITE_LED_PIN, OUTPUT);
  analogWriteResolution(16);
  analogWrite(BLUE_LED_PIN, 32767);
  analogWrite(WHITE_LED_PIN, 32767);
  dht.begin();
  status = WiFi.begin(ssid, pass); //attempt wifi connection
  delay(3000); //wait for wifi connection
}

void loop() {
  // put your main code here, to run repeatedly:
  if(millis() - task_millis[0] > DOWNLOAD_UPDATE_PERIOD){ //time to download the outdoor temp/humidity
    task_millis[0] = millis();
    updateRemoteTemps();
  }
  if(millis() -  task_millis[1] > LOCAL_UPDATE_PERIOD){ //time to read the indoor temp/humidity
    task_millis[1] = millis();
  }
  if(millis() - task_millis[2] > MODE_UPDATE_PERIOD){ //time to check the mode knob and display the appropriate stat
    task_millis[2] = millis();
    //updateMode();
    if(disp_mode == 0){
      indoor_temperature = analogRead(KNOB_PIN) >> 2;
      writeToMeter(indoor_temperature);
    }
    else if(disp_mode == 1){
      writeToMeter(indoor_humidity);
    }
    else if(disp_mode == 2){
      writeToMeter(outdoor_temperature);
    }
    else if(disp_mode == 3){
      writeToMeter(outdoor_humidity);
    }

    // indoor_temperature++;
    // writeToMeter(indoor_temperature);
  }
}
  
