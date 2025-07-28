#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include "secret.h"

#define REMOTE_TEMP

//scheduling
uint32_t task_millis[3]; //used to keep track of execution times of tasks [remote update, local update, check mode]
#define DOWNLOAD_UPDATE_PERIOD 60000 
#define LOCAL_UPDATE_PERIOD 5000
#define MODE_UPDATE_PERIOD 100 

//wifi 
char ssid[] = SSID_SECRET; //SSID and password are imported from ignored secret.h for security
char pass[] = PASSWORD_SECRET;
int status = WL_IDLE_STATUS; //status of Wifi connection
char server[] = "forecast.weather.gov";  //National Weather Service Data
WiFiClientSecure client; 

//data
char site_data[65535]; //buffer to hold the downloaded web page
uint8_t outdoor_temperature, outdoor_humidity, rain, indoor_temperature, indoor_humidity, data_temp; //weather stats to be displayed
char data_buf[3]; //to hold the strings to be parsed for values
char weather_buf[33]; 

//local sensor
#define DHTTYPE DHT11
#define SENSOR_PIN 16
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
        strncpy(weather_buf, data_start, 33);//copy the weather remark to the buffer
        if strstr(weather_buf, "RAIN"){ //it is currently raining
          analogWrite(WHITE_LED_PIN, 32767);
        }
        else{
          digitalWrite(WHITE_LED_PIN, 0);
        }
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
typedef struct {
    uint8_t temp;
    uint16_t val;
} temp_lut; //lookup table for temps
temp_lut temps_l[51] = { //low (celsius) range
    {0, 48830},
    {1, 48057},
    {2, 47285},
    {3, 46512},
    {4, 45740},
    {5, 44957},
    {6, 44101},
    {7, 43245},
    {8, 42390},
    {9, 41534},
    {10, 40606},
    {11, 39683},
    {12, 38760},
    {13, 37838},
    {14, 36915},
    {15, 35980},
    {16, 34968},
    {17, 33956},
    {18, 32944},
    {19, 31932},
    {20, 30840},
    {21, 29812},
    {22, 28784},
    {23, 27756},
    {24, 26728},
    {25, 25700},
    {26, 24672},
    {27, 23644},
    {28, 22616},
    {29, 21588},
    {30, 20560},
    {31, 19505},
    {32, 18449},
    {33, 17394},
    {34, 16338},
    {35, 15677},
    {36, 14592},
    {37, 13506},
    {38, 12421},
    {39, 11335},
    {40, 10794},
    {41, 9673},
    {42, 8552},
    {43, 7432},
    {44, 6311},
    {45, 6168},
    {46, 5047},
    {47, 3925},
    {48, 2804},
    {49, 1682},
    {50, 2065}
};

temp_lut temps_h[74] = { //high (farenheit) range
    {49, 41089},
    {50, 40606},
    {51, 40092},
    {52, 39578},
    {53, 39064},
    {54, 38550},
    {55, 38036},
    {56, 37522},
    {57, 37008},
    {58, 36494},
    {59, 35980},
    {60, 35409},
    {61, 34838},
    {62, 34267},
    {63, 33696},
    {64, 33124},
    {65, 32553},
    {66, 31982},
    {67, 31411},
    {68, 30840},
    {69, 30268},
    {70, 29697},
    {71, 29126},
    {72, 28555},
    {73, 27984},
    {74, 27413},
    {75, 26842},
    {76, 26271},
    {77, 25700},
    {78, 25128},
    {79, 24557},
    {80, 23986},
    {81, 23415},
    {82, 22844},
    {83, 22273},
    {84, 21702},
    {85, 21131},
    {86, 20560},
    {87, 20017},
    {88, 19474},
    {89, 18932},
    {90, 18389},
    {91, 17847},
    {92, 17304},
    {93, 16762},
    {94, 16219},
    {95, 15677},
    {96, 15134},
    {97, 14591},
    {98, 14049},
    {99, 13506},
    {100, 12964},
    {101, 12421},
    {102, 11879},
    {103, 11336},
    {104, 10794},
    {105, 10280},
    {106, 9766},
    {107, 9252},
    {108, 8738},
    {109, 8224},
    {110, 7710},
    {111, 7196},
    {112, 6682},
    {113, 6168},
    {114, 5712},
    {115, 5256},
    {116, 4800},
    {117, 4344},
    {118, 3888},
    {119, 3432},
    {120, 2976},
    {121, 2520},
    {122, 2065}
};

uint8_t disp_mode = 2; //counter for which stat is to be displayed
void writeToMeter(uint8_t temp){
  uint16_t meter_val = 0;
  uint8_t range = 0;
  if(temp < 0){
    temp = 0;
    Serial.println("Temp out of range (low)");
  }
  if(temp > 50){ //high range
    range = 1;
  }
  if(temp > 122){
    temp = 122;
    Serial.println("Temp out of range (high)");
  }
  for(uint8_t i = 0; i < 10; i++){
    if(range){ //high range
      digitalWrite(BLUE_LED_PIN, 0);
      for(uint8_t i = 0; i < 74; i++){
        if(temps_h[i].temp == temp){
          meter_val = temps_h[i].val;
        }
      }
    }
    else{ //low range
      analogWrite(BLUE_LED_PIN, 32767);
      for(uint8_t i = 0; i < 51; i++){
        if(temps_l[i].temp == temp){
          meter_val = temps_l[i].val;
        }
      }
    }
  }
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
  digitalWrite(BLUE_LED_PIN, 0);
  digitalWrite(WHITE_LED_PIN, 0);
  dht.begin();
  delay(3000);
  Serial.println("Attempting wifi connection!");
  status = WiFi.begin(ssid, pass); //attempt wifi connection
  delay(10000); //wait for wifi connection
  updateRemoteTemps();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(millis() - task_millis[0] > DOWNLOAD_UPDATE_PERIOD){ //time to download the outdoor temp/humidity
    task_millis[0] = millis();
    if(status != WL_CONNECTED){
      status = WiFi.begin(ssid, pass); //attempt wifi connection
    }
    updateRemoteTemps();
  }
  if(millis() -  task_millis[1] > LOCAL_UPDATE_PERIOD){ //time to read the indoor temp/humidity
    Serial.print("reading temp,humidity: ");
    Serial.print(dht.readTemperature(true,false));
    Serial.println((String)"," + dht.readHumidity(false));
    indoor_temperature = (int)dht.readTemperature(true,false);
    indoor_humidity = (int)dht.readHumidity(false);
    task_millis[1] = millis();
  }
  if(millis() - task_millis[2] > MODE_UPDATE_PERIOD){ //time to check the mode knob and display the appropriate stat
    task_millis[2] = millis();
    updateMode();
    Serial.println((String) "mode = "+ disp_mode);
    if(disp_mode == 0){
      // indoor_temperature = analogRead(KNOB_PIN) >> 2;
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
  }
}
  
